#pragma once

#include <cassert>
#include <string>
#include <utility>

#include "elf_def/const.hpp"
#include "elf_def/types.hpp"
#include "elf_def/struct.hpp"
#include "elf_def/hash.hpp"

template<ELFCLASS C>
class ELF : public ELF_Def::Structures<C> {
	using Def = typename ELF_Def::Structures<C>;
	using elfptr_t = typename Def::Elf_Addr;

	// Accessor to wrap elements
	template <typename D>
	class Accessor {
	 public:
		const D * _data = nullptr;
	};

	// Array using accessor
	template <typename A>
	class Array {
		using V = decltype(A::_data);

	 public:
		const A accessor;
		V const values;
		const size_t entries;

		Array(uintptr_t ptr, size_t entries, const A & accessor) : accessor(accessor), values(reinterpret_cast<V const>(ptr)), entries(entries) {}

		A operator[](size_t idx) const {
			assert(idx < entries);
			A ret = accessor;
			ret._data = values + idx;
			return ret;
		}

		size_t count() const {
			return entries;
		}

		bool empty() const {
			return entries == 0;
		}

		size_t index(const A & element) const {
			return (reinterpret_cast<size_t>(element._data) - reinterpret_cast<size_t>(values)) / sizeof(*element._data);
		}

		class iterator {
			V p;
			const A & a;

		 public:
			iterator(const V & p, const A & a): p(p), a(a){}

			iterator operator++() {
				++p;
				return *this;
			}
			
			bool operator!=(const iterator & o) const {
				return p != o.p; 
			}

			const A operator*() const {
				A ret = a;
				ret._data = p;
				return ret;
			}
		};

		iterator begin() const {
			return iterator(values, accessor);
		}

		iterator end() const {
			return iterator(values + entries, accessor);
		}
	};

 public:
	const uintptr_t start;
	const size_t length;

	// Header
	struct Header : Def::Ehdr {
		/*! \brief Check if this elf header is valid for ELFO */
		bool valid() const {
			static const char ehdr_ident_magic[4] = { 0x7f, 'E', 'L', 'F' };

			auto & ident = this->e_ident;
			for (int i = 0; i < 4; i++)
				if (ident.ei_magic[i] != ehdr_ident_magic[i])
					return false;
			return ident.ei_class == C
			    && ident.ei_data == host_data()
			    && ident.ei_version == Def::ELFVERSION_CURRENT;
		}

		/*! \brief File class */
		typename Def::ehdr_ident_class ident_class() const {
			return this->e_ident.ei_class;
		}

		/*! \brief Data encoding */
		typename Def::ehdr_ident_data ident_data() const {
			return this->e_ident.ei_data;
		}

		/*! \brief File version */
		typename Def::ehdr_ident_version ident_version() const {
			return this->e_ident.ei_version;
		}

		/*! \brief OS ABI identification */
		typename Def::ehdr_ident_abi ident_abi() const {
			return this->e_ident.ei_abi;
		}

		/*! \brief ABI Version */
		uint8_t ident_abiversion() const {
			return this->e_ident.ei_abiversion;
		}

		/*! \brief Object file type */
		typename Def::ehdr_type type() const {
			return this->e_type;
		}

		/*! \brief Architecture */
		typename Def::ehdr_machine machine() const {
			return this->e_machine;
		}

		/*! \brief Object file version */
		typename Def::ehdr_version version() const {
			return this->e_version;
		}

		/*! \brief Object file version */
		uintptr_t entry() const {
			return this->e_entry;
		}

		/*! \brief Object file version */
		uint32_t flags() const {
			return this->e_flags;
		}

	 private:
		/*! \brief Get host data encoding */
		static ELF_Def::Constants::ehdr_ident_data host_data() {
			static const int tmp = 1;
			return (1 == *(const char*)&tmp) ? ELF_Def::Constants::ELFDATA2LSB : ELF_Def::Constants::ELFDATA2MSB;
		}
	};

	// Segments (Program header table)
	struct Segment : Accessor<typename Def::Phdr> {
		const ELF<C> & elf;

		Segment(const ELF<C> & elf) : elf(elf) {}

		/*! \brief Segment Type */
		typename Def::phdr_type type() const {
			return this->_data->p_type;
		}

