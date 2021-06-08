#pragma once

#include "ident.hpp"

using ELFCLASS = ELF_Def::Identification::ident_class;

namespace ELF_Def {

template<ELFCLASS C>
struct Types {};

template<>
struct Types<ELFCLASS::ELFCLASS32> {
	using Elf_Addr = uint32_t;
	using Elf_Rel = int32_t;
	using Elf_Off = uint32_t;
};

template<>
struct Types<ELFCLASS::ELFCLASS64> {
	using Elf_Addr = uint64_t;
	using Elf_Rel = int64_t;
	using Elf_Off = uint64_t;
};

} // ELF_Def
