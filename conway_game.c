
/*
This program is an implementation of Conway's Game of Life, which is a simulation of the growth and death of cells in a grid. 
The program uses shared memory to store the grid, and forks child processes to update each row of the grid. 
A pipe is used for inter-process communication between the parent and child processes.
The main function initializes the shared memory segment for the grid and assigns it to a 2D array using a pointer. 
It then initializes the grid with random values, creates a new shared memory segment for the updated grid, and assigns it to a new 2D array.
Next, the program creates a pipe for inter-process communication between the parent and child processes. It then forks child processes 
to update each row of the grid. The child processes read the row index from the pipe, update their assigned row of the grid according 
to the rules of the game, and write the updated row back to the parent process through the pipe. The parent process continues to 
fork child processes until all rows of the grid have been updated.
After all child processes have finished, the updated grid is copied from the new shared memory segment to the original one. 
The program then prints the updated grid to the console. Finally, the shared memory segments are detached and deleted, and the program exits.
Overall, this program provides a parallel implementation of Conway's Game of Life using shared memory and forked child processes, 
which can be used to simulate larger grids and speed up the computation time.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define ROWS 10
#define COLS 10

int main() {
    int shmid, status;
    int (*grid)[COLS];  // Declare a 2D array using a pointer
    int (*newGrid)[COLS];  // Declare a new 2D array for the updated grid
    int pipefd[2];
    pid_t pid;
    
    // Create a shared memory segment to store the grid
    shmid = shmget(IPC_PRIVATE, ROWS * COLS * sizeof(int), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget error");
        exit(1);
    }
    
    // Attach the shared memory segment to the parent process
    grid = shmat(shmid, NULL, 0);
    if (grid == (void *)(-1)) {
	    perror("shmat error");
	    exit(1);
    }
    
    // Initialize the grid with random values
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            grid[i][j] = rand() % 2;
        }
    }
    
    // Create a new shared memory segment for the updated grid
    shmid = shmget(IPC_PRIVATE, ROWS * COLS * sizeof(int), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget error");
        exit(1);
    }
    
    // Attach the shared memory segment to the parent process
    newGrid = shmat(shmid, NULL, 0);
    if (newGrid == (void *)(-1)) {
	    perror("shmat error");
	    exit(1);
    }
    
    // Create a pipe for inter-process communication
    if (pipe(pipefd) < 0) {
        perror("pipe error");
        exit(1);
    }
    
    // Fork child processes to update the grid
    for (int i = 0; i < ROWS; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork error");
            exit(1);
        } 
        else if (pid == 0) {
            // Child process updates its assigned row of the grid
            close(pipefd[1]);  // Close the write end of the pipe
            
            int row;
            read(pipefd[0], &row, sizeof(row));  // Read the row index from the pipe
            
            for (int j = 0; j < COLS; j++) {
                // Count the number of live neighbors
                int neighbors = 0;
                if (row > 0 && grid[row-1][j] == 1) {
                    neighbors++;
                }
                if (row < ROWS-1 && grid[row+1][j] == 1) {
                    neighbors++;
                }
                if (j > 0 && grid[row][j-1] == 1) {
                    neighbors++;
                }
                if (j < COLS-1 && grid[row][j+1] == 1) {
                    neighbors++;
                }
                if (row > 0 && j > 0 && grid[row-1][j-1] == 1) {
                    neighbors++;
                }
                if (row > 0 && j < COLS-1 && grid[row-1][j+1] == 1) {
                    neighbors++;
                }
                if (row < ROWS-1 && j > 0 && grid[row+1][j-1] == 1) {
                    neighbors++;
                }
                if (row < ROWS-1 && j < COLS-1 && grid[row+1][j+1] == 1) {
                    neighbors++;
                }
                
                // Apply the rules of the game to update the new cell value
                if (grid[row][j] == 1) {
                    if (neighbors < 2 || neighbors > 3) {
                        newGrid[row][j] = 0;
                    } else {
                        newGrid[row][j] = 1;
                    }
                } else {
                    if (neighbors == 3) {
                        newGrid[row][j] = 1;
                    } else {
                        newGrid[row][j] = 0;
                    }
                }
            }
            
            // Write the updated row to the parent process through the pipe
            write(pipefd[1], &row, sizeof(row));
            shmdt(grid);  // Detach from the old shared memory segment
            shmdt(newGrid);  // Detach from the new shared memory segment
            exit(0);
        } else {
            // Parent process continues to fork child processes
            close(pipefd[0]);  // Close the read end of the pipe
            write(pipefd[1], &i, sizeof(i));  // Write the row index to the pipe
        }
    }
    
    // Wait for all child processes to finish
    for (int i = 0; i < ROWS; i++) {
        wait(&status);
    }
    
    // Copy the updated grid from the new shared memory segment to the original one
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            grid[i][j] = newGrid[i][j];
        }
    }
    
    // Print the updated grid
    printf("Updated grid:\n");
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            printf("%d ", grid[i][j]);
        }
        printf("\n");
    }
    
    // Detach from the shared memory segments
    shmdt(grid);
    shmdt(newGrid);
    
    // Deallocate the shared memory segments
    shmctl(shmid, IPC_RMID, NULL);
    
    return 0;
}