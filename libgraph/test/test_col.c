#include "stdio.h"

#define SIZE_BY_INT   16


#define INVALID_FLAG (0xffffffff)

int main(int argc,  char **argv)
{
#if 0
	int test_array[16] = {
		12,
		12,
		13,
		45,
		45,
		46,
		57,
		59,
		59,
		59,
		59,
		71,
		72,
		73,
		74,
		75
	};
#else
	int test_array[16] = {
		12,
		12,
		12,
		12,
		12,
		12,
		12,
		12,
		12,
		12,
		12,
		12,
		12,
		12,
		12,
		75
	};
#endif

	int ori[SIZE_BY_INT];
	int array[SIZE_BY_INT][SIZE_BY_INT];
	int tmp_array[SIZE_BY_INT][SIZE_BY_INT];

	int mask[SIZE_BY_INT];





	for (int j = 0; j < SIZE_BY_INT; j++)
	{
		ori[j] = test_array[j];

	}

	array[0][0] = ori[0];
	mask[0] = 1;
	for (int j = 1; j < SIZE_BY_INT; j++)
	{
		if (ori[j - 1] != ori[j])
		{
			array[0][j] = ori[j];
			mask[j] = 1;
		}
		else
		{
			array[0][j] = INVALID_FLAG;
			mask[j] = 0;
		}
	}
	int level_1_sum[8];

	for (int j = 0; j < 8; j++)
	{
		level_1_sum[j] = mask[2 * j] + mask[2 * j + 1];
	}

	int level_2_sum[4];

	for (int j = 0; j < 4; j++)
	{
		level_2_sum[j] = level_1_sum[2 * j] + level_1_sum[2 * j + 1];
	}

	int level_3_sum[2];

	for (int j = 0; j < 2; j++)
	{
		level_3_sum[j] = level_2_sum[2 * j] + level_2_sum[2 * j + 1];
	}

	int result = level_3_sum[0] + level_3_sum[1];



	for (int j = 0; j < SIZE_BY_INT; j++)
	{
		printf("%d %d \n", test_array[j], array[0][j]);
	}
	printf("----------------------\n");
#if 1
	for (int i = 1; i < 16; i++)
	{
//#pragma HLS PIPELINE
		{
//#pragma HLS latency min=1 max=1
			for (int j = 0; j < SIZE_BY_INT - 1 ; j++)
			{
//#pragma HLS UNROLL
				if (array[i - 1][j] == INVALID_FLAG)
				{
					tmp_array[i][j] = array[i - 1][j + 1];
				}
				else
				{
					tmp_array[i][j] = array[i - 1][j];
				}

			}
			if (array[i - 1][SIZE_BY_INT - 1] == INVALID_FLAG)
			{
				tmp_array[i][SIZE_BY_INT - 1] = INVALID_FLAG;
			}
			else
			{
				tmp_array[i][SIZE_BY_INT - 1] = array[i - 1][SIZE_BY_INT - 1];
			}
		}
		{
//#pragma HLS latency min=1 max=1
			array[i][0] = tmp_array[i][0];
			for (int j = 1; j < SIZE_BY_INT; j++)
			{

//#pragma HLS UNROLL
				if (tmp_array[i][j] == tmp_array[i][j - 1])
				{
					array[i][j] = INVALID_FLAG;
				}
				else
				{
					array[i][j] = tmp_array[i][j];
				}

			}
		}
		for (int k = 0; k < SIZE_BY_INT ; k++)
			printf("%d \n", array[i][k]);
		printf("----------------------\n");
	}

	printf("size %d\n", result );
}
#endif
