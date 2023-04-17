// Elfo - a lightweight parser for the Executable and Linking Format
// Copyright 2021-2023 by Bernhard Heinloth <heinloth@cs.fau.de>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#ifdef USE_DLH
#include <dlh/string.hpp>
#else
#include <cstring>
#endif

#include <elfo/elf.hpp>

template<ELFCLASS C>
class ELF_Dyn : public ELF<C> {
	using Def = typename ELF_Def::Structures<C>;

 public:
	template<typename T>
	using Array = typename ELF<C>::template Array<T>;
	template<typename T>
	using List = typename ELF<C>::template List<T>;
	using Section = typename ELF<C>::Section;
	using Relocation = typename ELF<C>::Relocation;
	using SymbolTable = typename ELF<C>::SymbolTable;
	using DynamicTable = typename ELF<C>::DynamicTable;
	using VersionNeeded = typename ELF<C>::VersionNeeded;
	using VersionDefinition = typename ELF<C>::VersionDefinition;

	DynamicTable dyn;
	SymbolTable symbols;
	Array<Relocation> relocations;
	Array<Relocation> relocations_plt;
	List<VersionNeeded> version_needed;
	List<VersionDefinition> version_definition;

	explicit ELF_Dyn(uintptr_t start)
	  : ELF<C>(start),
	    dyn(this->dynamic()),
	    symbols(dyn.get_symbol_table()),
	    relocations(dyn.get_relocations()),
	    relocations_plt(dyn.get_relocations_plt()),
	    version_needed(dyn.get_version_needed()),
	    version_definition(dyn.get_version_definition()) {}

	uint16_t version_index(const char * name) const {
		for (auto & v : version_needed)
			for (auto & aux : v.auxiliary())
				if (strcmp(name, aux.name()) == 0)
					return aux.version_index();

		for (auto & v : version_definition)
			if (!v.base() && strcmp(name, v.auxiliary()[0].name()) == 0)
				return v.version_index();

		return Def::VER_NDX_GLOBAL;
	}

	const char * version_name(uint16_t index) const {
		switch (index) {
			case Def::VER_NDX_LOCAL:
				return "*local*";
			case Def::VER_NDX_GLOBAL:
				return "*global*";
			case Def::VER_NDX_ELIMINATE:
				return "*eliminate*";
		}
		for (auto & v : version_needed)
			for (auto & aux : v.auxiliary())
				if (index == aux.version_index())
					return aux.name();

		for (auto & v : version_definition)
			if (!v.base() && index == v.version_index())
				return v.auxiliary()[0].name();

		return "*invalid*";
	}
};
