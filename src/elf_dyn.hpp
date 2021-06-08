#pragma once

#ifdef USE_DLH
#include <dlh/string.hpp>
#else
#include <cstring>
#endif

#include "elf.hpp"

template<ELFCLASS C>
class ELF_Dyn : public ELF<C> {
 public:
	template<typename T>
	using Array = typename ELF<C>::template Array<T>;
	template<typename T>
	using List = typename ELF<C>::template List<T>;
	using Section = typename ELF<C>::Section;
	using Dynamic = typename ELF<C>::Dynamic;
	using Relocation = typename ELF<C>::Relocation;
	using SymbolTable = typename ELF<C>::SymbolTable;
	using VersionNeeded = typename ELF<C>::VersionNeeded;
	using VersionDefinition = typename ELF<C>::VersionDefinition;

 private:
	using Def = typename ELF_Def::Structures<C>;
	using elfptr_t = typename Def::Elf_Addr;


	/*! \brief Find dynamic section */
	constexpr auto find_dynamic() const {
		for (auto & section : this->sections)
			if (section.type() == Def::SHT_DYNAMIC)
				return section.get_dynamic();
		return this->template get<Dynamic>();
	}

	/*! \brief Get a section referenced in the dynamic section
	 * \param tag section tag
	 * \return first section with tag or NULL section
	 */
	Section get_dynamic_section(typename Def::dyn_tag tag) const {
		// We don't care about O(n) since n is quite small.
		for (auto &dyn: dynamic)
			if (dyn.tag() == tag)
				return this->section_by_offset(dyn.value());
		return this->sections[0];
	}

	/*! \brief Find dyanmic symbol table
	 * Use hash if possible
	 * \return Symbol Table
	 */
	constexpr auto find_dynamic_symbol_table() const {
		typename Def::dyn_tag search_tags[] = { Def::DT_GNU_HASH, Def::DT_HASH, Def::DT_SYMTAB };

		for (auto & search_tag : search_tags) {
			const Section section = get_dynamic_section(search_tag);
			if (section.type() != Def::SHT_NULL) {
				assert(section.type() == Def::SHT_GNU_HASH || section.type() == Def::SHT_HASH || section.type() == Def::SHT_DYNSYM || section.type() == Def::SHT_SYMTAB);
				const Section version_section = get_dynamic_section(Def::DT_VERSYM);
				assert(version_section.type() == Def::SHT_NULL || version_section.type() == Def::SHT_GNU_VERSYM);
				return SymbolTable(*this, section, version_section);
			}
		}
		return SymbolTable(*this);
	}

	/*! \brief Find relocation table
	 * \return Relocation Table
	 */
	Array<Relocation> find_dynamic_relocations() const {
		uintptr_t jmprel = 0;  // Address of PLT relocation table
		uintptr_t pltrel = Elf::DT_NULL;  // Type of PLT relocation table (REL or RELA)
		size_t pltrelsz = 0;   // Size of PLT relocation table (and, hence, GOT)

		for (auto &dyn: dynamic) {
			switch (dyn.tag()) {
				case Elf::DT_JMPREL:
					jmprel = dyn.value();
					break;
				case Elf::DT_PLTREL:
					pltrel = dyn.value();
					break;
				case Elf::DT_PLTRELSZ:
					pltrelsz = dyn.value();
					break;
				default:
					continue;
			}
		}

		auto section = this->section_by_offset(jmprel);
		assert((jmprel == 0 && pltrel == Elf::DT_NULL) || section.size() == pltrelsz);
		return section.get_relocations();
	}

 public:
	Array<Dynamic> dynamic;
	SymbolTable symbols;
	Array<Relocation> relocations;
	List<VersionNeeded> version_needed;
	List<VersionDefinition> version_definition;

	ELF_Dyn(uintptr_t start)
	  : ELF<C>(start),
	    dynamic(find_dynamic()),
	    symbols(find_dynamic_symbol_table()),
	    relocations(find_dynamic_relocations()),
	    version_needed(get_dynamic_section(Def::DT_VERNEED).template get_list<VersionNeeded>(true)),
	    version_definition(get_dynamic_section(Def::DT_VERDEF).template get_list<VersionDefinition>(true)) {
	}

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
