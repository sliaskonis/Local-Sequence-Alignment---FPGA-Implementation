#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

#define N 32
#define M 65536
#define DIRECTION_MATRIX_SIZE N * (N + M - 1)
#define MATRIX_SIZE N * M
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


/* Reorder the resulting direction matrix to a 2D shape */
/////////////////////////////////////////////////////////////
char* order_matrix_blocks(char* in_matrix){
	char *out_matrix = (char *) malloc(sizeof(char) * MATRIX_SIZE);

	int num_diag = 0;
	int store_elem = 1;
	int store_index;
	int tmp_i = 0;
	int tmp_j = 0;
	int i = 0;
	int j;

	for (i = 0; i < DIRECTION_MATRIX_SIZE;){  // DIRECTION_MATRIX_SIZE = N * (N + M - 1)
		if(num_diag < M) {
			tmp_j = num_diag;
			tmp_i = 0;
		} else {
			tmp_i = (num_diag + 1) - M;
			tmp_j = M - 1;
		}

		for (j = 0; j < store_elem; j++) {
			store_index = tmp_j * N + tmp_i;
			out_matrix[store_index] = in_matrix[i];
			tmp_j--;
			tmp_i++;
			i++;
		}
		num_diag++;
		if(num_diag >= M){
			store_elem--;
			i = num_diag * N + N - store_elem;
		} else {
			if(store_elem != N){
				store_elem++;
				i = num_diag * N;
			}
		}
	}

	return out_matrix;
}

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
void compute_matrices_gold(
	char *string1, char *string2,
	int *max_index, int *similarity_matrix, short *direction_matrix)
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
		match = (string1[i] == string2[j]) ? MATCH : MISS_MATCH;

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

// void compute_matrices(char query[N], char database[M], int *max_index, char direction_matrix[N* (N + M - 1)])
// 	{

// 		int max_index_temp = 0;
// 		int max_index_arr[N];
// 		char buffer[3*(N+1)];
// 		char max_value = 0;
// 		char max_value_arr[N];
// 		char dir_buf[2*N];
// 		int it = 0;
// 		char val = 0;
// 		char dir;
// 		char match = 0;
// 		char val_west = 0;
// 		char val_north = 0;
// 		char val_northwest = 0;
// 		char val1;
// 		char val2;

// 		// Create buffer for query and database
// 		char query_buf[N];

// 		// Create database buffer
// 		char database_buf[N+1];

// 		// Copy the first N characters of the database to the database buffer
// 		memcpy(database_buf+1, database, N*sizeof(char));

// 		memcpy(query_buf, query, N*sizeof(char));
// 		memset(buffer, 0, 3*(N+1)*sizeof(char));
// 		memset(max_value_arr, 0, N*sizeof(char));
// 		memset(max_index_arr, 0, N*sizeof(int));

// 		// Fill the direction matrix using the anti-diagonal buffer.
// 		for (int j = N; j < M + 2*(N-1)+1; j++) {

// 			// Shift all database elements one place to the left
// 			for (int i = 0; i < N-1; i++) {
// 				database_buf[i] = database_buf[i+1];
// 			}
// 			database_buf[N-1] = database[j-1];

// 			for (int i = 0; i < N; i++) {

// 				it = j-N;
// 				int buffer_index = it % 3;
// 				switch (buffer_index) {
// 					case (0):
// 						val1 = 2;
// 						val2 = 1;
// 						break;
// 					case (1):
// 						val1 = -1;
// 						val2 = 1;
// 						break;
// 					case (2):
// 						val1 = -1;
// 						val2 = -2;
// 						break;
// 				}

// 				match = (query_buf[i] == database_buf[N-i-1]) ? MATCH : MISS_MATCH;

// 				// Calculate values
// 				val_west = buffer[(buffer_index+val1)*(N+1) + i] + GAP_i;
// 				val_north = buffer[(buffer_index+val1)*(N+1) + i+1] + GAP_d;
// 				val_northwest = buffer[(buffer_index+val2)*(N+1) +i] + match;

// 				if (val_northwest < val_north && val_north < val_west) {
// 					val = val_west;
// 					dir = WEST;
// 				} else if (val_northwest < val_north) {
// 					val = val_north;
// 					dir = NORTH;
// 				} else if (val_northwest < val_west) {
// 					val = val_west;
// 					dir = WEST;
// 				} else {
// 					val = val_northwest;
// 					dir = NORTH_WEST;
// 				}

