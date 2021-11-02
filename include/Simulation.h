#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include "define.h"
#include "struct.h"

void Simulation_CPU(LDPCCode *H, AWGNChannel *AWGN, Simulation *SIM, VN *Variablenode, CN *Checknode, int *Modulate_sym, int *CodeWord_sym, int *DecodeOutput);

int Statistic(Simulation *SIM, int *CodeWord_Frames, int *D, LDPCCode *LDPC);

void Get_H(LDPCCode *H, VN *Variablenode, CN *Checknode);

void free_VN_CN(LDPCCode* H, VN* Variablenode, CN* Checknode);

#endif
