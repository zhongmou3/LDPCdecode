#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"
#include "LDPC_Decoder.h"
#include "LDPC_Encoder.h"
#include "math.h"

int CodeWord_sym[codelength];

void Calculate(LDPCCode* H, int* Modulate_sym, int* CodeWord_sym)
{
	for (int s = 0; s < H->bit_length; s++)
	{
		Modulate_sym[s] = 1 - 2 * CodeWord_sym[s];
		// printf("%d ", Modulate_sym[s]);
	}
	// exit(0);
}