		/*! \brief Offset in file */
		uintptr_t offset() const {
			return this->_data->p_offset;
		}

		/*! \brief Pointer to segment data */
		void * data() const {
			return reinterpret_cast<void*>(elf.start + this->_data->p_offset);
		}

		/*! \brief Segment size (in ELF file)*/
		size_t size() const {
			return this->_data->p_filesz;
		}

		/*! \brief Segment virtual address */
		uintptr_t virt_addr() const {
			return this->_data->p_vaddr;
		}

		/*! \brief Segment size in memory */
		size_t virt_size() const {
			return this->_data->p_memsz;
		}

		/*! \brief Segment physical address */
		uintptr_t phys_addr() const {
			return this->_data->p_paddr;
		}

		/*! \brief Should segments virtual memory be readable? */
		bool readable() const {
			return this->_data->p_flags.r == 1;
		}

		/*! \brief Should segments virtual memory be writeable? */
		bool writeable() const {
			return this->_data->p_flags.w == 1;
		}

		/*! \brief Should segments virtual memory be executable? */
		bool executable() const {
			return this->_data->p_flags.x == 1;
		}

		/*! \brief Segments alignment */
		size_t alignment() const {
			return this->_data->p_align;
		}

		/*! \brief Get interpreter path */
		const char * path() const {
			return type() == Def::PT_INTERP ? reinterpret_cast<const char *>(elf.start + this->_data->p_offset) : nullptr;
		}
	};

	struct Section;

	struct Symbol : Accessor<typename Def::Sym> {
		const ELF<C> & elf;
		const uint16_t strtab;

		Symbol(const ELF<C> & elf, uint16_t strtab = 0) : elf(elf), strtab(strtab) {}

		bool valid() const {
			return strtab != 0 && (value() != 0 || size() != 0 || info() != 0 || other() != 0);
		}

		const char * name() const {
			return elf.string(strtab, this->_data->st_name);
		}

		uintptr_t value() const {
			return this->_data->st_value;
		}

		size_t size() const {
			return this->_data->st_size;
		}

		uint16_t section_index() const {
			return this->_data->st_shndx;
		}

		Section section() const {
			return elf.sections[section_index()];
		}

		unsigned char info() const {
			return this->_data->st_info.value;
		}

		typename Def::sym_bind bind() const {
			return reinterpret_cast<typename  Def::sym_bind>(this->_data->st_info.bind);
		}

		typename Def::sym_type type() const {
			return reinterpret_cast<typename  Def::sym_type>(this->_data->st_info.type);
		}

		unsigned char other() const {
			return this->_data->st_other.value;
		}

		typename Def::sym_visibility visibility() const {
			return reinterpret_cast<typename  Def::sym_visibility>(this->_data->st_other.visibility);
		}
	};

	struct SymbolTable {
		const ELF<C> & elf;
		Array<Symbol> symbols;

		SymbolTable(const ELF<C> & elf, const Section & symtab ) : SymbolTable(elf, symtab.get_symbols()) {
			assert(symtab.type() == Def::SHT_SYMTAB || symtab.type() == Def::SHT_DYNSYM);
		}


		inline const char * name(uint32_t idx) {
			return idx == 0 ? nullptr : elf.string(symbols.accessor.strtab, symbols.values[idx].st_name);
		}

		virtual uint32_t index(const char * search_name) {
			for (size_t i = 1; i < symbols.entries; i++)
				if (strcmp(search_name, name(i)) == 0)
					return i;
			return 0;
		}

		inline Symbol operator[](uint32_t idx) const {
			return symbols[idx];
		}

		inline Symbol operator[](const char * search_name) const {
			return operator[](index(search_name));  // 0 == UNDEF
		}

		inline Symbol operator[](std::string search_name) const {
			return operator[](search_name.c_str());
		}

	 protected:
		SymbolTable(const ELF<C> & elf, const Array<Symbol> & symbols) : elf(elf), symbols(symbols) {}

		static inline int strcmp(const char *s1, const char *s2) {
			if (s1 == nullptr || s2 == nullptr)
				return 0;
			while (*s1 == *s2++)
				if (*s1++ == '\0')
					return 0;
			return static_cast<int>(*s1) - static_cast<int>(*(s2-1));
		}
	};

