#ifndef _BB_H
#define _BB_H

#include <stdint.h>
#include <stdbool.h>

void bb_set(uint64_t *bb, int n);
void bb_clear(uint64_t *bb, int n);
bool bb_nth(uint64_t bb, int n);

#endif
