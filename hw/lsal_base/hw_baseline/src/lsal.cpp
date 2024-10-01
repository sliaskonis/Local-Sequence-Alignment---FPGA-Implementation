#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

#define N 8
#define M 9

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
extern "C" {
	void compute_matrices(
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
}
