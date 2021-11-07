#include "Simulation.h"
#include "LDPC_Encoder.h"
#include "LDPC_Decoder.h"
#include <assert.h>
#include <fstream>
#include <string>
#include "define.h"

/*
* 仿真函数
* AWGN:AWGNChannel类变量，包含噪声种子等
* 
*/
void Simulation_CPU(LDPCCode *H, AWGNChannel *AWGN, Simulation *SIM, VN *Variablenode, CN *Checknode, int *Modulate_sym, int *CodeWord_sym, int *DecodeOutput)
{

	float *Modulate_sym_Channelout;

	Modulate_sym_Channelout = (float *)malloc(H->bit_length * sizeof(float));

	while (SIM->num_Error_Frames < leastErrorFrames || SIM->num_Frames < leastTestFrames)
	{
		// printf("%d\n", SIM->num_Frames);
		SIM->num_Frames += 1;

		AWGNChannel_CPU(H, AWGN, Modulate_sym_Channelout, Modulate_sym);

		Demodulate(H, AWGN, Variablenode, Modulate_sym_Channelout);

		Decoding_Layered_MS(H, Variablenode, Checknode, DecodeOutput);

		//Decoding_MS(H, Variablenode, Checknode, DecodeOutput);

		//Decoding_BP(H, Variablenode, Checknode, DecodeOutput);

		Statistic(SIM, CodeWord_sym, DecodeOutput, H);
	}
	free(Modulate_sym_Channelout);
}

/*
* 统计函数，统计仿真结果
*/
int Statistic(Simulation *SIM, int *CodeWord_Frames, int *D, LDPCCode *H)
{
	int index1;
	int Error_msgBit = 0;

	for (index1 = 0; index1 < H->Variablenode_num; index1++)
	{
		Error_msgBit = (D[index1] != CodeWord_Frames[index1]) ? Error_msgBit + 1 : Error_msgBit;
	}
	SIM->num_Error_Bits += Error_msgBit;
	SIM->num_Error_Frames = (Error_msgBit != 0) ? SIM->num_Error_Frames + 1 : SIM->num_Error_Frames;

	SIM->Total_Iteration += H->iteraTime;

	if (SIM->num_Frames % displayStep == 0)
	{
		SIM->BER = ((double)SIM->num_Error_Bits / (double)(SIM->num_Frames)) / (double)(H->Variablenode_num);
		SIM->FER = (double)SIM->num_Error_Frames / (double)SIM->num_Frames;
		SIM->AverageIT = (double)SIM->Total_Iteration / (double)SIM->num_Frames;
		
		printf(" %.2f %8d  %4d  %6.4e  %6.4e  %.2f\n", SIM->SNR, SIM->num_Frames, SIM->num_Error_Frames, SIM->FER, SIM->BER, SIM->AverageIT);
		FILE *fp_H;
		if (NULL == (fp_H = fopen("results.txt", "a")))
		{
			printf("can not open file: results.txt\n");
			exit(0);
		}
		fprintf(fp_H, " %.2f %8d  %4d  %6.4e  %6.4e  %.2f  %6.4e %6.4e\n", SIM->SNR, SIM->num_Frames, SIM->num_Error_Frames, SIM->FER, SIM->BER, SIM->AverageIT);
		fclose(fp_H);
	}

	if (SIM->num_Error_Frames >= leastErrorFrames && SIM->num_Frames >= leastTestFrames)
	{
		SIM->BER = ((double)SIM->num_Error_Bits / (double)(SIM->num_Frames)) / (double)(H->Variablenode_num);
		SIM->FER = (double)SIM->num_Error_Frames / (double)SIM->num_Frames;
		SIM->AverageIT = (double)SIM->Total_Iteration / (double)SIM->num_Frames;
		// SIM->FER_Alarm = (double)SIM->num_Alarm_Frames / (double)SIM->num_Frames;
		// SIM->FER_False = (double)SIM->num_False_Frames / (double)SIM->num_Frames;
		printf(" %.2f %8d  %4d  %6.4e  %6.4e  %.2f\n", SIM->SNR, SIM->num_Frames, SIM->num_Error_Frames, SIM->FER, SIM->BER, SIM->AverageIT );
		FILE *fp_H;
		if (NULL == (fp_H = fopen("results.txt", "a")))
		{
			printf("can not open file: results.txt\n");
			exit(0);
		}
		fprintf(fp_H, " %.2f %8d  %4d  %6.4e  %6.4e  %.2f\n", SIM->SNR, SIM->num_Frames, SIM->num_Error_Frames, SIM->FER, SIM->BER, SIM->AverageIT );
		fclose(fp_H);
		return 1;
	}
	return 0;
}

