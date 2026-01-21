#ifndef BITSET_H
#define BITSET_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t *bitset;
#define div8_round_up(_ui) (((_ui)+7)/8)
#define bitset_create_zeroed(_N) (calloc(div8_round_up(_N), sizeof(uint8_t)))
#define bitset_destroy(_bset) (free(_bset))
static inline bool bitset_is_set_at(bitset bset, size_t i)
{
	return (bset[i/8] & (1 << i%8)) != 0;
}
#define bitset_is_set_at(_bset, _i) (((_bset)[(_i)/8] & (1 << (_i)%8)) != 0)
#define bitset_set_at(_bset, _i) ((_bset)[(_i)/8] |= (1 << (_i)%8))
#define bitset_clear_at(_bset, _i) ((_bset)[(_i)/8] &= ~(1 << (_i)%8))
#define bitset_toggle_at(_bset, _i) ((_bset)[(_i)/8] ^= (1 << (_i)%8))

#endif /* BITSET_H */
