#ifndef _LDPC_DECODER_H
#define _LDPC_DECODER_H

#include "define.h"
#include "struct.h"

void Demodulate(LDPCCode *H, AWGNChannel *AWGN, VN *Variablenode, float *Modulate_sym_Channelout);

int Decoding_BP(LDPCCode *H, VN *Variablenode, CN *Checknode, int *DecodeOutput);

int Decoding_MS(LDPCCode *H, VN *Variablenode, CN *Checknode, int *DecodeOutput);

#endif