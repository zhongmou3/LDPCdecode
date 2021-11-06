#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits>
#include <string.h>
#include <memory.h>
#include <time.h>
#include "define.h"
#include "struct.h"
#include "LDPC_Decoder.h"
#include "float.h"

float myabs(float a)
{
	if (a < 0)
	{
		return -a;
	}
	else
	{
		return a;
	}
}
int index_in_VN(CN *Checknode, int CNnum, int index_in_linkVNS, VN *Variablenode)
{
	for (int i = 0; i < Variablenode[Checknode[CNnum].linkVNs[index_in_linkVNS]].weight; i++)
	{
		if (Variablenode[Checknode[CNnum].linkVNs[index_in_linkVNS]].linkCNs[i] == CNnum)
		{
			return i;
		}
	}
	printf("index_in_VN error\n");
	exit(0);
}

int index_in_CN(VN *Variablenode, int VNnum, int index_in_linkCNS, CN *Checknode)
{
	for (int i = 0; i < Checknode[Variablenode[VNnum].linkCNs[index_in_linkCNS]].weight; i++)
	{
		if (Checknode[Variablenode[VNnum].linkCNs[index_in_linkCNS]].linkVNs[i] == VNnum)
		{
			return i;
		}
	}
	printf("index_in_CN error\n");
	exit(0);
}

void findmin_submin(CN *Checknode, VN *Variablenode, float &L_min, float &L_submin, int &sign, int row)
{
	L_min = FLT_MAX;
	L_submin = FLT_MAX;
	sign = 1;
	for (int i = 0; i < Checknode[row].weight; i++)
	{
		if (myabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]) < L_submin)
		{
			if (myabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]) < L_min)
			{
				L_submin = L_min;
				L_min = myabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]);
			}
			else
			{
				L_submin = myabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]);
			}
		}
		if (Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)] < 0)
		{
			sign = sign * -1;
		}
	}
}

void findmin_submin_for_layered(CN* Checknode, VN* Variablenode, float& L_min, float& L_submin, int& sign, int row,int L)
{
	L_min = FLT_MAX;
	L_submin = FLT_MAX;
	sign = 1;
	for (int i = 0; i < Checknode[row].weight; i++)
	{
		if (Checknode[row].linkVNs[i] >= Z * L && Checknode[row].linkVNs[i] < Z * (L + 1))
		{
			if (myabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]) < L_submin)
			{
				if (myabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]) < L_min)
				{
					L_submin = L_min;
					L_min = myabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]);
				}
				else
				{
					L_submin = myabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]);
				}
			}
			if (Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)] < 0)
			{
				sign = sign * -1;
			}
		}	
	}
}

