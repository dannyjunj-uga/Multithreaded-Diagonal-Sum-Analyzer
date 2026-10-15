#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "proj4.h"

/*
 * The struct thread_data_t stores the information needed by one worker to
 * process a subset of the diagonals in the input grid.
 */
typedef struct thread_data_t {
    grid * input;
    unsigned long targetSum;
    int workerId;
    int totalWorkers;
    unsigned char * downRightMarks;
    unsigned char * downLeftMarks;
} thread_data;

/*
 * Allocate an n-by-n grid on the heap using one contiguous block for cell
 * data and store the result in g.
 * input: pointer to grid to initialize and dimension n
 * output: none
 * assumptions: g is not NULL and n >= 1
 */
static void allocateGrid(grid * g, unsigned int n);

/*
 * Convert a down-right diagonal identifier into its starting row and column.
 * input: grid dimension n, diagonal id, and output pointers for row and col
 * output: none
 * assumptions: 0 <= diagId < (2 * n - 1)
 */
static void getDownRightStart(unsigned int n, unsigned int diagId,
unsigned int * startRow,
unsigned int * startCol);

/*
 * Convert a down-left diagonal identifier into its starting row and column.
 * input: grid dimension n, diagonal id, and output pointers for row and col
 * output: none
 * assumptions: 0 <= diagId < (2 * n - 1)
 */
static void getDownLeftStart(unsigned int n, unsigned int diagId,
unsigned int * startRow,
unsigned int * startCol);

/*
 * Return the length of one down-right diagonal.
 * input: grid dimension n and diagonal id
 * output: number of cells on the diagonal
 * assumptions: 0 <= diagId < (2 * n - 1)
 */
static unsigned int getDownRightLength(unsigned int n, unsigned int diagId);

/*
 * Return the length of one down-left diagonal.
 * input: grid dimension n and diagonal id
 * output: number of cells on the diagonal
 * assumptions: 0 <= diagId < (2 * n - 1)
 */
static unsigned int getDownLeftLength(unsigned int n, unsigned int diagId);

/*
 * Process one complete diagonal using a difference-array technique so that
 * every cell belonging to at least one matching contiguous segment is marked.
 * input: input grid, target sum, diagonal identifier, direction, and marks
 * output: none
 * assumptions: all pointers are valid and the diagonal id is valid
 */
static void processDiagonal(grid * input, unsigned long targetSum,
unsigned int diagId, int colStep,
unsigned char * marks);

/*
 * Process the diagonals assigned to one worker thread.
 * input: pointer to thread_data cast to void *
 * output: NULL
 * assumptions: arg points to a valid thread_data struct
 */
static void * diagonalWorker(void * arg);

/*
 * Merge all worker mark buffers into the output grid by copying the input
 * digit anywhere at least one mark buffer contains a nonzero value.
 * input: input grid, output grid, mark buffers, and worker count
 * output: none
 * assumptions: all pointers are valid and the grids share the same dimension
 */
static void mergeMarksIntoOutput(grid * input, grid * output,
unsigned char ** downRightMarks,
unsigned char ** downLeftMarks,
int workerCount);

/*
 * Allocate an n-by-n grid on the heap using one contiguous block for cell
 * data and store the result in g.
 * input: pointer to grid to initialize and dimension n
 * output: none
 * assumptions: g is not NULL and n >= 1
 */
static void allocateGrid(grid * g, unsigned int n) {
    unsigned int row = 0;
    unsigned char * data = NULL;

    g->n = n;
    g->p = (unsigned char **) malloc((size_t) n * sizeof(unsigned char *));
    data = (unsigned char *) calloc((size_t) n * (size_t) n,
    sizeof(unsigned char));

    while (row < n) {
        g->p[row] = data + ((size_t) row * (size_t) n);
        row++;
    }
}

/*
 * Initialize g based on fileName, where fileName is the name of a valid input
 * file containing an n-by-n grid of digits.
 * input: pointer to grid and input file name
 * output: none
 * assumptions: fileName exists, is readable, and contains valid input
 */
void initializeGrid(grid * g, char * fileName) {
    FILE * inputFile = fopen(fileName, "r");
    int currentChar = 0;
    unsigned int n = 0;
    unsigned int row = 0;
    unsigned int col = 0;

    currentChar = fgetc(inputFile);
    while (currentChar != '\n' && currentChar != EOF) {
        n++;
        currentChar = fgetc(inputFile);
    }

    rewind(inputFile);
    allocateGrid(g, n);

    row = 0;
    while (row < n) {
        col = 0;
        while (col < n) {
            currentChar = fgetc(inputFile);
            g->p[row][col] = (unsigned char) (currentChar - '0');
            col++;
        }
        currentChar = fgetc(inputFile);
        row++;
    }

    fclose(inputFile);
}

