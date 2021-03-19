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

}  // ELF_Def
