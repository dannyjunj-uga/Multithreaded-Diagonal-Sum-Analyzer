#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>
#include "proj4.h"

/*
 * Do not modify anything in this file. 
 */

/*
 * Helper function for main. 
 * Check for some errors and print error messages 
 * for command line arguments passed to main.
 * If an error is found, the program terminates.
 */
void errorCheck(int argc, char ** argv){
  bool errorFound = false;
  if(argc != 5){
    printf("Usage error: ./proj4.out inputFile outputFile s t\n");
    errorFound = true;
  }
  else if( access(argv[1], F_OK) != 0 ){
    printf("Error accessing %s in the present working directory\n", argv[1]);
    errorFound = true;
  }
  else {
    int t = atoi(argv[4]);
    if(t < 1 || 3 < t){
      printf("Error: t must be between 1 and 3 (inclusive)\n");
      errorFound = true;
    }
  }
  if(errorFound) exit(0);
}

/*
 * This program should be compiled to ./proj4.out using the provided
 * Makefile, and it will process five command line arguments.
 *   ./proj.out input.txt output.txt s t
 *      input.txt is the name of a file in the present working directory that 
 *        contains a n-by-n grid of digits (1 through 9),
 *        where n >= 1.
 *      output.txt is the name of a file in the present working directory to save
 *        the output of all of the diagonal sums. If the file does not exist,
 *        then this file will be created in the present working directory.
 *      s is the sum for the diagonal sums.
 *      t is the number of threads (1 <= t <= 3) to use
 *        to compute the diagonal sums. If t is 1, then only the 
 *        main thread will be used. If 2 <= t <= 3, then the main
 *        thread and (t - 1) POSIX thread(s) will be used to compute
 *        the diagonal sums.
 * This program will only time the call to diagonalSums.
 *
 */
int main(int argc, char ** argv){
  errorCheck(argc, argv);
  char * inputFile = argv[1];
  char * outputFile = argv[2];
  unsigned long sum = (unsigned long) atol(argv[3]);
  int t = atoi(argv[4]);
  grid g, diagonalSumsOutput;
  initializeGrid(&g, inputFile);

  printf("Computing the diagonal sums equal to %ld in a %d-by-%d grid using %d thread(s).\n",
         sum, g.n, g.n, t);
  struct timeval start, end;    // start and end time
  unsigned long e_usec; // elapsed microseconds
  gettimeofday(&start, 0); // mark the start time
  diagonalSums(&g, sum, &diagonalSumsOutput, t);
  gettimeofday(&end, 0);        // mark the end time
  e_usec = ((end.tv_sec * 1000000) + end.tv_usec) -
    ((start.tv_sec * 1000000) + start.tv_usec);
  printf("Elapsed time for computing the diagonal sums using %d thread(s): %lf seconds.\n",
         t, e_usec / 1000000.0);

  printf("Writing the diagonal sums equal to %ld to the file %s.\n",
         sum, outputFile);
  writeGrid(&diagonalSumsOutput, outputFile);
  freeGrid(&g);
  freeGrid(&diagonalSumsOutput);
  printf("Program is complete. Goodbye!\n");
  return 0;
}
