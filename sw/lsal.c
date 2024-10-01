#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

#ifdef DEBUG
#define NN 8
#define MM 9
#endif

const short GAP_i = -1;
const short GAP_d = -1;
const short MATCH = 2;
const short MISS_MATCH = -1;
const short CENTER = 0;
const short NORTH = 1;
const short NORTH_WEST = 2;
const short WEST = 3;
// static long int cnt_ops=0;
// static long int cnt_bytes=0;

/**********************************************************************************************
 * LSAL algorithm
 * Inputs:
 *          string1 is the query[N]
 *          string2 is the database[M]
 *          input sizes N, M
 * Outputs:
 *           max_index is the location of the highest similiarity score 
 *           similarity and direction matrices. Note that these two matrices are initialized with zeros.
 **********************************************************************************************/

void compute_matrices(
	char *string1, char *string2,
	int *max_index, int *similarity_matrix, short *direction_matrix, int N, int M)
{

    int index = 0;
    int i = 0;
	int j = 0;

    // Following values are used for the N, W, and NW values wrt. similarity_matrix[i]
    int north = 0;
	int west = 0;
	int northwest = 0;

	*max_index = N;
	 
	// Fill the first row of the similarity matrix.
	for (index = 0; index < N; index++) {
		int match = 0;
		int test_val1 = 0, test_val2 = 0;
		int val = 0;
		short dir = 0;

		i = index % N; // column index

		match = ( string1[i] == string2[0] ) ? MATCH : MISS_MATCH;
		
		test_val1 = match;
		if (index != 0) {
			test_val2 = similarity_matrix[index - 1] + GAP_i;
		}

		if (test_val1 >= test_val2) {
			val = test_val1;
			dir = NORTH_WEST;
		} else {
			val = test_val2;
			dir = WEST;
		}

		if(val <= 0) {
			val = 0;
			dir = CENTER;
		}

		if (val > similarity_matrix[*max_index])
			*max_index = index;
	
		similarity_matrix[index] = val;
		direction_matrix[index] = dir;
	}

	// Scan the N*M array row-wise starting from the second row.
	for(index = N; index < N*M; index++) {
		int match = 0;
		int test_val1 = 0, test_val2, test_val3;
		int val = 0;
		short dir = 0;

		i = index % N; // column index
		j = index / N; // row index

		north = similarity_matrix[index - N];
		
		// Check if we are in the first column.
		if (i == 0) {  
			// first column. 
			west = 0;
			northwest = 0;
		}
		else {      
			west  = similarity_matrix[index - 1];
			northwest = similarity_matrix[index - N - 1];
		}

		// Calculate the match value.
		match = ( string1[i] == string2[j] ) ? MATCH : MISS_MATCH;
		test_val1 = northwest + match;
		test_val2 = north + GAP_d;
		test_val3 = west + GAP_i;
		
		// Find the maximum value.
		if (test_val1 < test_val2) {
			if (test_val2 < test_val3) {
				val = test_val3;
				dir = WEST;
			} else {
				val = test_val2;
				dir = NORTH;
			}
		}
		else {
			if (test_val1 < test_val3) {
				val = test_val3;
				dir = WEST;
			} else {
				val = test_val1;
				dir = NORTH_WEST;
			}
		}

		if(val <= 0) {
			val = 0;
			dir = CENTER;
		}

		if (val > similarity_matrix[*max_index])
			*max_index = index;

		similarity_matrix[index] = val;
		direction_matrix[index] = dir;
	}   // end of for-loop
}  // end of function

/************************************************************************/

/*
 return a random number in [0, limit].
 */
int rand_lim(int limit) {

	int divisor = RAND_MAX / (limit + 1);
	int retval;

	do {
		retval = rand() / divisor;
	} while (retval > limit);

	return retval;
}

/*
 Fill the string with random values
 */
void fillRandom(char* string, int dimension) {
	//fill the string with random letters..
	static const char possibleLetters[] = "ATCG";

	string[0] = '-';

	int i;
	for (i = 0; i < dimension; i++) {
		int randomNum = rand_lim(3);
		string[i] = possibleLetters[randomNum];
	}

}

/*******************************************************************/
int main(int argc, char** argv) {

    clock_t t1, t2;
	#ifdef DEBUG
	int N = NN;
	int M = MM;
	#endif

	#ifndef DEBUG
	if (argc != 3) {
		printf("%s <Query Size N> <DataBase Size M>\n", argv[0]);
		return EXIT_FAILURE;
	}
	#endif
 
	fflush(stdout);

	/* Typically, M >> N */
	#ifndef DEBUG
	int N = atoi(argv[1]); 
    int M = atoi(argv[2]);
	long int iterate = (long int) N*M;
	if(iterate > INT_MAX) {
		perror("DIMENSIONS OUT OF LIMIT!");
		exit(EXIT_FAILURE);
	}
	#endif

	/************************ Memory Allocation ************************/
    char *query = (char*) malloc(sizeof(char) * N);
	char *database = (char*) malloc(sizeof(char) * M);
	int *similarity_matrix = (int*) malloc(sizeof(int) * N * M);
	short *direction_matrix = (short*) malloc(sizeof(short) * N * M);
	int *max_index = (int *) malloc(sizeof(int));

	/* Create the two input strings by calling a random number generator */
	#ifndef DEBUG
	fillRandom(query, N);
	fillRandom(database, M);
	#endif

	#ifdef DEBUG
	query = "GTGGGATG";
	database = "GCCACAAAG";
	
	// query = "TGTTACGG";
	// database = "GGTTGACTA";
	#endif

	memset(similarity_matrix, 0, sizeof(int) * N * M);
	memset(direction_matrix, 0, sizeof(short) * N * M);

	/************************ Execute LSAL algorithm ************************/
    t1 = clock();
	compute_matrices(query, database, max_index, similarity_matrix, direction_matrix, N, M);
	t2 = clock();

	/************************ Display results ************************/
	#ifdef SHOW_RESULTS
    printf(" max index is in position (%d, %d) \n", max_index[0]/N, max_index[0]%N );
	printf(" execution time of LSAL SW algorithm is %f sec \n", (double)(t2-t1) / CLOCKS_PER_SEC);
	#endif

	#ifndef DEBUG
	printf("%f\n", (double)(t2-t1) / CLOCKS_PER_SEC);
	#endif

	#ifdef DEBUG
	printf("Similarty Matrix\n");
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			printf("%d ", similarity_matrix[i * N + j]);
		}
		printf("\n");
	}

	printf("Direction Matrix\n");
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			printf("%d ", direction_matrix[i * N + j]);
		}
		printf("\n");
	}
	#endif

	/************************ Free Memory ************************/
	free(similarity_matrix);
	free(direction_matrix);
	free(max_index);

    // printf("cnt_ops=%d, cnt_bytes=%d\n", cnt_ops, cnt_bytes);
	return EXIT_SUCCESS;
}
