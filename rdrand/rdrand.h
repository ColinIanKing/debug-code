#ifndef __RDRND_H__
#define __RDRND_H__

#include <stdint.h>

void rdrand16(uint16_t *val);
void rdrand32(uint32_t *val);
void rdrand64(uint64_t *val);

#endif
