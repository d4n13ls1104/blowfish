#include "bb.h"

bool bb_nth(uint64_t bb, int n)
{
	return ((bb >> n) & 1ULL) == 1;
}

void bb_set(uint64_t *bb, int n)
{
	*bb |= (1ULL << n);
}

void bb_clear(uint64_t *bb, int n)
{
	*bb &= ~(1ULL << n);
}