	// https://flapenguin.me/elf-dt-hash
	struct SymbolTable_Hash : public SymbolTable {
		const ELF_Def::Hash_header * header;
		const uint32_t * bucket;
		const uint32_t * chain;

		SymbolTable_Hash(const ELF<C> & elf, const Section & hash)
		  : SymbolTable(elf, elf.sections[hash.link()].get_symbols()), 
			header(reinterpret_cast<ELF_Def::Hash_header*>(hash.data())),
			bucket(reinterpret_cast<const uint32_t *>(header + 1)),
			chain(bucket + header->nbucket) {
			assert(hash.type() == Def::SHT_HASH);
		}

		uint32_t index(const char *search_name) override {
			uint32_t h = hash(search_name);

			for (uint32_t i = bucket[h % (header->nbucket)]; i; i = chain[i])
				if (!SymbolTable::strcmp(search_name, SymbolTable::name(i)))
					return i;

			return 0;
		}

	 private:
		static uint32_t hash(const char *s) {
			uint32_t g, h = 0;
			for (; *s; s++) {
				h = (h << 4) + *s;
				if (g = h & 0xf0000000)
					h ^= g >> 24;
				h &= ~g;
			}
			return h;
		}
	};
	
	// https://blogs.oracle.com/solaris/gnu-hash-elf-sections-v2
	// https://flapenguin.me/elf-dt-gnu-hash
	struct SymbolTable_GnuHash : public SymbolTable {
		const ELF_Def::GnuHash_header * header;
		const elfptr_t * bloom;
		const uint32_t * buckets;
		const uint32_t * chain;

		SymbolTable_GnuHash(const ELF<C> & elf, const Section & gnuhash)
		  : SymbolTable(elf, elf.sections[gnuhash.link()].get_symbols()), 
			header(reinterpret_cast<ELF_Def::GnuHash_header*>(gnuhash.data())),
			bloom(reinterpret_cast<const elfptr_t *>(header + 1 )),
			buckets(reinterpret_cast<const uint32_t *>(bloom + header->bloom_size)),
			chain(buckets + header->nbuckets) {
			assert(gnuhash.type() == Def::SHT_GNU_HASH);
		}

		uint32_t index(const char *search_name) override {
			uint32_t h1 = gnuhash(search_name);

			uint32_t c = sizeof(elfptr_t) * 8;
			elfptr_t n = (h1 / c) % header->bloom_size;
			const elfptr_t one = 1;
			elfptr_t mask = (one << (h1 % c))
			              | (one << ((h1 >> header->bloom_shift) % c));
			if ((bloom[n] & mask) != mask)
				return 0;

			n = buckets[h1 % header->nbuckets];
			if (n == 0)
				return 0;

			const uint32_t * hashval = chain + (n - header->symoffset);

			for (h1 &= ~1; true; n++) {
				uint32_t h2 = *hashval++;
				if ((h1 == (h2 & ~1)) && !SymbolTable::strcmp(search_name, SymbolTable::name(n)) /* TODO: Check Symbol Version */)
					return n;
				if (h2 & 1)
					break;
			}
			return 0;
		}

	 private:
		static uint_fast32_t gnuhash(const char *s) {
			uint_fast32_t h = 5381;
			for (unsigned char c = *s; c != '\0'; c = *++s)
				h = h * 33 + c;
			return h & 0xffffffff;
		}
	};

	struct Relocation : Accessor<typename Def::Rel> {
		const ELF<C> & elf;
		const uint16_t symtab;

		Relocation(const ELF<C> & elf, uint16_t symtab = 0) : elf(elf), symtab(symtab) {}

		bool valid() const {
			return symtab != 0;
		}

		uintptr_t offset() const {
			return this->_data->r_info.value;
		}

		uintptr_t info() const {
			return this->_data->r_info.value;
		}

		std::pair<uint16_t, uint32_t> symbol_index() const {
			return std::pair<uint16_t, uint32_t>(symtab, this->_data->r_info.sym);
		}

		Symbol symbol() const {
			return elf.symbol(symtab, this->_data->r_info.sym);
		}

		// Depending on arch
		template<typename E>
		E type() const {
			return reinterpret_cast<E>(this->_data->r_info.type);
		}
	};

