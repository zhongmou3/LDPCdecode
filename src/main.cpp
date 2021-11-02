#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"
#include "LDPC_Decoder.h"
#include "LDPC_Encoder.h"
#include "math.h"
#include "codeword.h"

int main()
{
	AWGNChannel *AWGN;
	AWGN = (AWGNChannel *)malloc(sizeof(AWGNChannel));
	Simulation *SIM;
	SIM = (Simulation *)malloc(sizeof(Simulation));

	CN *Checknode;	  // LDPC码各分块中校验节点的重量
	VN *Variablenode; // LDPC码各分块中变量节点的重量

	LDPCCode *H;
	H = (LDPCCode *)malloc(sizeof(LDPCCode));

	//	先读取行数和列数,分配空间
	FILE *fp_H;

	if (NULL == (fp_H = fopen(Matrixfile, "r")))
	{
		printf("can not open file: %s\n", Matrixfile);
		exit(0);
	}

	fscanf(fp_H, "%d", &H->Variablenode_num); // 变量节点个数（行数）
	Variablenode = (VN *)malloc(H->Variablenode_num * sizeof(VN));

	fscanf(fp_H, "%d", &H->Checknode_num); // 校验节点个数（列数）
	Checknode = (CN *)malloc(H->Checknode_num * sizeof(CN));

	fclose(fp_H);
	//
	Get_H(H, Variablenode, Checknode); //初始化剩下的参数


	int *DecodeOutput;
	DecodeOutput = (int *)malloc(H->Variablenode_num * sizeof(int));
	memset(DecodeOutput, 0, H->Variablenode_num * sizeof(int));


	int *Modulate_sym;
	Modulate_sym = (int *)malloc(H->bit_length * sizeof(int));

	Modulate(H, Modulate_sym, CodeWord_sym);

	for (SIM->SNR = startSNR; SIM->SNR <= stopSNR; SIM->SNR += stepSNR)
	{
		AWGN->seed[0] = ix_define;
		AWGN->seed[1] = iy_define;
		AWGN->seed[2] = iz_define;
		AWGN->sigma = 0;

		if (snrtype == 0)
		{
			AWGN->sigma = (float)sqrt(0.5 / (H->rate * (pow(10.0, (SIM->SNR / 10.0))))); //(float)LDPC->msgLen / LDPC->codewordLen;
		}
		else if (snrtype == 1)
		{
			AWGN->sigma = (float)sqrt(0.5 / (pow(10.0, (SIM->SNR / 10.0))));
		}

		SIM->num_Frames = 0; // 重新开始统计
		SIM->num_Error_Frames = 0;
		SIM->num_Error_Bits = 0;
		SIM->Total_Iteration = 0;
		SIM->num_False_Frames = 0;
		SIM->num_Alarm_Frames = 0;

		// BPSK(H,BPSK_Out,CodeWord);

		Simulation_CPU(H, AWGN, SIM, Variablenode, Checknode, Modulate_sym, CodeWord_sym, DecodeOutput);

		// for(int i=0;i<H->Variablenode_num;i++)
		// {
		// 	printf("%f + %f i\n",CComplex_sym_Channelout[i].Real,CComplex_sym_Channelout[i].Image);
		// }
		// printf("\n");
		// exit(0);
	}
	free_VN_CN(H, Variablenode,Checknode);
	free(AWGN);
	free(SIM);
	free(H);
	free(Checknode);
	free(Variablenode);
	free(Modulate_sym);
	free(DecodeOutput);

	return 0;
}