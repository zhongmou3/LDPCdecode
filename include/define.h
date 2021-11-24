#ifndef _DEFINE_H_
#define _DEFINE_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//#include <conio.h>
#include <string.h>
#include <memory.h>
#include <time.h>
//#include <direct.h>
#include "struct.h"
#include "Simulation.h"


//#define Matrixfile "T_BG1_26_4_48.txt"
#define Matrixfile "Tanner_PON_LDPC.txt"


//LDPC译码器相关参数
#define maxIT 15      // LDPC译码器最大迭代次数.其中对Q值赋初值用了一次迭代
#define factor_BP (1)  // NMS算法中的修正因子.浮点译码器
#define factor_NMS (0.75) // NMS算法中的修正因子.浮点译码器

//AWGN参数
#define ix_define 173
#define iy_define 173
#define iz_define 173

#define Add_noise 1 // 0--No; 1--Yes
#define snrtype 0   // 0--Eb/No; 1--Es/No

//仿真参数
#define startSNR 3.8
#define stepSNR 0.1
#define stopSNR 4.0

#define leastErrorFrames 100 // 最少错误帧数
#define leastTestFrames 200  // 最少仿真帧数
#define displayStep 1000   // 定义将译码结果写入相应txt文件的频率

//CUDA c相应参数
#define PI (3.1415926)
#define Message_CW 0 // 提前终止和统计时只看信息位还是看整个码字:0->只看信息位;1->看整个码字

//打孔操作参数
#define Punch 2 // 为0时不需要打孔，为1时在前面打孔，为2时在后面打孔
#define Z 256

//调试用参数
#define myprint 0

#endif