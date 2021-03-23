#pragma once

#include <string>
#include <vector>
#include <map>

#include "elf.hpp"

template<ELFCLASS C>
class ELF_Dyn : public ELF<C> {
	template<typename T>
	using Array = typename ELF<C>::template Array<T>;
	using Section = typename ELF<C>::Section;
	using Dynamic = typename ELF<C>::Dynamic;
	using SymbolTable = typename ELF<C>::SymbolTable;

	using Def = typename ELF_Def::Structures<C>;
	using elfptr_t = typename Def::Elf_Addr;

	std::map<uint16_t, std::string> version_idx_to_name;
	std::map<std::string, uint16_t> version_name_to_idx;

	/*! \brief Find dynamic */
	constexpr auto find_dynamic() const {
		for (auto & section : this->sections)
			if (section.type() == Def::SHT_DYNAMIC)
				return section.get_dynamic();
		return this->template get<Dynamic>();
	}

	/*! \brief Find dyanmic symbol table
	 * Use hash if possible
	 * \return Symbol Table
	 */
	constexpr auto find_dynamic_symbol_table() const {
		uintptr_t offsets[3] = { 0 };
		uintptr_t version = 0;

		for (auto &dyn: dynamic) {
			switch(dyn.tag()) {
				case Def::DT_GNU_HASH:
					offsets[0] = dyn.value();
					break;

				case Def::DT_HASH:
					offsets[1] = dyn.value();
					break;

				case Def::DT_SYMTAB:
					offsets[2] = dyn.value();
					break;

				case Def::SHT_GNU_VERSYM:
					version = dyn.value();
					break;
			}
		}

		for (auto & offset : offsets)
			if (offset != 0) {
				const Section & section = this->section_by_offset(offset);
				assert(section.type() == Def::SHT_GNU_HASH || section.type() == Def::SHT_HASH || section.type() == Def::SHT_DYNSYM || section.type() == Def::SHT_SYMTAB);
				const Section & version_section = this->section_by_offset(version);
				assert(version_section.type() == Def::SHT_NULL || version_section.type() == Def::SHT_GNU_VERSYM);
				return SymbolTable(*this, section, version_section);
			}

		return SymbolTable(*this);
	}

 public:
	Array<Dynamic> dynamic;
	SymbolTable symbols;

	ELF_Dyn(uintptr_t start, size_t length) : ELF<C>(start, length), dynamic(find_dynamic()), symbols(find_dynamic_symbol_table())  {

	}

	uint16_t version_index(std::string name) {
		return 0;
	}

	std::string version_name(uint16_t index) {
		return 0;
	}

};
