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

	// Scan the N*M array row-wise starting from the second row.
	for(i = 1; i < M; i++) {
		for (j = 1; j < N; j++) {
			int match = 0;
			int test_val1 = 0, test_val2 = 0, test_val3 = 0;
			int val = 0;
			short dir = 0;

			// Calculate the index
			index = i*N + j;

			// Get the N, W, and NW values
			north = similarity_matrix[index - N];	
			west  = similarity_matrix[index - 1];
			northwest = similarity_matrix[index - N - 1];
			
			// Calculate the similarity score
			match = (string1[j-1] == string2[i-1]) ? MATCH : MISS_MATCH;

			// Calculate the maximum value
			test_val1 = northwest + match;
			test_val2 = north + GAP_d;
			test_val3 = west + GAP_i;
			
			// Find the maximum value
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

			if (val > similarity_matrix[*max_index]) {
				*max_index = index;
			}

			similarity_matrix[index] = val;
			direction_matrix[index] = dir;
		}
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
	int *similarity_matrix = (int*) malloc(sizeof(int) * (N+1) * (M+1));
	short *direction_matrix = (short*) malloc(sizeof(short) * (N+1) * (M+1));
	int *max_index = (int *) malloc(sizeof(int));

	/* Create the two input strings by calling a random number generator 
	 * On debug mode, the strings are fixed to "TGTTACGG" and "GGTTGACTA"
	 */

	#ifndef DEBUG
	fillRandom(query, N);
	fillRandom(database, M);
	#endif

	#ifdef DEBUG
	query = "TGTTACGG";
	database = "GGTTGACTA";
	#endif

	memset(similarity_matrix, 0, sizeof(int) * (N+1) * (M+1));
	memset(direction_matrix, 0, sizeof(short) * (N+1) * (M+1));

	/************************ Execute LSAL Algorithm  ************************/
    t1 = clock();
	compute_matrices(query, database, max_index, similarity_matrix, direction_matrix, N+1, M+1);
	t2 = clock();

	/************************ Display Results ************************/
	#ifdef SHOW_RESULTS
    printf(" max index is in position (%d, %d) \n", max_index[0]/N, max_index[0]%N );
	printf(" execution time of LSAL SW algorithm is %f sec \n", (double)(t2-t1) / CLOCKS_PER_SEC);
	#endif
	
	#ifndef DEBUG
	printf("%f\n", (double)(t2-t1) / CLOCKS_PER_SEC);
	#endif

	#ifdef DEBUG
	printf("Similarty Matrix\n");
	for (int i = 1; i < M+1; i++) {
		for (int j = 1; j < N+1; j++) {
			printf("%d ", similarity_matrix[i*(N+1) + j]);
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
