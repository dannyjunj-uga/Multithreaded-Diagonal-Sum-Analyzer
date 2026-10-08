# Multithreaded Diagonal Sum Analyzer

## Overview

Multithreaded Diagonal Sum Analyzer is a C-based systems programming project that processes large two-dimensional grids of numerical values and identifies diagonal sequences whose values add up to a specified target sum. The program supports both diagonal directions and uses POSIX threads to parallelize the computation across large input grids.

The project was designed to demonstrate practical systems programming concepts including multithreading, Unix file I/O, dynamic memory management, pointers, shared data, synchronization, and algorithm optimization.

## Features

* Processes large `n × n` grids containing numerical values.
* Searches for qualifying diagonal sequences in both diagonal directions.
* Identifies diagonal sequences whose values sum to a user-specified target.
* Supports 1, 2, or 3 POSIX threads using `pthread_create()`.
* Distributes computational work across multiple threads to improve performance on large datasets.
* Uses dynamic memory allocation to store and process grid data.
* Reads input data from files and writes results to an output file using Unix file I/O.
* Handles large grids containing millions of elements.
* Produces deterministic output regardless of the number of threads used.
* Designed with an emphasis on memory efficiency and scalability.

## Technical Implementation

The program is implemented in C and makes use of POSIX threads for concurrent processing. When multiple threads are requested, the grid-processing workload is divided among the available threads. Each thread independently performs its assigned portion of the diagonal-sum search before the results are combined into the final output.

The algorithm is designed with a worst-case time complexity of **O(n³)** and a space complexity of **O(n²)**, allowing it to process substantially larger grids than a naive implementation while maintaining predictable memory requirements.

The project also uses dynamic memory allocation and deallocation to manage the grid and intermediate data structures. Proper memory management is important because the program is designed to operate on large input files.

## File I/O

Input grids are read from text files containing the dimensions of the grid followed by the grid values. The program accepts a target sum and number of threads through the command line and generates an output file containing the resulting diagonal-sum analysis.

Example:

```bash
./proj4.out input.txt output.txt 1222 3
```

The arguments specify:

1. Input file
2. Output file
3. Target diagonal sum
4. Number of threads

## Multithreading

The project demonstrates the use of POSIX threads through the `pthread` library. The program can operate using one, two, or three threads, allowing performance to be compared across different levels of parallelism.

For sufficiently large grids, additional threads can reduce the amount of time required to complete the diagonal-sum search. The project also evaluates the practical performance differences between thread counts, demonstrating that multithreading benefits become more noticeable as the size of the workload increases.

## Memory Management and Testing

Because the program processes potentially very large grids, memory management is an important part of the implementation. Dynamic memory is allocated only as needed and properly released when processing is complete.

The program can be tested using multiple input datasets of different sizes to verify both correctness and performance. Valgrind can also be used to detect memory leaks and other memory-management issues.

## Technologies

* **C**
* **POSIX Threads (`pthread`)**
* **Unix/Linux**
* **Dynamic Memory Allocation**
* **Unix File I/O**
* **GDB**
* **Valgrind**
* **Multithreading**
* **Algorithm Optimization**
* **Data Structures**