/*
 * Convert a down-right diagonal identifier into its starting row and column.
 * input: grid dimension n, diagonal id, and output pointers for row and col
 * output: none
 * assumptions: 0 <= diagId < (2 * n - 1)
 */
static void getDownRightStart(unsigned int n, unsigned int diagId,
unsigned int * startRow,
unsigned int * startCol) {
    if (diagId < n) {
        *startRow = 0;
        *startCol = diagId;
    }
    else {
        *startRow = diagId - n + 1;
        *startCol = 0;
    }
}

/*
 * Convert a down-left diagonal identifier into its starting row and column.
 * input: grid dimension n, diagonal id, and output pointers for row and col
 * output: none
 * assumptions: 0 <= diagId < (2 * n - 1)
 */
static void getDownLeftStart(unsigned int n, unsigned int diagId,
unsigned int * startRow,
unsigned int * startCol) {
    if (diagId < n) {
        *startRow = 0;
        *startCol = diagId;
    }
    else {
        *startRow = diagId - n + 1;
        *startCol = n - 1;
    }
}

/*
 * Return the length of one down-right diagonal.
 * input: grid dimension n and diagonal id
 * output: number of cells on the diagonal
 * assumptions: 0 <= diagId < (2 * n - 1)
 */
static unsigned int getDownRightLength(unsigned int n, unsigned int diagId) {
    unsigned int length = 0;

    if (diagId < n) {
        length = n - diagId;
    }
    else {
        length = (2U * n) - 1U - diagId;
    }

    return length;
}

/*
 * Return the length of one down-left diagonal.
 * input: grid dimension n and diagonal id
 * output: number of cells on the diagonal
 * assumptions: 0 <= diagId < (2 * n - 1)
 */
static unsigned int getDownLeftLength(unsigned int n, unsigned int diagId) {
    unsigned int length = 0;

    if (diagId < n) {
        length = diagId + 1U;
    }
    else {
        length = (2U * n) - 1U - diagId;
    }

    return length;
}

/*
 * Process one complete diagonal using a difference-array technique so that
 * every cell belonging to at least one matching contiguous segment is marked.
 * input: input grid, target sum, diagonal identifier, direction, and marks
 * output: none
 * assumptions: all pointers are valid and the diagonal id is valid
 */
static void processDiagonal(grid * input, unsigned long targetSum,
unsigned int diagId, int colStep,
unsigned char * marks) {
    unsigned int n = input->n;
    unsigned int startRow = 0;
    unsigned int startCol = 0;
    unsigned int length = 0;
    unsigned int startIndex = 0;
    unsigned int endIndex = 0;
    unsigned int pos = 0;
    unsigned int row = 0;
    unsigned int col = 0;
    int * diff = NULL;
    int coverage = 0;

    if (colStep == 1) {
        getDownRightStart(n, diagId, &startRow, &startCol);
        length = getDownRightLength(n, diagId);
    }
    else {
        getDownLeftStart(n, diagId, &startRow, &startCol);
        length = getDownLeftLength(n, diagId);
    }

    diff = (int *) calloc((size_t) length + 1U, sizeof(int));

    startIndex = 0;
    while (startIndex < length) {
        unsigned long runningSum = 0;
        endIndex = startIndex;
        while (endIndex < length) {
            row = startRow + endIndex;
            col = (unsigned int) ((int) startCol + ((int) endIndex * colStep));
            runningSum += (unsigned long) input->p[row][col];
            if (runningSum == targetSum) {
                diff[startIndex]++;
                diff[endIndex + 1U]--;
            }
            endIndex++;
        }
        startIndex++;
    }

    coverage = 0;
    pos = 0;
    while (pos < length) {
        coverage += diff[pos];
        if (coverage > 0) {
            row = startRow + pos;
            col = (unsigned int) ((int) startCol + ((int) pos * colStep));
            marks[((size_t) row * (size_t) n) + (size_t) col] = 1;
        }
        pos++;
    }

    free(diff);
}

/*
 * Process the diagonals assigned to one worker thread.
 * input: pointer to thread_data cast to void *
 * output: NULL
 * assumptions: arg points to a valid thread_data struct
 */