void Demodulate(LDPCCode *H, AWGNChannel *AWGN, VN *Variablenode, float *Modulate_sym_Channelout)
{
	if (Punch == 0)
	{
		for (int s = 0; s < H->Variablenode_num; s++)
		{
			Variablenode[s].L_ch = 2 * Modulate_sym_Channelout[s] / (AWGN->sigma * AWGN->sigma);
		}
	}
	if (Punch == 1)
	{
		for (int s = 0; s < 2 * Z; s++)
			Variablenode[s].L_ch = 0;
		for (int s = 2 * Z; s < H->Variablenode_num; s++)
		{
			Variablenode[s].L_ch = 2 * Modulate_sym_Channelout[s] / (AWGN->sigma * AWGN->sigma);
		}
	}
	if (Punch == 2)
	{
		for (int s = 0; s < H->Variablenode_num - 2 * Z; s++)
			Variablenode[s].L_ch = 2 * Modulate_sym_Channelout[s] / (AWGN->sigma * AWGN->sigma);
		for (int s = H->Variablenode_num - 2 * Z; s < H->Variablenode_num; s++)
		{
			Variablenode[s].L_ch = 0;
		}
	}
}
int Decoding_BP(LDPCCode *H, VN *Variablenode, CN *Checknode, int *DecodeOutput)
{
	for (int col = 0; col < H->Variablenode_num; col++)
	{
		for (int d = 0; d < Variablenode[col].weight; d++)
		{
			Variablenode[col].L_v2c[d] = Variablenode[col].L_ch;
		}
	}
	for (int row = 0; row < H->Checknode_num; row++)
	{
		for (int d = 0; d < Checknode[row].weight; d++)
		{
			Checknode[row].L_c2v[d] = 0;
		}
	}

	int iter_number = 0;
	bool decode_correct = true;
	while (iter_number++ < maxIT)
	{
		// printf("it_time: %d\n",iter_number);
		for (int col = 0; col < H->Variablenode_num; col++)
		{
			for (int d = 0; d < Variablenode[col].weight; d++)
			{
				Variablenode[col].LLR = Variablenode[col].L_ch;
			}
		}
		for (int col = 0; col < H->Variablenode_num; col++)
		{
			for (int d = 0; d < Variablenode[col].weight; d++)
			{
				Variablenode[col].LLR += Checknode[Variablenode[col].linkCNs[d]].L_c2v[index_in_CN(Variablenode, col, d, Checknode)];
			}
			if (Variablenode[col].LLR > 0)
			{
				DecodeOutput[col] = 0;
			}
			else
			{
				DecodeOutput[col] = 1;
			}
			// printf("%d ", DecodeOutput[col]);
		}
		// printf("\n");
		// exit(0);

		decode_correct = true;
		int sum_temp = 0;
		for (int row = 0; row < H->Checknode_num; row++)
		{
			for (int i = 0; i < Checknode[row].weight; i++)
			{
				sum_temp = sum_temp ^ DecodeOutput[Checknode[row].linkVNs[i]];
			}
			if (sum_temp)
			{
				decode_correct = false;
				break;
			}
		}
		if (decode_correct)
		{
			H->iteraTime = iter_number - 1;
			return 1;
		}

		// message from var to check
		for (int col = 0; col < H->Variablenode_num; col++)
		{
			for (int dv = 0; dv < Variablenode[col].weight; dv++)
			{

				Variablenode[col].L_v2c[dv] = Variablenode[col].LLR - Checknode[Variablenode[col].linkCNs[dv]].L_c2v[index_in_CN(Variablenode, col, dv, Checknode)];
			}
		}

		// message from check to var
		for (int row = 0; row < H->Checknode_num; row++)
		{
			double q = 1;
			for (int dc = 0; dc < Checknode[row].weight; dc++)
			{
				double temp = tanh(Variablenode[Checknode[row].linkVNs[dc]].L_v2c[index_in_VN(Checknode, row, dc, Variablenode)] / 2);
				if (temp >= 1)
				{
					temp = 1 - std::numeric_limits<double>::epsilon();
				}
				else if (temp <= -1)
				{
					temp = -1 + std::numeric_limits<double>::epsilon();
					;
				}
				q *= temp;
			}
			for (int dc = 0; dc < Checknode[row].weight; dc++)
			{

				double temp = q / tanh(Variablenode[Checknode[row].linkVNs[dc]].L_v2c[index_in_VN(Checknode, row, dc, Variablenode)] / 2);
				if (temp >= 1)
				{
					temp = 1 - std::numeric_limits<double>::epsilon();
				}
				else if (temp <= -1)
				{
					temp = -1 + std::numeric_limits<double>::epsilon();
					;
				}
				Checknode[row].L_c2v[dc] = 2 * atanh(temp);
				// Checknode[row].L_c2v[dc] *= factor_BP; //修正
			}
		}
	}
	H->iteraTime = iter_number - 1;
	return 0;
}
int Decoding_Layered_MS(LDPCCode* H, VN* Variablenode, CN* Checknode, int* DecodeOutput)
{
	for (int col = 0; col < H->Variablenode_num; col++)
	{
		for (int d = 0; d < Variablenode[col].weight; d++)
		{
			Variablenode[col].L_v2c[d] = Variablenode[col].L_ch;
		}
	}
	for (int row = 0; row < H->Checknode_num; row++)
	{
		for (int d = 0; d < Checknode[row].weight; d++)
		{
			Checknode[row].L_c2v[d] = 0;
		}
	}

	int iter_number = 0;
	bool decode_correct = true;
	int col_layer_num = H->Variablenode_num/Z;
	while (iter_number++ < maxIT)
	{
		for (int L = 0; L < col_layer_num; L++)
		{
			for (int col = L*Z; col < (L+1)*Z; col++)
			{
				for (int d = 0; d < Variablenode[col].weight; d++)
				{
					Variablenode[col].LLR = Variablenode[col].L_ch;
				}
			}
			for (int col = L * Z; col < (L + 1) * Z; col++)
			{
				for (int d = 0; d < Variablenode[col].weight; d++)
				{
					Variablenode[col].LLR += Checknode[Variablenode[col].linkCNs[d]].L_c2v[index_in_CN(Variablenode, col, d, Checknode)];
				}
				if (Variablenode[col].LLR > 0)
				{
					DecodeOutput[col] = 0;
				}
				else
				{
					DecodeOutput[col] = 1;
				}
				// printf("%d ", DecodeOutput[col]);
			}
			// printf("\n");
			// exit(0);

			

			// message from var to check
			for (int col = 0; col < H->Variablenode_num; col++)
			{
				for (int dv = 0; dv < Variablenode[col].weight; dv++)
				{

					Variablenode[col].L_v2c[dv] = Variablenode[col].LLR - Checknode[Variablenode[col].linkCNs[dv]].L_c2v[index_in_CN(Variablenode, col, dv, Checknode)];
				}
			}

			float L_min = 0;
			float L_submin = 0;
			int sign = 1;
			
			// message from check to var
			for (int row = 0; row < H->Checknode_num; row++)
			{

				//find max and submax
				findmin_submin_for_layered(Checknode, Variablenode, L_min, L_submin, sign, row, L);
				// printf("%f %f\n", L_min, L_submin);
				// exit(0);
				for (int dc = 0; dc < Checknode[row].weight; dc++)
				{
					if (Checknode[row].linkVNs[dc] >= Z * L && Checknode[row].linkVNs[dc] < Z * (L + 1))
					{
						if (L_min != FLT_MAX&& L_submin!= FLT_MAX)//最小值和次小值都找到了
						{
							if (myabs(Variablenode[Checknode[row].linkVNs[dc]].L_v2c[index_in_VN(Checknode, row, dc, Variablenode)]) != L_min)
							{
								if (Variablenode[Checknode[row].linkVNs[dc]].L_v2c[index_in_VN(Checknode, row, dc, Variablenode)] >= 0)
								{
									Checknode[row].L_c2v[dc] = sign * L_min;
								}
								else
								{
									Checknode[row].L_c2v[dc] = -sign * L_min;
								}
							}
							else
							{
								if (Variablenode[Checknode[row].linkVNs[dc]].L_v2c[index_in_VN(Checknode, row, dc, Variablenode)] >= 0)
								{
									Checknode[row].L_c2v[dc] = sign * L_submin;
								}
								else
								{
									Checknode[row].L_c2v[dc] = -sign * L_submin;
								}
							}
						}
					}
					
					Checknode[row].L_c2v[dc] *= factor_NMS;
				}
			}
		}
		//Hard decision
		decode_correct = true;
		int sum_temp = 0;
		for (int row = 0; row < H->Checknode_num; row++)
		{
			for (int i = 0; i < Checknode[row].weight; i++)
			{
				sum_temp = sum_temp ^ DecodeOutput[Checknode[row].linkVNs[i]];
			}
			if (sum_temp)
			{
				decode_correct = false;
				break;
			}
		}
		if (decode_correct)
		{
			H->iteraTime = iter_number - 1;
			return 1;
		}
	}
	H->iteraTime = iter_number - 1;
	return 0;
}
int Decoding_MS(LDPCCode *H, VN *Variablenode, CN *Checknode, int *DecodeOutput)
{
	for (int col = 0; col < H->Variablenode_num; col++)
	{
		for (int d = 0; d < Variablenode[col].weight; d++)
		{
			Variablenode[col].L_v2c[d] = Variablenode[col].L_ch;
		}
	}
	for (int row = 0; row < H->Checknode_num; row++)
	{
		for (int d = 0; d < Checknode[row].weight; d++)
		{
			Checknode[row].L_c2v[d] = 0;
		}
	}

	int iter_number = 0;
	bool decode_correct = true;
	while (iter_number++ < maxIT)
	{
		// printf("it_time: %d\n",iter_number);
		for (int col = 0; col < H->Variablenode_num; col++)
		{
			for (int d = 0; d < Variablenode[col].weight; d++)
			{
				Variablenode[col].LLR = Variablenode[col].L_ch;
			}
		}
		for (int col = 0; col < H->Variablenode_num; col++)
		{
			for (int d = 0; d < Variablenode[col].weight; d++)
			{
				Variablenode[col].LLR += Checknode[Variablenode[col].linkCNs[d]].L_c2v[index_in_CN(Variablenode, col, d, Checknode)];
			}
			if (Variablenode[col].LLR > 0)
			{
				DecodeOutput[col] = 0;
			}
			else
			{
				DecodeOutput[col] = 1;
			}
			// printf("%d ", DecodeOutput[col]);
		}
		// printf("\n");
		// exit(0);

		decode_correct = true;
		int sum_temp = 0;
		for (int row = 0; row < H->Checknode_num; row++)
		{
			for (int i = 0; i < Checknode[row].weight; i++)
			{
				sum_temp = sum_temp ^ DecodeOutput[Checknode[row].linkVNs[i]];
			}
			if (sum_temp)
			{
				decode_correct = false;
				break;
			}
		}
		if (decode_correct)
		{
			H->iteraTime = iter_number - 1;
			return 1;
		}

		// message from var to check
		for (int col = 0; col < H->Variablenode_num; col++)
		{
			for (int dv = 0; dv < Variablenode[col].weight; dv++)
			{

				Variablenode[col].L_v2c[dv] = Variablenode[col].LLR - Checknode[Variablenode[col].linkCNs[dv]].L_c2v[index_in_CN(Variablenode, col, dv, Checknode)];
			}
		}

		float L_min = 0;
		float L_submin = 0;
		int sign = 1;

		// message from check to var
		for (int row = 0; row < H->Checknode_num; row++)
		{
			//find max and submax
			findmin_submin(Checknode, Variablenode, L_min, L_submin, sign, row);
			// printf("%f %f\n", L_min, L_submin);
			// exit(0);
			for (int dc = 0; dc < Checknode[row].weight; dc++)
			{
				if (myabs(Variablenode[Checknode[row].linkVNs[dc]].L_v2c[index_in_VN(Checknode, row, dc, Variablenode)]) != L_min)
				{
					if (Variablenode[Checknode[row].linkVNs[dc]].L_v2c[index_in_VN(Checknode, row, dc, Variablenode)] >= 0)
					{
						Checknode[row].L_c2v[dc] = sign * L_min;
					}
					else
					{
						Checknode[row].L_c2v[dc] = -sign * L_min;
					}
				}
				else
				{
					if (Variablenode[Checknode[row].linkVNs[dc]].L_v2c[index_in_VN(Checknode, row, dc, Variablenode)] >= 0)
					{
						Checknode[row].L_c2v[dc] = sign * L_submin;
					}
					else
					{
						Checknode[row].L_c2v[dc] = -sign * L_submin;
					}
				}
				Checknode[row].L_c2v[dc] *= factor_NMS;
			}
		}
	}
	H->iteraTime = iter_number - 1;
	return 0;
}