	struct RelocationAddend : Accessor<typename Def::Rela> {
		const ELF<C> & elf;
		const uint16_t symtab;

		RelocationAddend(const ELF<C> & elf, uint16_t symtab = 0) : elf(elf), symtab(symtab) {}

		bool valid() const {
			return symtab != 0;
		}

		uintptr_t offset() const {
			return this->_data->r_info.value;
		}

		uintptr_t info() const {
			return this->_data->r_info.value;
		}

		std::pair<uint16_t, uint32_t> symbol_index() const {
			return std::pair<uint16_t, uint32_t>(symtab, this->_data->r_info.sym);
		}

		Symbol symbol() const {
			return elf.symbol(symtab, this->_data->r_info.sym);
		}

		// Depending on arch
		template<typename E>
		E type() const {
			return reinterpret_cast<E>(this->_data->r_info.type);
		}

		uintptr_t addend() const {
			return this->_data->r_addend;
		}
	};

	struct Dynamic : Accessor<typename Def::Dyn> {
		const ELF<C> & elf;
		const uint16_t strtab;

		Dynamic(const ELF<C> & elf, uint16_t strtab = 0) : elf(elf), strtab(strtab) {}

		bool valid() const {
			return strtab != 0;
		}

		typename Def::dyn_tag tag() const {
			return (typename Def::dyn_tag)(this->_data->d_tag);
		}

		uintptr_t value() const {
			return this->_data->d_un.d_val;
		}

		const char * string() const {
			return elf.string(strtab, this->_data->d_un.d_val);
		}
	};
	
	// Sections (Section header table)
	struct Section : Accessor<typename Def::Shdr> {
		const ELF<C> & elf;

		Section(const ELF<C> & elf) : elf(elf) {}

		typename Def::shdr_type type() const {
			return this->_data->sh_type;
		}

		/*! \brief Is section writable in virtual addr? */
		bool writeable() const {
			return this->_data->sh_flags.write == 1;
		}

		/*! \brief Does section occupt memory during execution? */
		bool allocate() const {
			return this->_data->sh_flags.alloc == 1;
		}

		/*! \brief Is section executable in virtual memory */
		bool executable() const {
			return this->_data->sh_flags.execinstr == 1;
		}

		/*! \brief Might section be merged? */
		bool merge() const {
			return this->_data->sh_flags.merge == 1;
		}

		/*! \brief Contains nul-terminated strings? */
		bool strings() const {
			return this->_data->sh_flags.strings == 1;
		}

		/*! \brief Does \ref info() contain a section header index? */
		bool info_link() const {
			return this->_data->sh_flags.info_link == 1;
		}

		/*! \brief Must the order be preserved after combining? */
		bool link_order() const {
			return this->_data->sh_flags.link_order == 1;
		}

		/*! \brief Is non-standard OS specific handling required? */
		bool os_nonconforming() const {
			return this->_data->sh_flags.os_nonconforming == 1;
		}

		/*! \brief Is the section member of a group? */
		bool group() const {
			return this->_data->sh_flags.group == 1;
		}

		/*! \brief Does the section hold thread-local data? */
		bool tls() const {
			return this->_data->sh_flags.tls == 1;
		}

		/*! \brief Does the section contain compressed data? */
		bool compressed() const {
			return this->_data->sh_flags.compressed == 1;
		}

		/*! \brief Section virtual addr at execution */
		uintptr_t address() const {
			return this->_data->sh_addr;
		}

		/*! \brief Section file offset */
		uintptr_t offset() const {
			return this->_data->sh_offset;
		}

		/*! \brief Pointer to dection contents */
		void * data() const {
			return reinterpret_cast<void*>(elf.start + this->_data->sh_offset);
		}

		/*! \brief Section size in bytes */
		size_t size() const {
			return this->_data->sh_size;
		}

		/*! \brief Entry size if section holds table */
		size_t entry_size() const {
			return this->_data->sh_entsize;
		}

		/*! \brief Section alignment */
		size_t alignment() const {
			return this->_data->sh_addralign;
		}

		/*! \brief Link to another section */
		uint32_t link() const {
			return this->_data->sh_link;
		}

		/*! \brief Additional section information */
		uint32_t info() const {
			return this->_data->sh_info;
		}