/*
H:校验矩阵
Weight_Checknode:按顺序记录每个校验节点的重量
Weight_Variablenode:按顺序记录每个变量节点的重量
Address_Variablenode:变量节点相连的校验节点的序号
Address_Checknode:校验节点相连的变量节点的序号
*/
void Get_H(LDPCCode *H, VN *Variablenode, CN *Checknode)
{
	int index1;

	FILE *fp_H;

	if (NULL == (fp_H = fopen(Matrixfile, "r")))
	{
		printf("can not open file: %s\n", Matrixfile);
		exit(0);
	}

	fscanf(fp_H, "%d", &H->Variablenode_num); // 变量节点个数（行数）
	// Variablenode=(VN *)malloc(H->Variablenode_num*sizeof(VN));

	fscanf(fp_H, "%d", &H->Checknode_num); // 校验节点个数（列数）
	// Checknode=(CN *)malloc(H->Checknode_num*sizeof(CN));

	if (Punch == 0)
	{
		H->rate = (float)(H->Variablenode_num - H->Checknode_num) / H->Variablenode_num;
	}
	else 
	{
		H->rate = (float)(H->Variablenode_num - H->Checknode_num) / (H->Variablenode_num-2*Z);
	}

	H->bit_length = H->Variablenode_num;

	fscanf(fp_H, "%d", &H->maxWeight_variablenode); //变量节点相连的校验节点的最大个数

	fscanf(fp_H, "%d", &H->maxWeight_checknode); //校验节点相连的变量节点的最大个数

	for (int i = 0; i < H->Variablenode_num; i++)
	{
		fscanf(fp_H, "%d", &index1);
		Variablenode[i].weight = index1;
		Variablenode[i].linkCNs = (int *)malloc(Variablenode[i].weight * sizeof(int));
		Variablenode[i].L_v2c = (float *)malloc(Variablenode[i].weight * sizeof(float));
	}

	for (int i = 0; i < H->Checknode_num; i++)
	{
		fscanf(fp_H, "%d", &index1);
		Checknode[i].weight = index1;
		Checknode[i].linkVNs = (int *)malloc(Checknode[i].weight * sizeof(int));
		Checknode[i].L_c2v = (float *)malloc(Checknode[i].weight * sizeof(float));
	}

	for (int i = 0; i < H->Variablenode_num; i++)
	{
		for (int j = 0; j < Variablenode[i].weight; j++)
		{
			fscanf(fp_H, "%d", &index1);
			Variablenode[i].linkCNs[j] = index1 - 1;
		}
	}

	for (int i = 0; i < H->Checknode_num; i++)
	{
		for (int j = 0; j < Checknode[i].weight; j++)
		{
			fscanf(fp_H, "%d", &index1);
			Checknode[i].linkVNs[j] = index1 - 1;
		}
	}

	fclose(fp_H);
}

void free_VN_CN(LDPCCode* H, VN* Variablenode, CN* Checknode)
{
	for (int i = 0; i < H->Variablenode_num; i++)
	{
		free(Variablenode[i].linkCNs);
		free(Variablenode[i].L_v2c);
	}

	for (int i = 0; i < H->Checknode_num; i++)
	{
		free(Checknode[i].linkVNs);
		free(Checknode[i].L_c2v);
	}
}