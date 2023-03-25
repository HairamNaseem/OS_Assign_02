# OS_Assign_02
k213062 OS Assignment 2

This program is an implementation of Conway's Game of Life, which is a simulation of the growth and death of cells in a grid. The program uses shared memory to store the grid, and forks child processes to update each row of the grid. A pipe is used for inter-process communication between the parent and child processes.

The main function initializes the shared memory segment for the grid and assigns it to a 2D array using a pointer. It then initializes the grid with random values, creates a new shared memory segment for the updated grid, and assigns it to a new 2D array.

Next, the program creates a pipe for inter-process communication between the parent and child processes. It then forks child processes to update each row of the grid. The child processes read the row index from the pipe, update their assigned row of the grid according to the rules of the game, and write the updated row back to the parent process through the pipe. The parent process continues to fork child processes until all rows of the grid have been updated.

After all child processes have finished, the updated grid is copied from the new shared memory segment to the original one. The program then prints the updated grid to the console. Finally, the shared memory segments are detached and deleted, and the program exits.

Overall, this program provides a parallel implementation of Conway's Game of Life using shared memory and forked child processes, which can be used to simulate larger grids and speed up the computation time.
