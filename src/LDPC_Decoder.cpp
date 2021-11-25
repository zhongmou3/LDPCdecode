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
/***********************************************************************************************
*
* @brief    计算绝对值
* @explanation    计算绝对值
* @param    a
* @return   a的绝对值
************************************************************************************************/
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
/***********************************************************************************************
*
* @brief          计算变量节点中的参数
* @explanation    已知一个校验节点与一个变量节点相连
*                 知道这是第几个校验节点
*                 同时知道这个校验节点连接的变量节点是第几个
*				  计算对于这个变量节点来说，这是第几个连接他的校验节点             
* @param    CN *Checknode   校验节点的指针
*           int CNnum       第CNnum个校验节点
*           int index_in_linkVNS  校验节点连接的变量节点是第index_in_linkVNS个
*           VN *Variablenode   变量节点的指针
* @return   i--变量节点连接的第i个校验点
************************************************************************************************/
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
/***********************************************************************************************
*
* @brief          计算校验节点中的参数
* @explanation    已知一个变量节点与一个校验节点相连
*                 知道这是第几个变量节点
*                 同时知道这个变量节点连接的校验节点是第几个
*				  计算对于这个校验节点来说，这是第几个连接他的变量节点
* @param    VN *Variablenode--变量节点的指针
*           int VNnum--第VNnum个变量节点
*           int index_in_linkCNS--变量节点连接的校验节点是第index_in_linkVNS个
*           CN *Checknode--校验节点的指针
* @return   i--校验节点连接的第i个变量节点
************************************************************************************************/
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
/***********************************************************************************************
*
* @brief      寻找最小值和次小值   
* @explanation    已知第几个校验节点
*                 计算出对于这个校验节点来说，连接的变量节点传递的似然比信息的最小值和次小值
* @param       CN *Checknode--校验节点的指针
*			   VN *Variablenode--变量节点的指针
*              float &L_min--计算出来的最小值
*              float &L_submin--计算出来的次小值
*			   int &sign--符号
*              int row--这个校验节点是第row个
* @return   none
************************************************************************************************/
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
/***********************************************************************************************
*
* @brief      分层算法寻找最小值和次小值1
* @explanation    已知第几个校验节点
*                 计算出对于这个校验节点来说，连接的变量节点传递的似然比信息的最小值和次小值
*                 这个是分层算法特有的寻找最小值的函数
* @param       CN *Checknode--校验节点的指针
*			   VN *Variablenode--变量节点的指针
*              float &L_min--计算出来的最小值
*              float &L_submin--计算出来的次小值
*			   int &sign--符号
*              int row--这个校验节点是第row个
*              int &my_min_refresh_num--最小值更新了，变量节点是这个校验节点对应的第my_min_refresh_num个
*              int &my_submin_refresh_num--次小值更新了，变量节点是这个校验节点对应的第my_submin_refresh_num个
*              int min_refresh_num--之前的最小值所对应的变量节点是这个校验节点对应的第my_min_refresh_num个
* @return   none
************************************************************************************************/
void findmin_submin_new(CN* Checknode, VN* Variablenode, float& L_min, float& L_submin, int& sign, int row, int &my_min_refresh_num, int &my_submin_refresh_num,int min_refresh_num,int submin_refresh_num)
{
	L_min = FLT_MAX;
	L_submin = FLT_MAX;
	int pre_min_refresh_num= min_refresh_num;
	int pre_submin_refresh_num=submin_refresh_num; //这两个变量的意义在函数findmin_submin_for_layered中有说明，作用相同
	sign = 1;
	for (int i = 0; i < Checknode[row].weight; i++)
	{
		if (myabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]) < L_submin)
		{
			if (myabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]) < L_min)
			{
				L_submin = L_min;
				my_submin_refresh_num = pre_min_refresh_num;
				L_min = myabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]);
				my_min_refresh_num = i;
			}
			else
			{
				L_submin = myabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]);
				my_submin_refresh_num = i;
				my_min_refresh_num = pre_min_refresh_num;
			}
		}
		else
		{
			my_min_refresh_num = pre_min_refresh_num;
			my_submin_refresh_num = pre_submin_refresh_num;
		}
		pre_min_refresh_num=my_min_refresh_num;
		pre_submin_refresh_num = my_submin_refresh_num;
		//printf("min_refresh_num: %d %d\n", my_min_refresh_num, my_submin_refresh_num);
		if (Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)] < 0)
		{
			sign = sign * -1;
		}
	}
}
/***********************************************************************************************
*
* @brief      分层算法寻找最小值和次小值2
* @explanation    已知第几个校验节点
*                 计算出对于这个校验节点来说，连接的变量节点传递的似然比信息的最小值和次小值
*				  不过由于是分层算法，因此只要把这一层的变量节点消息和之前的比较即可
* @param       CN *Checknode--校验节点的指针
*			   VN *Variablenode--变量节点的指针
*              float &L_min--计算出来的最小值
*              float &L_submin--计算出来的次小值
*			   int &sign--符号
*              int row--这个校验节点是第row个
*			   int L--第L层
*			   float original_L_min--上一层计算得到的最小值
*			   float original_L_submin--上一层计算得到的次小值
*              int &my_min_refresh_num--最小值更新了，变量节点是这个校验节点对应的第my_min_refresh_num个
*              int &my_submin_refresh_num--次小值更新了，变量节点是这个校验节点对应的第my_submin_refresh_num个
*              int min_refresh_num--之前的最小值所对应的变量节点是这个校验节点对应的第my_min_refresh_num个
*              int &refresh_flag--为0说明最小值次小值没更新，否则更新了
* @return   none
************************************************************************************************/
void findmin_submin_for_layered(CN* Checknode, VN* Variablenode, float& L_min, float& L_submin, int& sign, int row,int L,float original_L_min,float original_L_submin, int& my_min_refresh_num, int& my_submin_refresh_num, int min_refresh_num,int submin_refresh_num,int &refresh_flag)
{
	L_min = original_L_min;
	L_submin = original_L_submin;
	if (0)//(myprint == 1)
	{
		if (row == 30)
		{
			printf("function_min: %f %f\n", L_min, L_submin);
			printf("min_refresh_num: %d\n", min_refresh_num);
		}
	}//调试用	
	refresh_flag = 0;
	//my_min_refresh_num和my_submin_refresh_num这个变量如果没有在下面的if语句里更新，在函数最后要更新一下
	//由于没注意到这个，花了很久debug
	//利用refresh_flag作为标志，0--两个都没更新，1--更新过了
	int pre_min_refresh_num = min_refresh_num;
	int pre_submin_refresh_num = submin_refresh_num;//这两个变量是为了防止有更新两次的情况，所以每次要更新一下最小值的位置
	sign = 1;
	for (int i = 0; i < Checknode[row].weight; i++)
	{
		if (Checknode[row].linkVNs[i] >= Z * (L-1) && Checknode[row].linkVNs[i] < Z * (L))//只要更新这一层即可
		{
			if (myabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]) < L_submin)
			{
				if (0)//(myprint == 1)
				{
					if (row == 30)
					{
						printf("row:  %d\n", row);
						printf("Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]):  %f\n", myabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]));
						printf("Checknode[row].linkVNs[i]:  %d\n", Checknode[row].linkVNs[i]);
						printf("index_in_VN(Checknode, row, i, Variablenode) %d\n", index_in_VN(Checknode, row, i, Variablenode));
					}
				}
				if (myabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]) < L_min)
				{
					L_submin = L_min;
					my_submin_refresh_num = pre_min_refresh_num;
					L_min = myabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]);
					my_min_refresh_num = i;
				}
				else
				{
					L_submin = myabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]);
					my_submin_refresh_num = i;
					my_min_refresh_num = pre_submin_refresh_num;
				}
				refresh_flag = 1;
			}
			else
			{
				my_min_refresh_num = pre_min_refresh_num;
				my_submin_refresh_num = pre_submin_refresh_num;
			}
			pre_min_refresh_num = my_min_refresh_num;
			pre_submin_refresh_num = my_submin_refresh_num;
			
		}
		if (Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)] < 0)
		{
			sign = sign * -1;
		}
	}
	if (refresh_flag == 0)
	{
		my_min_refresh_num = pre_min_refresh_num;
		my_submin_refresh_num = pre_submin_refresh_num;
	}
	if (0)//(myprint == 1)
	{
		if (row == 30)
		{
			printf("my_min_refresh_num: %d\n", my_min_refresh_num);
		}
	}

	
}
/***********************************************************************************************
*
* @brief      初始化信道输出
* @explanation   处理信道输出的公式很简单，2y(i)/(sigma*sigma),不过要考虑到打孔
*                打孔的参数punch在define.h里
* @param       LDPCCode *H--奇偶校验矩阵的参数的指针
*			   AWGNChannel *AWGN--AWGN信道的参数的指针
*              VN *Variablenode--变量节点的的指针
*              float *Modulate_sym_Channelout--信道输出
* @return   none
************************************************************************************************/
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
int Decoding_BP(LDPCCode* H, VN* Variablenode, CN* Checknode, int* DecodeOutput)
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

