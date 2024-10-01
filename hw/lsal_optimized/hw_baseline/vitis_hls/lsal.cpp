#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

#define N 32
#define M 65536

const short GAP_i = -1;
const short GAP_d = -1;
const short MATCH = 2;
const short MISS_MATCH = -1;
const short CENTER = 0;
const short NORTH = 1;
const short NORTH_WEST = 2;
const short WEST = 3;

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
	void compute_matrices(char query[N], char database[M+N-1], int *max_index, char direction_matrix[N* (N + M - 1)])
	{
#pragma HLS ARRAY_PARTITION dim=1 factor=8 type=cyclic variable=database
#pragma HLS ARRAY_PARTITION dim=1 factor=8 type=cyclic variable=direction_matrix
		int max_index_temp = 0;
		int max_index_arr[N];
#pragma HLS ARRAY_PARTITION dim=1 factor=8 type=cyclic variable=max_index_arr
#pragma HLS BIND_STORAGE variable=max_index_arr type=ram_t2p impl=bram
		char buffer[3*(N+1)];
#pragma HLS ARRAY_PARTITION dim=1 type=complete variable=buffer
		char max_value = 0;
		char max_value_arr[N];
#pragma HLS ARRAY_PARTITION dim=1 factor=8 type=cyclic variable=max_value_arr
#pragma HLS BIND_STORAGE variable=max_value_arr type=ram_t2p impl=bram
		char dir_buf[2*N];
#pragma HLS ARRAY_PARTITION dim=1 type=complete variable=dir_buf
		char database_buf[N+1];
// #pragma HLS BIND_STORAGE variable=database_buf type=ram_t2p impl=bram
// #pragma HLS ARRAY_PARTITION dim=1 factor=4 type=cyclic variable=database_buf
		int it = 0;
		short val = 0;
		char dir = 0;
		short match = 0;
		short val_west = 0;
		short val_north = 0;
		short val_northwest = 0;
		short val1;
		short val2;

		// Create buffer for query and database
		char query_buf[N];

		// Copy the first N characters of the database to the database buffer
		memcpy(database_buf+1, database, N*sizeof(char));

		memcpy(query_buf, query, N*sizeof(char));
		memset(dir_buf, 0, 2*N*sizeof(char));
		memset(buffer, 0, 3*(N+1)*sizeof(char));
		memset(max_value_arr, 0, N*sizeof(char));
		memset(max_index_arr, 0, N*sizeof(int));

		// Fill the direction matrix using the anti-diagonal buffer.
		row_loop: for (int j = N; j < M + 2*(N-1)+1; j++) {
#pragma HLS PIPELINE off

			for (int l = 0; l < N-1; l++) {
				database_buf[l] = database_buf[l+1];

			}
			database_buf[N-1] = database[j-1];
			col_loop: for (int i = 0; i < N; i++) {
#pragma HLS UNROLL

				it = j-N;
				int buffer_index = it % 3;
				switch (buffer_index) {
					case (0):
						val1 = 2;
						val2 = 1;
						break;
					case (1):
						val1 = -1;
						val2 = 1;
						break;
					case (2):
						val1 = -1;
						val2 = -2;
						break;
				}

				match = (query_buf[i] == database_buf[N-i-1]) ? MATCH : MISS_MATCH;

				// Calculate values
				val_west = buffer[(buffer_index+val1)*(N+1) + i] + GAP_i;
				val_north = buffer[(buffer_index+val1)*(N+1) + i+1] + GAP_d;
				val_northwest = buffer[(buffer_index+val2)*(N+1) +i] + match;

				if (val_northwest < val_north && val_north < val_west) {
					val = val_west;
					dir = WEST;
				} else if (val_northwest < val_north) {
					val = val_north;
					dir = NORTH;
				} else if (val_northwest < val_west) {
					val = val_west;
					dir = WEST;
				} else {
					val = val_northwest;
					dir = NORTH_WEST;
				}

				int indx = buffer_index*(N+1)+i+1;

				if (val > 0) {
					buffer[indx]=val;
					dir_buf[(j%2)*N + i]=dir;
				}
				else {
					buffer[indx] = 0;
					dir_buf[(j%2)*N + i] = CENTER;
				}

				// Calculate max index
				if (val > max_value_arr[i]) {
					max_value_arr[i] = val;
					max_index_arr[i] = (it-i)*N + i;
				}
			}
			memcpy(direction_matrix+((it*N)), dir_buf+(j%2)*N, N*sizeof(char));
		}

		max_loop: for(int k = 0; k < N; k++) {
			if (max_value_arr[k] > max_value) {
				max_value = max_value_arr[k];
				max_index_temp = max_index_arr[k];
			}
		}
		*max_index = max_index_temp;
	}
}