static void * diagonalWorker(void * arg) {
    thread_data * data = (thread_data *) arg;
    unsigned int totalDiagonals = (2U * data->input->n) - 1U;
    unsigned int diagId = (unsigned int) data->workerId;

    while (diagId < totalDiagonals) {
        processDiagonal(data->input, data->targetSum, diagId, 1,
        data->downRightMarks);
        processDiagonal(data->input, data->targetSum, diagId, -1,
        data->downLeftMarks);
        diagId += (unsigned int) data->totalWorkers;
    }

    return NULL;
}

/*
 * Merge all worker mark buffers into the output grid by copying the input
 * digit anywhere at least one mark buffer contains a nonzero value.
 * input: input grid, output grid, mark buffers, and worker count
 * output: none
 * assumptions: all pointers are valid and the grids share the same dimension
 */
static void mergeMarksIntoOutput(grid * input, grid * output,
unsigned char ** downRightMarks,
unsigned char ** downLeftMarks,
int workerCount) {
    unsigned int row = 0;
    unsigned int col = 0;
    unsigned int n = input->n;
    int worker = 0;

    while (row < n) {
        col = 0;
        while (col < n) {
            size_t index = ((size_t) row * (size_t) n) + (size_t) col;
            int marked = 0;

            worker = 0;
            while (worker < workerCount) {
                if (downRightMarks[worker][index] != 0 ||
                downLeftMarks[worker][index] != 0) {
                    marked = 1;
                }
                worker++;
            }

            if (marked != 0) {
                output->p[row][col] = input->p[row][col];
            }
            col++;
        }
        row++;
    }
}

/*
 * Compute all diagonal sums in input equal to s using t threads and store the
 * marked output grid in output.
 * input: input grid, target sum s, output grid, and number of threads t
 * output: none
 * assumptions: input and output are valid pointers and 1 <= t <= 3
 */
void diagonalSums(grid * input, unsigned long s, grid * output, int t) {
    pthread_t threads[2];
    thread_data data[3];
    unsigned int n = input->n;
    unsigned char * downRightMarks[3] = {NULL, NULL, NULL};
    unsigned char * downLeftMarks[3] = {NULL, NULL, NULL};
    int worker = 0;

    allocateGrid(output, n);

    worker = 0;
    while (worker < t) {
        downRightMarks[worker] = (unsigned char *) calloc((size_t) n * (size_t) n,
        sizeof(unsigned char));
        downLeftMarks[worker] = (unsigned char *) calloc((size_t) n * (size_t) n,
        sizeof(unsigned char));
        data[worker].input = input;
        data[worker].targetSum = s;
        data[worker].workerId = worker;
        data[worker].totalWorkers = t;
        data[worker].downRightMarks = downRightMarks[worker];
        data[worker].downLeftMarks = downLeftMarks[worker];
        worker++;
    }

    if (t >= 2) {
        pthread_create(&threads[0], NULL, diagonalWorker, &data[1]);
    }
    if (t == 3) {
        pthread_create(&threads[1], NULL, diagonalWorker, &data[2]);
    }

    diagonalWorker(&data[0]);

    if (t >= 2) {
        pthread_join(threads[0], NULL);
    }
    if (t == 3) {
        pthread_join(threads[1], NULL);
    }

    mergeMarksIntoOutput(input, output, downRightMarks, downLeftMarks, t);

    worker = 0;
    while (worker < t) {
        free(downRightMarks[worker]);
        free(downLeftMarks[worker]);
        worker++;
    }
}

/*
 * Write the contents of g to fileName, overwriting the file if it exists or
 * creating it with owner read and write permissions if it does not exist.
 * input: grid pointer and output file name
 * output: none
 * assumptions: g is valid and fileName is not NULL
 */
void writeGrid(grid * g, char * fileName) {
    int fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    FILE * outputFile = fdopen(fd, "w");
    unsigned int row = 0;
    unsigned int col = 0;

    while (row < g->n) {
        col = 0;
        while (col < g->n) {
            fputc((int) ('0' + g->p[row][col]), outputFile);
            col++;
        }
        fputc('\n', outputFile);
        row++;
    }

    fclose(outputFile);
    chmod(fileName, S_IRUSR | S_IWUSR);
}

/*
 * Free all dynamically allocated memory associated with g.
 * input: pointer to grid to free
 * output: none
 * assumptions: g was allocated by this program and g is not NULL
 */
void freeGrid(grid * g) {
    if (g->p != NULL) {
        free(g->p[0]);
        free(g->p);
        g->p = NULL;
        g->n = 0;
    }
}