int Decoding_MS(LDPCCode* H, VN* Variablenode, CN* Checknode, int* DecodeOutput)
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
			//printf("%f %f\n", L_min, L_submin);
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
int Decoding_RowLayered_MS(LDPCCode* H, VN* Variablenode, CN* Checknode, int* DecodeOutput)
{
	for (int col = 0; col < H->Variablenode_num; col++)
	{
		Variablenode[col].LLR = Variablenode[col].L_ch;
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
	int row_layer_num = H->Checknode_num / Z;
	float L_min = 0;
	float L_submin = 0;
	int sign = 1;
	while (iter_number++ < maxIT)
	{
		for (int L = 0; L < row_layer_num; L++)
		{
			// message from var to check
			for (int col = 0; col < H->Variablenode_num; col++)
			{
				for (int dv = 0; dv < Variablenode[col].weight; dv++)
				{
					//if (Variablenode[col].linkCNs[dv] >= Z * L && Variablenode[col].linkCNs[dv] < Z * (L + 1))
					{
						Variablenode[col].L_v2c[dv] = Variablenode[col].LLR - Checknode[Variablenode[col].linkCNs[dv]].L_c2v[index_in_CN(Variablenode, col, dv, Checknode)];

					}
				}
			}
			// message from check to var
			for (int row = Z * L; row < Z * (L + 1); row++)
			{
				//find max and submax
				findmin_submin(Checknode, Variablenode, L_min, L_submin, sign, row);
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
			//变量节点消息之和
			for (int col = 0; col < H->Variablenode_num; col++)
			{
				for (int dv = 0; dv < Variablenode[col].weight; dv++)
				{
					if (Variablenode[col].linkCNs[dv] >= Z * L && Variablenode[col].linkCNs[dv] < Z * (L + 1))
					{
						Variablenode[col].LLR = Variablenode[col].L_v2c[dv] + Checknode[Variablenode[col].linkCNs[dv]].L_c2v[index_in_CN(Variablenode, col, dv, Checknode)];
						if (Variablenode[col].LLR > 0)
						{
							DecodeOutput[col] = 0;
						}
						else
						{
							DecodeOutput[col] = 1;
						}
					}
				}
			}
		}
		//hard decision
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
int Decoding_ColLayered_MS(LDPCCode* H, VN* Variablenode, CN* Checknode, int* DecodeOutput)
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
	int col_layer_num = H->Variablenode_num / Z;
	float L_min = 0;
	float L_submin = 0;
	int my_min_refresh_num = 0;
	int my_submin_refresh_num = 0;
	int sign = 1;
	float* original_L_min;
	float* original_L_submin;
	int* min_refresh_num;//首先，分层算法如果每一层都要把大小全部比一遍，那会浪费异常多的时间
	//因此有一种方法，就是只要第一次比较一下，然后把最小值记录下来，之后每一层只要和这个值比较大小就行了
	//那么这个flag是干什么的呢，当我们又找回到这个最小值的变量节点的时候，变量节点更新了，之前的最小值就不是最小值了
	//这个时候就又要全部比一遍了，因此需要这个变量来确定最小值的变量节点的位置
	int* submin_refresh_num;//这个变量同理
	int test = 0;
	original_L_min = (float*)malloc(H->Checknode_num * sizeof(float));
	original_L_submin = (float*)malloc(H->Checknode_num * sizeof(float));
	min_refresh_num = (int*)malloc(H->Checknode_num * sizeof(int));
	submin_refresh_num = (int*)malloc(H->Checknode_num * sizeof(int));
	for (int row = 0; row < H->Checknode_num; row++)
	{
		min_refresh_num[row] = 0;
		submin_refresh_num[row] = 0;
	}

	while (iter_number++ < maxIT)
	{
		/*for (int dc = 0; dc < Variablenode[17240].weight; dc++)
		{
			printf("%f ", Variablenode[17240].linkCNs[dc]);
		}
		printf("%d", Variablenode[17240]);*/
		if (myprint == 1)
		{
			printf("\n");
			printf("\n");
			printf("\n");
			printf("\n");
			printf("\n");
		}
		for (int L = 0; L < col_layer_num; L++)
		{
			//test = 0;
			// message from check to var


			for (int row = 0; row < H->Checknode_num; row++)
			{
				/*if (myprint == 1 && row == 30)
				{
					printf(" Checknode[row].linkVNs[min_refresh_num[row]] %d\n ", Checknode[row].linkVNs[min_refresh_num[row]]);
					printf("Checknode[row].linkVNs[submin_refresh_num[row]] %d\n ", Checknode[row].linkVNs[submin_refresh_num[row]]);
				}*/
				if (L == 0
					|| (Checknode[row].linkVNs[min_refresh_num[row]] >= Z * (L - 1) && Checknode[row].linkVNs[min_refresh_num[row]] < Z * (L))
					|| (Checknode[row].linkVNs[submin_refresh_num[row]] >= Z * (L - 1) && Checknode[row].linkVNs[submin_refresh_num[row]] < Z * (L)))       //如果这个变量节点就是之前找过的最小值，那就重新更新一下        
				{
					findmin_submin_new(Checknode, Variablenode, L_min, L_submin, sign, row, my_min_refresh_num, my_submin_refresh_num, min_refresh_num[row], submin_refresh_num[row]);
					min_refresh_num[row] = my_min_refresh_num;
					submin_refresh_num[row] = my_submin_refresh_num;
					original_L_min[row] = L_min;
					original_L_submin[row] = L_submin;
					if (myprint == 1)
					{
						if (row == 2835)
						{
							printf("iter_number: %d\n", iter_number);
							printf("%d %d %d\n", min_refresh_num[row], submin_refresh_num[row], 1);
							printf("%f %f\n", original_L_min[row], original_L_submin[row]);
							for (int dc = 0; dc < Checknode[row].weight; dc++)
							{
								printf("%f ", myabs(Variablenode[Checknode[row].linkVNs[dc]].L_v2c[index_in_VN(Checknode, row, dc, Variablenode)]));
							}

							printf("\n");
							printf("\n");
							FILE* fp_H;
							if (NULL == (fp_H = fopen("debugresult2.txt", "a")))
							{
								printf("can not open file: debugresult1.txt\n");
								exit(0);
							}
							fprintf(fp_H, "iter_number: %d\n", iter_number);
							fprintf(fp_H, "%d %d %d\n", min_refresh_num[row], submin_refresh_num[row], 1);
							fprintf(fp_H, "%f %f\n", original_L_min[row], original_L_submin[row]);
							for (int dc = 0; dc < Checknode[row].weight; dc++)
							{
								fprintf(fp_H, "%f ", myabs(Variablenode[Checknode[row].linkVNs[dc]].L_v2c[index_in_VN(Checknode, row, dc, Variablenode)]));
							}

							fprintf(fp_H, "\n");
							fprintf(fp_H, "\n");
							fclose(fp_H);
						}
					}

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
					//test++;
				}
				else
				{
					int refresh_flag = 0;//等于1更新

					findmin_submin_for_layered(Checknode, Variablenode, L_min, L_submin, sign, row, L, original_L_min[row], original_L_submin[row], my_min_refresh_num, my_submin_refresh_num, min_refresh_num[row], submin_refresh_num[row], refresh_flag);
					min_refresh_num[row] = my_min_refresh_num;
					submin_refresh_num[row] = my_submin_refresh_num;
					original_L_min[row] = L_min;
					original_L_submin[row] = L_submin;
					if (myprint == 1)
					{
						if (row == 2835)
						{
							printf("iter_number: %d\n", iter_number);
							printf("%d %d %d\n", min_refresh_num[row], submin_refresh_num[row], 0);
							printf("%f %f\n", original_L_min[row], original_L_submin[row]);
							for (int dc = 0; dc < Checknode[row].weight; dc++)
							{
								printf("%f ", myabs(Variablenode[Checknode[row].linkVNs[dc]].L_v2c[index_in_VN(Checknode, row, dc, Variablenode)]));
							}
							printf("\n");
							printf(" Z* (L - 1) %d\n ", Z * (L - 1));
							printf(" Z* (L) %d\n ", Z * (L));
							printf("%d\n", refresh_flag);
							printf("\n");
							FILE* fp_H;
							if (NULL == (fp_H = fopen("debugresult2.txt", "a")))
							{
								printf("can not open file: debugresult1.txt\n");
								exit(0);
							}
							fprintf(fp_H, "iter_number: %d\n", iter_number);
							fprintf(fp_H, "%d %d %d\n", min_refresh_num[row], submin_refresh_num[row], 0);
							fprintf(fp_H, "%f %f\n", original_L_min[row], original_L_submin[row]);
							for (int dc = 0; dc < Checknode[row].weight; dc++)
							{
								fprintf(fp_H, "%f ", myabs(Variablenode[Checknode[row].linkVNs[dc]].L_v2c[index_in_VN(Checknode, row, dc, Variablenode)]));
							}

							fprintf(fp_H, "\n");
							fprintf(fp_H, "\n");
							fclose(fp_H);
						}
					}
					if (refresh_flag == 1)
					{
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
					//test--;
				}
			}
			//变量节点消息之和
			for (int col = L * Z; col < (L + 1) * Z; col++)
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
			}
			// message from var to check
			for (int col = L * Z; col < (L + 1) * Z; col++)
			{
				for (int dv = 0; dv < Variablenode[col].weight; dv++)
				{
					Variablenode[col].L_v2c[dv] = Variablenode[col].LLR - Checknode[Variablenode[col].linkCNs[dv]].L_c2v[index_in_CN(Variablenode, col, dv, Checknode)];
				}
			}
			//printf("%d \n", test);
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



