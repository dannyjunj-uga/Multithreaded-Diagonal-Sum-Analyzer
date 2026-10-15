Name: Daniel Jungwirth
811#: 811866976

Project 4: Multithreaded Diagonal Sums

    Compilation Instructions:
        make

        If you do not have the provided Makefile available, you can also compile with:
        gcc -O2 -pthread -Wall -Wextra -std=c11 main.c proj4.c -o proj4.out

    Execution Instructions:
        ./proj4.out inputFile outputFile s t

    Example runs:
        ./proj4.out in1.txt out1.txt 10 1
        ./proj4.out in2.txt out2.txt 19 2
        ./proj4.out in3.txt out3.txt 3 3
        ./proj4.out in4.txt out4.txt 13 2
        ./proj4.out in5.txt out5.txt 1222 3

    Summary of approach:
        The program reads the input grid into dynamically allocated memory, then examines every diagonal in both diagonal directions. For each diagonal, it checks every contiguous diagonal segment and marks the cells that belong to at least one segment whose sum matches the target value s. The work is divided across 1 to 3 threads by assigning diagonals round-robin to the workers. Each worker stores marks in its own buffers, and the buffers are merged at the end into the final output grid.

    Findings on thread performance:
        For small grids, using more threads may not always be faster because the overhead of creating and joining threads can outweigh the benefit of parallelism. For large grids, using 2 threads should generally outperform 1 thread, and using 3 threads should generally outperform 2 threads because the diagonal computations are divided across multiple workers. The project examples support this idea, especially for the larger inputs where the amount of computation is much greater.