// 				int indx = buffer_index*(N+1)+i+1;

// 				if (val > 0) {
// 					buffer[indx]=val;
// 					dir_buf[(j%2)*N + i]=dir;
// 				}
// 				else {
// 					buffer[indx] = 0;
// 					dir_buf[(j%2)*N + i] = CENTER;
// 				}

// 				// Calculate max index
// 				if (val > max_value_arr[i]) {
// 					max_value_arr[i] = val;
// 					max_index_arr[i] = (it-i)*N + i;
// 				}
// 			}
// 			memcpy(direction_matrix+((it*N)), dir_buf+(j%2)*N, N*sizeof(char));
// 		}
// 		max_loop: for(int k = 0; k < N; k++) {
// 			if (max_value_arr[k] > max_value) {
// 				max_value = max_value_arr[k];
// 				max_index_temp = max_index_arr[k];
// 			}
// 		}
// 		*max_index = max_index_temp;
// 	}

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
int main() {

    int i, j;

	fflush(stdout);

	/************************ Memory Allocation ************************/
    char *query = (char*) malloc(sizeof(char) * (N));
	char *database = (char*) malloc(sizeof(char) * (2*(N-1)+M));
	char *direction_matrix = (char*) malloc(sizeof(char) * (N * (N + M - 1)));
	int *max_index = (int *) malloc(sizeof(int));

	if (query == NULL || database == NULL || direction_matrix == NULL || max_index == NULL) {
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}

	/* Create the two input strings by calling a random number generator */
	fillRandom(query, N);
	fillRandom(database, 2*(N-1)+M);

	// Add padding to the database
	for(i = 0; i < N-1; i++){
		database[i] = 'P';
	}
	for(i = M+N-1; i < M+2*(N-1); i++){
		database[i] = 'P';
	}

	memset(direction_matrix, 0, sizeof(char) * (N * (N+M-1)));

	/************************ Execute LSAL algorithm ************************/
	compute_matrices(query, database, max_index, direction_matrix);

	char *ordered_matrix = order_matrix_blocks(direction_matrix);

	// Write results to out.dat
	FILE *fp;
	fp = fopen("out.dat", "w");
	fprintf(fp,"Direction Matrix\n");
	for (i = 0; i < M; i++) {
		for (j = 0; j < N; j++) {
			fprintf(fp, "%hhu ", ordered_matrix[i * N + j]);
		}
		fprintf(fp,"\n");
	}
	 fprintf(fp, "Max index: %d\n", *max_index);
	fclose(fp);

	/************************ Execute golden algorithm ************************/
    char *database2 = (char*) malloc(sizeof(char) * M);
	int *similarity_matrix2 = (int*) malloc(sizeof(int) * N * M);
	short *direction_matrix2 = (short*) malloc(sizeof(short) * N * M);

	memset(similarity_matrix2, 0, sizeof(int) * N * M);
	memset(direction_matrix2, 0, sizeof(short) * N * M);
	*max_index = 0;

	for(i = 0; i < M; i++){
		database2[i] = database[i+N-1];
	}

	compute_matrices_gold(query, database2, max_index, similarity_matrix2, direction_matrix2);

	fp = fopen("out.gold.dat", "w");
	fprintf(fp,"Direction Matrix\n");
	for (i = 0; i < M; i++) {
		for (j = 0; j < N; j++) {
			fprintf(fp, "%hu ", direction_matrix2[i * N + j]);
		}
		fprintf(fp,"\n");
	}
	 fprintf(fp, "Max index: %d\n", *max_index);
	fclose(fp);

	/************************ Free Memory ************************/
	free(direction_matrix);
	free(max_index);
    free(similarity_matrix2);
    free(direction_matrix2);


	/************************ Compare results ************************/
	printf ("Comparing against output data \n");
	if (system("diff -w out.dat out.gold.dat > diff.txt")) {
		fprintf(stdout, "*******************************************\n");
		fprintf(stdout, "FAIL: Output DOES NOT match the golden output\n");
		fprintf(stdout, "*******************************************\n");
		return 1;
	} else {
		fprintf(stdout, "*******************************************\n");
		fprintf(stdout, "PASS: The output matches the golden output!\n");
		fprintf(stdout, "*******************************************\n");
		return 0;
	}

// printf("cnt_ops=%d, cnt_bytes=%d\n", cnt_ops, cnt_bytes);
}

