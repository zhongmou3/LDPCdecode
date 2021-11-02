#ifndef _LDPC_ENCODER_H
#define _LDPC_ENCODER_H

#include "define.h"
#include "struct.h"

void Modulate(LDPCCode *H, int *Modulate_sym, int *CodeWord_sym);

void AWGNChannel_CPU(LDPCCode *H, AWGNChannel *AWGN, float *Modulate_sym_Channelout, int *Modulate_sym);

float RandomModule(int *seed);

#endif