		/*! \brief Section name */
		const char * name() const {
			return elf.string(elf.header.e_shstrndx, this->_data->sh_name);
		}

		template<typename T>
		Array<T> get() const {
			assert(entry_size() == sizeof(*T::_data));
			return Array<T>(elf.start + offset(), size() / entry_size(), T(elf, link()));
		}

		Array<Symbol> get_symbols() const {
			assert(type() == Def::SHT_SYMTAB || type() == Def::SHT_DYNSYM);
			return get<Symbol>();
		}

		SymbolTable get_symbol_table() const {
			return SymbolTable(elf, *this);
		}

		Array<Dynamic> get_dynamic() const {
			assert(type() == Def::SHT_DYNAMIC);
			// Calculate length
			uintptr_t ptr = elf.start + offset();
			typename Def::Dyn * dyn = reinterpret_cast<typename Def::Dyn *>(ptr);
			assert(entry_size() == sizeof(*dyn));
			size_t entries = 0;
			for (; entries < (size() / entry_size()) - 1 && dyn[entries].d_tag != Def::DT_NULL; entries++) {}
			return Array<Dynamic>(ptr, entries + 1, Dynamic(elf, link()));
		}

	};


	// Header
	const Header &header;

	Array<Segment> segments;

	Array<Section> sections;


	ELF(uintptr_t start, size_t length) : start(start), length(length), 
	    header(*reinterpret_cast<Header*>(start)), 
	    segments(start + header.e_phoff, header.e_phnum, Segment(*this)),
	    sections(start + header.e_shoff, header.e_shnum, Section(*this)) {
		assert(sizeof(Header) == header.e_ehsize);
		assert(sizeof(typename Def::Phdr) == header.e_phentsize);
		assert(sizeof(typename Def::Shdr) == header.e_shentsize);
	}


	template<typename T>
	Array<T> get(uintptr_t offset, size_t size, const T & t) const {
		return Array<T>(offset, size, t);
	}

	template<typename T>
	Array<T> get() const {
		return Array<T>(0, 0, T(*this));
	}

	Section section_by_offset(uintptr_t offset) const {
		for (auto &s : sections)
			if (s.offset() == offset)
				return s;
		return sections[0];
	}

	Symbol symbol(const Section & section, uint32_t index) const {
		return sections.get_symbols()[index];
	}

	Symbol symbol(uint16_t section_index, uint32_t index) const {
		return symbol(sections[section_index], index);
	}

	const char * string(const Section & section, uint32_t offset) const {
		assert(section.type() == Def::SHT_STRTAB);
		return reinterpret_cast<const char *>(start + section.offset() + offset);
	}

	const char * string(uint16_t section_index, uint32_t offset) const {
		return string(sections[section_index], offset);
	}

	/*! \brief Find dynamic */
	Array<Dynamic> dynamic() const {
		for (auto & section : sections)
			if (section.type() == Def::SHT_DYNAMIC)
				return section.get_dynamic();
		return get<Dynamic>();
	}

	/*! \brief Find dyanmic symbol table
	 * Use hash if possible
	 * return allocated object (delete it after use) or nullptr
	 */
	SymbolTable * dynamic_symbol_table() const {
		uintptr_t gnuhash_offset = 0;
		uintptr_t hash_offset = 0;
		uintptr_t sym_offset = 0;

		for (auto &dyn: dynamic()) {
			switch(dyn.tag()) {
				case Def::DT_GNU_HASH:
					gnuhash_offset = dyn.value();
					break;

				case Def::DT_HASH:
					hash_offset = dyn.value();
					break;

				case Def::DT_SYMTAB:
					sym_offset = dyn.value();
					break;
			}
		}

		if (gnuhash_offset != 0)
			return new SymbolTable_GnuHash(*this, section_by_offset(gnuhash_offset));

		if (hash_offset != 0)
			return new SymbolTable_Hash(*this, section_by_offset(hash_offset));

		if (sym_offset != 0)
			return new SymbolTable(*this, section_by_offset(sym_offset));

		return nullptr;
	}
};

using ELF32 = ELF<ELFCLASS::ELFCLASS32>;
using ELF64 = ELF<ELFCLASS::ELFCLASS64>;


