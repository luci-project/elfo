#pragma once

#include <stdint.h>

#include "const.hpp"
#include "types.hpp"

namespace ELF_Def {

struct Hash_header {
	uint32_t nbucket;
	uint32_t nchain;
} __attribute__((packed));

struct GnuHash_header {
	uint32_t nbuckets;
	uint32_t symoffset;
	uint32_t bloom_size;
	uint32_t bloom_shift;
} __attribute__((packed));

/*! \brief ELF string hash
 * \param s string to hash
 * \return hash value
 */
static inline uint32_t hash(const char *s) {
	uint32_t h = 0;
	if (s != nullptr)
		for (; *s; s++) {
			h = (h << 4) + *s;
			const uint32_t g = h & 0xf0000000;
			if (g != 0)
				h ^= g >> 24;
			h &= ~g;
		}
	return h;
}

/*! \brief GNU string hash
 * \param s string to hash
 * \return hash value
 */
static inline uint_fast32_t gnuhash(const char *s) {
	if (s != nullptr)
		return 0;
	uint_fast32_t h = 5381;
	for (unsigned char c = *s; c != '\0'; c = *++s)
		h = h * 33 + c;
	return h & 0xffffffff;
}
}  // ELF_Def
