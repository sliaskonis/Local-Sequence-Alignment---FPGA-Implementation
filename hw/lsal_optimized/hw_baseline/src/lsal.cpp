//

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

#define N 128
#define M 307200

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
#pragma HLS TOP name=compute_matrices
		int it = 0;
		int max_index_temp = 0;
		char dir = 0;
		char dir_buf[2*N];
#pragma HLS ARRAY_PARTITION variable=dir_buf dim=1 factor=8 cyclic
		short max_value = 0;
		// Arrays
		int max_index_arr[N];
#pragma HLS BIND_STORAGE variable=max_index_arr type=ram_t2p impl=bram
#pragma HLS ARRAY_PARTITION variable=max_index_arr dim=1 factor=4 cyclic
		short max_value_arr[N];
#pragma HLS BIND_STORAGE variable=max_value_arr type=ram_t2p impl=bram
#pragma HLS ARRAY_PARTITION variable=max_value_arr dim=1 factor=4 cyclic
		// Buffers
		char query_buf[N];
		char database_buf[N+1];
#pragma HLS ARRAY_PARTITION variable=database_buf dim=1 factor=8 cyclic
		// Compare values
		short match = 0;
		short val_west = 0;
		short val_north = 0;
		short val_northwest = 0;
		short val;

		// Similarity buffers
		short sim_buf_1[N+1];
#pragma HLS ARRAY_PARTITION variable=sim_buf_1 dim=1 complete
		short sim_buf_2[N+1];
#pragma HLS ARRAY_PARTITION variable=sim_buf_2 dim=1 complete
		short sim_buf_3[N+1];
#pragma HLS ARRAY_PARTITION variable=sim_buf_3 dim=1 complete


		// Copy the first N characters of the database to the database buffer
		memcpy(database_buf+1, database, N*sizeof(char));
		memcpy(query_buf, query, N*sizeof(char));
		memset(dir_buf, 0, 2*N*sizeof(char));

		// Init similarity buffers
		memset(sim_buf_1, 0, (N+1)*sizeof(short));
		memset(sim_buf_2, 0, (N+1)*sizeof(short));
		memset(sim_buf_3, 0, (N+1)*sizeof(short));

		memset(max_value_arr, 0, N*sizeof(short));
		memset(max_index_arr, 0, N*sizeof(int));

		// Fill the direction matrix using the anti-diagonal buffer.
		row_loop: for (int j = N; j < M + 2*(N-1)+1; j++) {
#pragma HLS PIPELINE II=34

			for (int l = 0; l < N-1; l++) {
				database_buf[l] = database_buf[l+1];

			}
			database_buf[N-1] = database[j-1];
			col_loop: for (int i = 0; i < N; i++) {
#pragma HLS UNROLL

				it = j-N;

				match = (query_buf[i] == database_buf[N-i-1]) ? MATCH : MISS_MATCH;

				// Current buffer -> sim_buf_3
				val_west = sim_buf_2[i] + GAP_i;
				val_north = sim_buf_2[i+1] + GAP_d;
				val_northwest = sim_buf_1[i] + match;

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

				if (val > 0) {
					sim_buf_3[i+1] = val;
					dir_buf[(j%2)*N + i]=dir;
				}
				else {
					sim_buf_3[i+1] = 0;
					dir_buf[(j%2)*N + i] = CENTER;
				}

				// Calculate max index
				if (val > max_value_arr[i]) {
					max_value_arr[i] = val;
					max_index_arr[i] = (it-i)*N + i;
				}
			}
			memcpy(direction_matrix+((it*N)), dir_buf+(j%2)*N, N*sizeof(char));

			// Copy buffers
			memcpy(sim_buf_1, sim_buf_2, (N+1)*sizeof(short));
			memcpy(sim_buf_2, sim_buf_3, (N+1)*sizeof(short));
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
