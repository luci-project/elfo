#pragma once

#ifdef USE_DLH
#include <dlh/assert.hpp>
#include <dlh/string.hpp>
#else
#include <cassert>
#include <cstring>
#endif

#include "elf_def/const.hpp"
#include "elf_def/ident.hpp"
#include "elf_def/hash.hpp"
#include "elf_def/struct.hpp"
#include "elf_def/types.hpp"


/*! \brief Parser for data in the Executable and Linking Format
 * \tparam C 32- or 64-bit elf class
 */
template<ELFCLASS C>
class ELF : public ELF_Def::Structures<C> {
	using Def = typename ELF_Def::Structures<C>;
	using elfptr_t = typename Def::Elf_Addr;

	/*! \brief Start address of ELF in memory */
	inline uintptr_t start() const {
		return reinterpret_cast<uintptr_t>(&header);
	}

	/*! \brief Accessor to wrap elements of a given data type
	 * \tparam DT data type of element
	 */
	template <typename DT>
	struct Accessor {
		/*! \brief Parent object */
		const ELF<C> & _elf;

		/*! \brief Pointer to payload */
		const DT * _data;

		/*! \brief Constructor */
		explicit Accessor(const ELF<C> & elf, const DT * data = nullptr)
		  : _elf{elf}, _data{data} {}

		/*! \brief Elf object */
		const ELF<C> & elf() const {
			return _elf;
		}

		/*! \brief Retrieve payload */
		const DT * ptr() const {
			return _data;
		}

		/*! \brief Get current element */
		const DT data() const {
			return *_data;
		}

		/*! \brief Get current element */
		const DT operator*() const {
			return *_data;
		}

		/*! \brief Absolute memory address of current element */
		uintptr_t address() const {
			return reinterpret_cast<uintptr_t>(_data);
		}

		/*! \brief Is this the identical payload? */
		bool operator==(const Accessor<DT> & o) const {
			return _data == o._data;
		}

		/*! \brief Is this a different payload? */
		bool operator!=(const Accessor<DT> & o) const {
			return _data != o._data;
		}

		/*! \brief Pointer to next element */
		const DT * next(size_t i = 1) const {
			return _data + i;
		}

		/*! \brief Size (in bytes) of an entry */
		size_t element_size() const {
			return sizeof(DT);
		}
	};

	/*! \brief Iterator */
	template <typename A>
	class Iterator {
		/*! \brief Accessor Template for elements */
		A accessor;

	 public:
		/*! \brief Iterator constructor
		 * \param p pointer to current element
		 * \param a accessor template
		 */
		Iterator(const A & accessor)
		  : accessor{accessor} {}

		/*! \brief Next element
		 * \note Element must provide a `next` method!
		 */
		Iterator operator++() {
			assert(accessor._data != accessor.next());
			accessor._data = accessor.next();
			return *this;
		}

		/*! \brief Compare current iterator element */
		bool operator==(const Iterator & other) const {
			return accessor._data == other.accessor._data;
		}

		/*! \brief Compare current iterator element */
		bool operator!=(const Iterator & other) const {
			return accessor._data != other.accessor._data;
		}

		/*! \brief Get current element */
		const A operator*() const {
			return accessor;
		}
	};


	template <typename A>
	class Accessors {
	 protected:
		/*! \brief Data type of element */
		using V = decltype(A::_data);

		/*! \brief Accessor template with pointer to first element */
		const A _accessor;

		/*! \brief Pointer to last element */
		const V _end;

		/*! \brief Create new accessor pointing to specific element */
		static A _accessor_value(const A & accessor, V ptr) {
			A a = accessor;
			a._data = ptr;
			return a;
		}

		/*! \brief Create new accessor pointing to specific element */
		static A _accessor_value(const A & accessor, void * ptr) {
			return _accessor_value(accessor, reinterpret_cast<V>(ptr));
		}

		/*! \brief Constructor (using end pointer) */
		Accessors(const A & accessor, V end)
		  : _accessor{accessor}, _end{end} {}

		/*! \brief Constructor (using size) */
		Accessors(const A & accessor, size_t entries)
		  : _accessor{accessor}, _end{_accessor.next(entries)} {}

	 public:
		/*! \brief Get accessor template
		 * \return reference to current accessor
		 */
		const A & accessor() const {
			return _accessor;
		}

		/*! \brief Get address of first element
		 * \return Memory address of first element
		 */
		uintptr_t address() const {
			return reinterpret_cast<uintptr_t>(_accessor._data);
		}

		/*! \brief Get Iterator for first element */
		Iterator<A> begin() const {
			return { _accessor };
		}

		/*! \brief Get Iterator identicating end of list */
		Iterator<A> end() const {
			return { _accessor_value(_accessor, _end) };
		}
	};

 public:
	/*! \brief Array-like access to data with fixed element size using accessor
	 * \tparam A classd
	 */
	template <typename A>
	class Array : public Accessors<A> {
		/*! \brief Data type of element */
		using V = decltype(A::_data);

	 public:
		/*! \brief Construct new Array access
		 * \param accessor \ref Accessor template
		 * \param ptr Pointer to first element
		 * \param entries number of elements
		 */
		Array(const A & accessor, void * ptr, size_t entries)
		  : Accessors<A>{this->_accessor_value(accessor, ptr), entries} {
			assert(this->_accessor.element_size() > 0);
		}

		/*! \brief Array-like access
		 * \param idx index
		 * \return Accessor for element
		 */
		A operator[](size_t idx) const {
			assert(this->_accessor.next(idx) < this->_end);
			return this->_accessor_value(this->_accessor, this->_accessor.next(idx));
		}

		/*! \brief Number of elements in array
		 */
		size_t count() const {
			return (reinterpret_cast<size_t>(this->_end) - reinterpret_cast<size_t>(this->_accessor._data)) / this->_accessor.element_size();
		}

		/*! \brief Are there any elements in the array?
		 * \return `false` if there is at least one element
		 */
		bool empty() const {
			return this->_accessor._data == this->_end;
		}

		/*! \brief Get index of element
		 * \param element Element in array to get index of
		 * \note you have to ensure that this element exists in the array
		 * \return index
		 */
		size_t index(const A & element) const {
			return (reinterpret_cast<size_t>(element._data) - reinterpret_cast<size_t>(this->_accessor._data)) / this->_accessor.element_size();
		}
	};

	/*! \brief Sequential access (single linked-list style) to data with variable element size using accessor
	 * \tparam A class
	 */
	template <typename A>
	class List : public Accessors<A> {
		using V = typename Accessors<A>::V;
	 public:
		/*! \brief Construct new List access
		 * \param accessor \ref Accessor template
		 * \param begin Pointer to first element
		 * \param end Indicator for end of list
		 */
		List(const A & accessor, void * begin, void * end)
		  : Accessors<A>{this->_accessor_value(accessor, begin), reinterpret_cast<V>(end)} {}

		/*! \brief Array-like access
		 * \note O(n) complexity!
		 * \param idx index
		 * \return Accessor for element
		 */
		A operator[](size_t idx) const {
			for (const auto & entry : *this)
				if (idx-- == 0)
					return entry;
			assert(false);
			return this->_accessor;
		}

		/*! \brief Number of elements in array
		 * \note O(n) complexity!
		 */
		size_t count() const {
			size_t entries = 0;
			for (const auto & entry : *this)
				entries++;
			return entries;
		}

		/*! \brief Are there any elements in the array?
		 * \return `false` if there is at least one element
		 */
		bool empty() const {
			return this->begin() == this->end();
		}
	};


	/*! \brief ELF Header */
	struct Header : Def::Ehdr {
		/*! \brief Check if this elf identification header is valid for ELFO */
		bool valid() const {
			return this->identification.valid()
			    && this->identification.elfclass() == C
			    && this->identification.data_supported();
		}

		/*! \brief File class */
		typename Def::ident_class ident_class() const {
			return this->identification.elfclass();
		}

		/*! \brief Data encoding */
		typename Def::ident_data ident_data() const {
			return this->identification.data();
		}

		/*! \brief File version */
		typename Def::ident_version ident_version() const {
			return this->identification.version();
		}

		/*! \brief OS ABI identification */
		typename Def::ident_abi ident_abi() const {
			return this->identification.abi();
		}

		/*! \brief ABI Version */
		unsigned ident_abiversion() const {
			return this->identification.abiversion();
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

		/*! \brief Object entry point */
		uintptr_t entry() const {
			return this->e_entry;
		}

		/*! \brief Object flags  */
		uint32_t flags() const {
			return this->e_flags;
		}
	};

	/*! \brief Forward declaration for Dynamic section */
	struct Dynamic;
	struct DynamicTable;

	// Segments (Program header table)
	struct Segment : Accessor<typename Def::Phdr> {
		/*! \brief Constructor for new Segment entry */
		explicit Segment(const ELF<C> & elf)
		  : Accessor<typename Def::Phdr>{elf} {}

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
			return this->_elf.data(this->_data->p_offset);
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
			return type() == Def::PT_INTERP ? reinterpret_cast<const char *>(this->_elf.data(this->_data->p_offset)) : nullptr;
		}

		/*! \brief Get contents of dynamic secion
		 * \param mapped the elf file is already mapped to the segments virt_mem
		 */
		Array<Dynamic> get_dynamic(bool mapped = false) const {
			assert(type() == Def::PT_DYNAMIC);
			size_t entries = 0;
			uintptr_t strtaboff = 0;
			bool absolute_address = false;
			void * dyn = load_dynamic(mapped, strtaboff, entries, absolute_address);
			return { Dynamic{this->_elf, strtaboff}, dyn, entries };
		}

		/*! \brief Get contents of dynamic secion
		 * \param mapped the elf file is already mapped to the segments virt_mem
		 */
		DynamicTable get_dynamic_table(bool mapped = false) const {
			assert(type() == Def::PT_DYNAMIC);
			size_t entries = 0;
			uintptr_t strtaboff = 0;
			bool absolute_address = false;
			void * dyn = load_dynamic(mapped, strtaboff, entries, absolute_address);
			return DynamicTable{this->_elf, dyn, entries, strtaboff, !mapped, absolute_address};
		}

	 private:
		void * load_dynamic(bool mapped, uintptr_t & strtaboff, size_t & entries, bool & absolute_address) const {
			// Static, non relocatable binaries use absolute addressing
			absolute_address = this->_elf.header.type() == Def::ET_EXEC;

			// If mapped use virtual address, otherwise offset
			typename Def::Dyn * dyn;
			if (mapped)
				dyn = reinterpret_cast<typename Def::Dyn *>((absolute_address ? 0 : this->_elf.start()) + virt_addr());
			else
				dyn = reinterpret_cast<typename Def::Dyn *>(this->_elf.data(offset()));

			size_t limit = (size() / sizeof(*dyn)) - 1;
			entries = 0;
			for (; entries < limit && dyn[entries].d_tag != Def::DT_NULL; entries++)
				if (dyn[entries].d_tag == Def::DT_STRTAB)
					strtaboff = dyn[entries].d_un.d_val;

			if (!mapped)
				strtaboff = DynamicTable::translate(this->_elf, strtaboff);
			else if (absolute_address)
				strtaboff -= this->_elf.start();

			entries++;

			return dyn;
		}
	};

	/*! \brief Forward declaration for Section */
	struct Section;

	/*! \brief Symbol */
	struct Symbol : Accessor<typename Def::Sym> {
		/*! \brief Offset of string table */
		const uintptr_t strtaboff;

		/*! \brief Construct symbol
		 * \param elf ELF object to which this symbol belongs to
		 * \param strtab String table index associated with the symbol table for this symbol
		 * \param ptr Pointer to the memory containting the current symbol
		 */
		Symbol(const ELF<C> & elf, uint16_t strtab, void * ptr = nullptr)
		  : Symbol{elf, elf.sections[strtab], ptr} {}

		/*! \brief Construct symbol
		 * \param elf ELF object to which this symbol belongs to
		 * \param strtab String table index associated with the symbol table for this symbol
		 * \param ptr Pointer to the memory containting the current symbol
		 */
		Symbol(const ELF<C> & elf, const Section & strtab, void * ptr = nullptr)
		  : Symbol{elf, strtab.offset(), ptr} {
			assert(strtab.type() == Def::SHT_STRTAB);
		}

		/*! \brief Construct symbol
		 * \param elf ELF object to which this symbol belongs to
		 * \param strtaboff Offset to string table associated with the symbol table for this symbol
		 * \param ptr Pointer to the memory containting the current symbol
		 */
		explicit Symbol(const ELF<C> & elf, uintptr_t strtaboff = 0, void * ptr = nullptr)
		  : Accessor<typename Def::Sym>{elf, reinterpret_cast<typename Def::Sym *>(ptr)}, strtaboff(strtaboff) {}

		/*! \brief Is the symbol valid? */
		bool valid() const {
			return strtaboff != 0 && (value() != 0 || size() != 0 || info() != 0 || other() != 0);
		}

		/*! \brief Symbol name */
		const char * name() const {
			assert(strtaboff != 0);
			return reinterpret_cast<const char *>(this->_elf.start() + strtaboff + this->_data->st_name);
		}

		/*! \brief Symbol value */
		uintptr_t value() const {
			return this->_data->st_value;
		}

		/*! \brief Size of Symbol */
		size_t size() const {
			return this->_data->st_size;
		}

		/*! \brief Index of the section containing the symbol  */
		uint16_t section_index() const {
			return this->_data->st_shndx;
		}

		/*! \brief Section containing the symbol  */
		Section section() const {
			return this->_elf.sections[section_index()];
		}

		/*! \brief Symbol info (including type and binding) */
		unsigned char info() const {
			return this->_data->st_info.value;
		}

		/*! \brief Symbol binding */
		typename Def::sym_bind bind() const {
			return reinterpret_cast<typename  Def::sym_bind>(this->_data->st_info.bind);
		}

		/*! \brief Symbol type */
		typename Def::sym_type type() const {
			return reinterpret_cast<typename  Def::sym_type>(this->_data->st_info.type);
		}

		/*! \brief Additional Symbol info (including visibility) */
		unsigned char other() const {
			return this->_data->st_other.value;
		}

		/*! \brief Symbol visibility */
		typename Def::sym_visibility visibility() const {
			return reinterpret_cast<typename  Def::sym_visibility>(this->_data->st_other.visibility);
		}
	};

	/*! \brief Symbol Table
	 * Use hash if possible
	 */
	struct SymbolTable : public Array<Symbol> {
		const typename Def::shdr_type section_type;
		const void * header;
		const uint16_t * const versions;
		using Array<Symbol>::operator[];
		using Array<Symbol>::index;

		/*! \brief Symbol table
		 * similar to symbols array, but offering a symbol lookup
		 * (which can be quite fast when using hash)
		 */
		SymbolTable(const ELF<C> & elf, const Section & section)
		  : SymbolTable{elf, section, elf.sections[0]} {}

		/*! \brief Symbol table with version information
		 */
		SymbolTable(const ELF<C> & elf, const Section & section, const Section & version_section)
		  : SymbolTable{elf, (section.type() == Def::SHT_GNU_HASH) || (section.type() == Def::SHT_HASH), section, version_section} {}

		/*! \brief Raw Symbol table constructor
		 * \param elf This elf file
		 * \param section_type Hash, Gnu Hash, Symbol or Dynamic Symbol Table?
		 * \param header Pointer to hash header if applicable
		 * \param symtab Pointer to symbol table
		 * \param symtabentries Number of entries in symbol table
		 * \param versions Pointer versions array (for symbol table) if available (otherwise: `nullptr`)
		 * \param strtaboff Offset in elf file to string table
		 */
		SymbolTable(const ELF<C> & elf, const typename Def::shdr_type section_type, const void * header, void * symtab, size_t symtabentries, const uint16_t * versions, uintptr_t strtaboff)
		  : Array<Symbol>{ Symbol{ elf, strtaboff}, symtab, symtabentries}, section_type{section_type}, header{header}, versions{versions} {}

		/*! \brief Empty (non-existing) symbol table
		 */
		explicit SymbolTable(const ELF<C> & elf)
		  : Array<Symbol>{Symbol{elf}, 0, 0}, section_type{Def::SHT_NULL}, header{nullptr}, versions{nullptr} {}

		/*! \brief Elf object */
		const ELF<C> & elf() const {
			return this->_accessor._elf;
		}

		/*! \brief Get symbol name by index */
		inline const char * name(uint32_t idx) const {
			return idx == Def::STN_UNDEF ? nullptr : elf().string(this->_accessor.strtaboff, this->_accessor._data[idx].st_name);
		}

		/*! \brief Get symbol version index by symbol index
		 * \param idx symbol index
		 * \return Symbol version or VER_NDX_GLOBAL if none
		 */
		inline uint16_t version(uint32_t idx) const{
			return versions == nullptr ? Def::VER_NDX_GLOBAL : (versions[idx] & 0x7fff);
		}

		/*! \brief Get the high order bit of the symbol version index by symbol index
		 * \param idx symbol index
		 * \return true if high order bit is set
		 */
		inline bool ignored(uint32_t idx) const{
			return versions == nullptr ? false : ((versions[idx] & 0x8000) != 0);
		}

		/*! \brief Find symbol
		 * use hash if available
		 * \note Undefined symbols are usually excluded from hash hence they might not be found using this method!
		 * \param search_name symbol name to search
		 * \param required_version required version or VER_NDX_GLOBAL if none
		 * \return index of object or STN_UNDEF
		 */
		size_t index(const char * search_name, uint16_t required_version = Def::VER_NDX_GLOBAL) const {
			if (required_version != Def::VER_NDX_GLOBAL && versions == nullptr)
				required_version = Def::VER_NDX_GLOBAL;
			switch (section_type) {
				case Def::SHT_HASH:
					return index_by_hash(search_name, ELF_Def::hash(search_name), required_version);
				case Def::SHT_GNU_HASH:
					return index_by_gnuhash(search_name, ELF_Def::gnuhash(search_name), required_version);
				case Def::SHT_DYNSYM:
				case Def::SHT_SYMTAB:
					return index_by_strcmp(search_name, required_version);
				default:
					return Def::STN_UNDEF;
			}
		}

		/*! \brief Find symbol using calculated hash values
		 * \note Undefined symbols are usually excluded from hash hence they might not be found using this method!
		 * \param search_name symbol name to search
		 * \param hash_value elf hash value of symbol_name
		 * \param gnu_hash_value gnu hash value
		 * \param required_version required version or VER_NDX_GLOBAL if none
		 * \return index of object or STN_UNDEF
		 */
		size_t index(const char * search_name, uint32_t hash_value, uint32_t gnu_hash_value, uint16_t required_version = Def::VER_NDX_GLOBAL) const {
			if (required_version != Def::VER_NDX_GLOBAL && versions == nullptr)
				required_version = Def::VER_NDX_GLOBAL;
			switch (section_type) {
				case Def::SHT_HASH:
					return index_by_hash(search_name, hash_value, required_version);
				case Def::SHT_GNU_HASH:
					return index_by_gnuhash(search_name, gnu_hash_value, required_version);
				case Def::SHT_DYNSYM:
				case Def::SHT_SYMTAB:
					return index_by_strcmp(search_name, required_version);
				default:
					return Def::STN_UNDEF;
			}
		}

		/*! \brief Access symbol by char* index
		 * \param search_name symbol name to search
		 * \return Symbol
		 */
		inline Symbol operator[](const char * search_name) const {
			return operator[](index(search_name));  // 0 == UNDEF
		}

	 private:
		/*! \brief Helper constructor */
		SymbolTable(const ELF<C> & elf, bool use_hash, const Section & section, const Section & version_section)
		  : SymbolTable{elf, section.type(), use_hash ? section.data() : nullptr, use_hash ? elf.sections[section.link()] : section, version_section} {}

		/*! \brief Helper constructor */
		SymbolTable(const ELF<C> & elf, const typename Def::shdr_type section_type, void * header, const Section & symbol_section, const Section & version_section)
		  : SymbolTable{elf, section_type, header, elf.data(symbol_section.offset()), symbol_section.entries(), version_section.type() == Def::SHT_GNU_VERSYM ? version_section.get_versions() : nullptr, elf.sections[symbol_section.link()].offset()} {
			assert(section_type == Def::SHT_GNU_HASH || section_type == Def::SHT_HASH || section_type == Def::SHT_DYNSYM || section_type == Def::SHT_SYMTAB);
			assert(section_type == Def::SHT_DYNSYM || section_type == Def::SHT_SYMTAB || header != nullptr);
		}

		/*! \brief Find symbol index using ELF Hash
		 * \see https://flapenguin.me/elf-dt-hash
		 * \param search_name symbol name to search
		 * \param hash_value elf hash value of symbol_name
		 * \return index of object or STN_UNDEF
		 */
		uint32_t index_by_hash(const char *search_name, uint32_t hash_value, uint16_t required_version) const {
			const ELF_Def::Hash_header * header = reinterpret_cast<const ELF_Def::Hash_header*>(this->header);
			const uint32_t * bucket = reinterpret_cast<const uint32_t *>(header + 1);
			const uint32_t * chain = bucket + header->nbucket;

			for (uint32_t i = bucket[hash_value % (header->nbucket)]; i; i = chain[i])
				if (!::strcmp(search_name, SymbolTable::name(i)) && check_version(i, required_version))
					return i;

			return Def::STN_UNDEF;
		}

		/*! \brief Find symbol index using ELF Hash
		 * \see https://blogs.oracle.com/solaris/gnu-hash-elf-sections-v2
		 * \see https://flapenguin.me/elf-dt-gnu-hash
		 * \param search_name symbol name to search
		 * \param hash_value gnu hash value
		 * \return index of object or STN_UNDEF
		 */
		uint32_t index_by_gnuhash(const char *search_name, uint32_t hash_value, uint16_t required_version) const {
			const ELF_Def::GnuHash_header * header = reinterpret_cast<const ELF_Def::GnuHash_header*>(this->header);
			const elfptr_t * bloom = reinterpret_cast<const elfptr_t *>(header + 1);
			const uint32_t * buckets = reinterpret_cast<const uint32_t *>(bloom + header->bloom_size);
			const uint32_t * chain = buckets + header->nbuckets;

			const uint32_t c = sizeof(elfptr_t) * 8;
			const elfptr_t one = 1;
			const elfptr_t mask = (one << (hash_value % c))
			                    | (one << ((hash_value >> header->bloom_shift) % c));

			elfptr_t n = (hash_value / c) % header->bloom_size;
			if ((bloom[n] & mask) != mask)
				return Def::STN_UNDEF;

			n = buckets[hash_value % header->nbuckets];
			if (n == 0)
				return Def::STN_UNDEF;

			const uint32_t * hashval = chain + (n - header->symoffset);

			for (hash_value &= ~1; true; n++) {
				uint32_t h2 = *hashval++;
				if ((hash_value == (h2 & ~1)) && !::strcmp(search_name, SymbolTable::name(n)) && check_version(n, required_version))
					return n;
				if (h2 & 1)
					break;
			}
			return Def::STN_UNDEF;
		}

		/*! \brief Find symbol index using string comparison
		 * \param search_name symbol name to search
		 * \return index of object or STN_UNDEF
		 */
		uint32_t index_by_strcmp(const char *search_name, uint16_t required_version) const {
			const auto entries = this->count();
			for (size_t i = 1; i < entries; i++)
				if (!::strcmp(search_name, name(i)) && check_version(i, required_version))
					return i;
			return Def::STN_UNDEF;
		}

		/*! \brief Helper to compare version
		 */
		inline bool check_version(size_t idx, uint16_t required_version) const {
			return required_version == Def::VER_NDX_GLOBAL
			    || versions == nullptr
			    || required_version == (versions[idx] & 0x7fff);
		}
	};

	/*! \brief Relocation entry without addend */
	struct RelocationWithoutAddend : Accessor<typename Def::Rel> {
		/*! \brief Corresponding symbol table offset */
		const uintptr_t symtaboff;
		/*! \brief String table offset (for symbol table) */
		const uintptr_t strtaboff;

		/*! \brief Construct relocation entry (without addend)
		 * \param elf ELF object to which this relocation belongs to
		 * \param symtab Symbol table index for this relocation
		 * \param ptr Pointer to the memory containting the current relocation
		 */
		RelocationWithoutAddend(const ELF<C> & elf, uint16_t symtab, void * ptr = nullptr)
		  : RelocationWithoutAddend{elf, elf.sections[symtab], ptr} {}

		/*! \brief Construct relocation entry (without addend)
		 * \param elf ELF object to which this relocation belongs to
		 * \param symtab Symbol table section for this relocation
		 * \param ptr Pointer to the memory containting the current relocation
		 */
		RelocationWithoutAddend(const ELF<C> & elf, const Section & symtab, void * ptr = nullptr)
		  : RelocationWithoutAddend{elf, symtab.offset(), elf.sections[symtab.link()].offset(), ptr} {
			assert(symtab.type() == Def::SHT_SYMTAB || symtab.type() == Def::SHT_DYNSYM);
			assert(elf.sections[symtab.link()].type() == Def::SHT_STRTAB);
		}

		/*! \brief Construct relocation entry (without addend)
		 * \param elf ELF object to which this relocation belongs to
		 * \param symtaboff Offset to the symbol table for this relocation
		 * \param strtaboff Offset to the string table (required by the symbol table)
		 * \param ptr Pointer to the memory containting the current relocation
		 */
		explicit RelocationWithoutAddend(const ELF<C> & elf, uintptr_t symtaboff = 0, uintptr_t strtaboff = 0, void * ptr = nullptr)
		  : Accessor<typename Def::Rel>{elf, reinterpret_cast<typename Def::Rel *>(ptr)}, symtaboff{symtaboff}, strtaboff{strtaboff} {}

		/*! \brief Valid relocation */
		bool valid() const {
			return symtaboff != 0 && strtaboff != 0;
		}

		/*! \brief Address */
		uintptr_t offset() const {
			return this->_data->r_offset;
		}

		/*! \brief Relocation type and symbol index */
		uintptr_t info() const {
			return this->_data->r_info.value;
		}

		/*! \brief Target symbol */
		Symbol symbol() const {
			return Symbol{this->_elf, strtaboff, this->_elf.data(symtaboff + this->_data->r_info.sym * sizeof(typename Def::Sym))};
		}

		/*! \brief Index of target symbol in corresponding symbol table */
		uint32_t symbol_index() const {
			return this->_data->r_info.sym;
		}

		/*! \brief Relocation type (depends on architecture) */
		uint32_t type() const {
			return static_cast<uint32_t>(this->_data->r_info.type);
		}

		/*! \brief Addend (which is always zero for this relocation type) */
		intptr_t addend() const {
			return 0;
		}
	};

	/*! \brief Relocation entry with addend */
	struct RelocationWithAddend : Accessor<typename Def::Rela> {
		/*! \brief Corresponding symbol table offset */
		const uintptr_t symtaboff;
		/*! \brief String table offset (for symbol table) */
		const uintptr_t strtaboff;

		/*! \brief Construct relocation entry (with addend)
		 * \param elf ELF object to which this relocation belongs to
		 * \param symtab Symbol table index for this relocation
		 * \param ptr Pointer to the memory containting the current relocation
		 */
		RelocationWithAddend(const ELF<C> & elf, uint16_t symtab, void * ptr = nullptr)
		  : RelocationWithAddend{elf, elf.sections[symtab], ptr} {}

		/*! \brief Construct relocation entry (with addend)
		 * \param elf ELF object to which this relocation belongs to
		 * \param symtab Symbol table section for this relocation
		 * \param ptr Pointer to the memory containting the current relocation
		 */
		RelocationWithAddend(const ELF<C> & elf, const Section & symtab, void * ptr = nullptr)
		  : RelocationWithAddend{elf, symtab.offset(), elf.sections[symtab.link()].offset(), ptr} {
			assert(symtab.type() == Def::SHT_SYMTAB || symtab.type() == Def::SHT_DYNSYM);
			assert(elf.sections[symtab.link()].type() == Def::SHT_STRTAB);
		}

		/*! \brief Construct relocation entry (with addend)
		 * \param elf ELF object to which this relocation belongs to
		 * \param symtaboff Offset to the symbol table for this relocation
		 * \param strtaboff Offset to the string table (required by the symbol table)
		 * \param ptr Pointer to the memory containting the current relocation
		 */
		explicit RelocationWithAddend(const ELF<C> & elf, uintptr_t symtaboff = 0, uintptr_t strtaboff = 0, void * ptr = nullptr)
		  : Accessor<typename Def::Rela>{elf, reinterpret_cast<typename Def::Rela *>(ptr)}, symtaboff{symtaboff}, strtaboff{strtaboff} {}

		/*! \brief Valid relocation */
		bool valid() const {
			return symtaboff != 0 && strtaboff != 0;
		}

		/*! \brief Address */
		uintptr_t offset() const {
			return this->_data->r_offset;
		}

		/*! \brief Relocation type and symbol index */
		uintptr_t info() const {
			return this->_data->r_info.value;
		}

		/*! \brief Target symbol */
		Symbol symbol() const {
			return Symbol{this->_elf, strtaboff, this->_elf.data(symtaboff + this->_data->r_info.sym * sizeof(typename Def::Sym))};
		}

		/*! \brief Index of target symbol in corresponding symbol table */
		uint32_t symbol_index() const {
			return this->_data->r_info.sym;
		}

		/*! \brief Relocation type (depends on architecture) */
		uint32_t type() const {
			return static_cast<uint32_t>(this->_data->r_info.type);
		}

		/*! \brief Addend */
		intptr_t addend() const {
			return this->_data->r_addend;
		}
	};

	/*! \brief Generic interface for relocations */
	struct Relocation : Accessor<void> {
		/*! \brief Corresponding symbol table offset */
		const uintptr_t symtaboff;
		/*! \brief String table offset (for symbol table) */
		const uintptr_t strtaboff;

		/*! \brief Does the structure contain an addend field? */
		const bool withAddend;

		/*! \brief Construct relocation entry (with addend)
		 * \param elf ELF object to which this relocation belongs to
		 * \param symtab Symbol table index for this relocation
		 * \param withAddend Relocation with (`true`) or without (`false`) addend
		 * \param ptr Pointer to the memory containting the current relocation
		 */
		Relocation(const ELF<C> & elf, uint16_t symtab, bool withAddend, void * ptr = nullptr)
		  : Relocation{elf, elf.sections[symtab], withAddend, ptr} {}

		/*! \brief Construct relocation entry (with addend)
		 * \param elf ELF object to which this relocation belongs to
		 * \param symtab Symbol table section for this relocation
		 * \param withAddend Relocation with (`true`) or without (`false`) addend
		 * \param ptr Pointer to the memory containting the current relocation
		 */
		Relocation(const ELF<C> & elf, const Section & symtab, bool withAddend, void * ptr = nullptr)
		  : Relocation{elf, symtab.offset(), elf.sections[symtab.link()].offset(), withAddend, ptr} {
			assert(symtab.type() == Def::SHT_SYMTAB || symtab.type() == Def::SHT_DYNSYM);
			assert(elf.sections[symtab.link()].type() == Def::SHT_STRTAB);
		}

		/*! \brief Construct relocation entry
		 * \param elf ELF object to which this relocation belongs to
		 * \param symtaboff Offset to the symbol table for this relocation
		 * \param strtaboff Offset to the string table (required by the symbol table)
		 * \param withAddend Relocation with (`true`) or without (`false`) addend
		 * \param ptr Pointer to the memory containting the current relocation
		 */
		explicit Relocation(const ELF<C> & elf, uintptr_t symtaboff = 0, uintptr_t strtaboff = 0, bool withAddend = false, void * ptr = nullptr)
		  : Accessor<void>{elf, ptr}, symtaboff{symtaboff}, strtaboff{strtaboff}, withAddend{withAddend} {}

		/*! \brief Valid relocation */
		bool valid() const {
			return symtaboff != 0 && strtaboff != 0;
		}

		/*! \brief Address */
		uintptr_t offset() const {
			if (withAddend)
				return static_cast<const typename Def::Rela*>(this->_data)->r_offset;
			else
				return static_cast<const typename Def::Rel*>(this->_data)->r_offset;
		}

		/*! \brief Relocation type and symbol index */
		uintptr_t info() const {
			if (withAddend)
				return static_cast<const typename Def::Rela*>(this->_data)->r_info.value;
			else
				return static_cast<const typename Def::Rel*>(this->_data)->r_info.value;
		}

		/*! \brief Target symbol */
		Symbol symbol() const {
			return Symbol{this->_elf, strtaboff, this->_elf.data(symtaboff + symbol_index() * sizeof(typename Def::Sym))};
		}

		/*! \brief Index of target symbol in corresponding symbol table */
		uint32_t symbol_index() const {
			if (withAddend)
				return static_cast<const typename Def::Rela*>(this->_data)->r_info.sym;
			else
				return static_cast<const typename Def::Rel*>(this->_data)->r_info.sym;

		}

		/*! \brief Relocation type (depends on architecture) */
		uint32_t type() const {
			if (withAddend)
				return static_cast<uint32_t>(static_cast<const typename Def::Rela*>(this->_data)->r_info.type);
			else
				return static_cast<uint32_t>(static_cast<const typename Def::Rel*>(this->_data)->r_info.type);
		}

		/*! \brief Addend */
		intptr_t addend() const {
			return withAddend ? static_cast<const typename Def::Rela*>(this->_data)->r_addend : 0;
		}

		/*! \brief Pointer to next element (depending on underlying structure) */
		const void * next(size_t i = 1) const {
			if (withAddend)
				return static_cast<const typename Def::Rela*>(this->_data) + i;
			else
				return static_cast<const typename Def::Rel*>(this->_data) + i;
		}

		/*! \brief Size (in bytes) of an entry */
		size_t element_size() const {
			if (withAddend)
				return sizeof(typename Def::Rela);
			else
				return sizeof(typename Def::Rel);
		}
	};

	/*! \brief Dynamic table entry */
	struct Dynamic : Accessor<typename Def::Dyn> {
		/*! \brief Index of string table */
		const uintptr_t strtaboff;

		/*! \brief construct dynamic table entry
		 * \param elf ELF object to which this symbol belongs to
		 * \param strtab Index of string table
		 * \param ptr Pointer to the memory containting the current relocation
		 */
		Dynamic(const ELF<C> & elf, uint16_t strtab, void * ptr = nullptr)
		  : Dynamic{elf, elf.sections[strtab], ptr} {}

		/*! \brief construct dynamic table entry
		 * \param elf ELF object to which this symbol belongs to
		 * \param strtab String table section
		 * \param ptr Pointer to the memory containting the current relocation
		 */
		Dynamic(const ELF<C> & elf, const Section & strtab, void * ptr = nullptr)
		  : Dynamic{elf, strtab.offset(), ptr} {
			assert(strtab.type() == Def::SHT_STRTAB);
		}

		/*! \brief construct dynamic table entry
		 * \param elf ELF object to which this symbol belongs to
		 * \param strtaboff Offset to string table
		 * \param ptr Pointer to the memory containting the current dynamic entry
		 */
		explicit Dynamic(const ELF<C> & elf, uintptr_t strtaboff = 0, void * ptr = nullptr)
		  : Accessor<typename Def::Dyn>{elf, reinterpret_cast<typename Def::Dyn *>(ptr)}, strtaboff{strtaboff} {}

		/*! \brief Valid dynamic table? */
		bool valid() const {
			return strtaboff != 0;
		}

		/*! \brief Tag of entry */
		typename Def::dyn_tag tag() const {
			return (typename Def::dyn_tag)(this->_data->d_tag);
		}

		/*! \brief Value of entry */
		uintptr_t value() const {
			return this->_data->d_un.d_val;
		}

		/*! \brief Associated string
		 * \note Availability depending on tag!
		 */
		const char * string() const {
			return reinterpret_cast<const char *>(this->_elf.data(strtaboff + this->_data->d_un.d_val));
		}
	};

	/*! \brief GNU Note entry */
	struct Note : Accessor<typename Def::Nhdr> {
		/*! \brief Construct note entry
		 * \param elf ELF object to which this note belongs to
		 * \param link Associated section index (must be `0`)
		 * \param ptr Pointer to the memory containting the current note
		 */
		Note(const ELF<C> & elf, uint16_t link, void * ptr = nullptr)
		  : Accessor<typename Def::Nhdr>{elf, reinterpret_cast<typename Def::Nhdr *>(ptr)} {
			assert(link == 0);
		}

		/*! \brief Construct note entry
		 * \param elf ELF object to which this note belongs to
		 * \param section Associated section (must be of type `SHT_NULL`)
		 * \param ptr Pointer to the memory containting the current note
		 */
		Note(const ELF<C> & elf, const Section & section, void * ptr = nullptr)
		  : Accessor<typename Def::Nhdr>{elf, reinterpret_cast<typename Def::Nhdr *>(ptr)} {
			assert(section.type() == Def::SHT_NULL);
		}

		/*! \brief Construct note entry
		 * \param elf ELF object to which this note belongs to
		 * \param offset Associated section offset (must be `0`)
		 * \param ptr Pointer to the memory containting the current note
		 */
		explicit Note(const ELF<C> & elf, uintptr_t offset = 0, void * ptr = nullptr) : Accessor<typename Def::Nhdr>(elf, reinterpret_cast<typename Def::Nhdr *>(ptr)) {
			assert(offset == 0);
		}

		/*! \brief Note name */
		const char * name() const {
			return this->_data->n_namesz == 0 ? nullptr : (reinterpret_cast<const char *>(this->_data) + sizeof(typename Def::Nhdr));
		}

		/*! \brief Note description */
		const void * description() const {
			return this->_data->n_descsz == 0 ? nullptr : reinterpret_cast<const void *>(reinterpret_cast<uintptr_t>(this->_data) + sizeof(typename Def::Nhdr) + align(this->_data->n_namesz));
		}

		/*! \brief description size (in bytes!)
		 */
		size_t size() const {
			return this->_data->n_descsz;
		}

		/*! \brief Note type */
		typename Def::nhdr_type type() const {
			return this->_data->n_type;
		}

	 private:
		friend class List<Note>;
		friend class Iterator<Note>;

		/*! \brief Next element
		 * \param i the i-th next successor (only `1` is valid!)
		 * \return pointer to next element
		 */
		typename Def::Nhdr * next(size_t i = 1) const {
			assert(i == 1);
			uintptr_t next = reinterpret_cast<uintptr_t>(this->_data)
			               + sizeof(typename Def::Nhdr)
			               + align(this->_data->n_namesz)
			               + align(this->_data->n_descsz);
			return reinterpret_cast<typename Def::Nhdr*>(next);
		}

		/*! \brief Helper to perform 4 byte alignment of value */
		template<typename T>
		static inline T align(T size) {
			return (size + 3)  & (~0x3);  // 4 byte alignment
		}
	};

	/*! \brief Version definition entry */
	struct VersionDefinition : Accessor<typename Def::Verdef> {
		/*! \brief Auxiliary information for version definition */
		struct Auxiliary : Accessor<typename Def::Verdaux> {
			/*! \brief String table offset */
			const uintptr_t strtaboff;

			/*! \brief Construct auxiliary entry for version definition
			 * \param elf ELF object to which this entry belongs to
			 * \param strtab String table index for this erntry
			 */
			Auxiliary(const ELF<C> & elf, uintptr_t strtaboff)
			  : Accessor<typename Def::Verdaux>{elf}, strtaboff{strtaboff} {}

			/*! \brief Definition name */
			const char * name() const {
				assert(strtaboff != 0);
				return reinterpret_cast<const char *>(this->_elf.data(strtaboff + this->_data->vda_name));
			}

		 private:
			friend class List<VersionDefinition::Auxiliary>;
			friend class Iterator<VersionDefinition::Auxiliary>;

			/*! \brief Next element
			 * \param i the i-th next successor (only `1` is valid!)
			 * \return pointer to next element or `nullptr` if end
			 */
			typename Def::Verdaux * next(size_t i = 1) const {
				assert(i == 1);
				return this->_data->vda_next == 0 ? nullptr : reinterpret_cast<typename Def::Verdaux*>(reinterpret_cast<uintptr_t>(this->_data) + this->_data->vda_next);
			}
		};

		friend class List<VersionDefinition>;
		friend class Iterator<VersionDefinition>;

		/*! \brief Next element
		 * \param i the i-th next successor (only `1` is valid!)
		 * \return pointer to next element or `nullptr` if end
		 */
		typename Def::Verdef * next(size_t i = 1) const {
			assert(i == 1);
			uintptr_t next_adr = reinterpret_cast<uintptr_t>(this->_data) + this->_data->vd_next;
			return this->_data->vd_next == 0 ? nullptr : reinterpret_cast<typename Def::Verdef*>(next_adr);
		}

	public:
		/*! \brief String table index */
		const uintptr_t strtaboff;

		/*! \brief Construct version definition entry
		 * \param elf ELF object to which this entry belongs to
		 * \param strtab String table index for this entry
		 * \param ptr Pointer to the memory containting the current symbol
		 */
		VersionDefinition(const ELF<C> & elf, uint16_t strtab, void * ptr = nullptr)
		  : VersionDefinition{elf, elf.sections[strtab], ptr} {}

		/*! \brief Construct version definition entry
		 * \param elf ELF object to which this entry belongs to
		 * \param strtab String table index for this entry
		 * \param ptr Pointer to the memory containting the current symbol
		 */
		VersionDefinition(const ELF<C> & elf, const Section & strtab, void * ptr = nullptr)
		  : VersionDefinition{elf, strtab.offset(), ptr} {
			assert(strtab.type() == Def::SHT_STRTAB);
		}

		/*! \brief Construct version definition entry
		 * \param elf ELF object to which this entry belongs to
		 * \param strtaboff Offset to string table for this entry
		 * \param ptr Pointer to the memory containting the current version definition
		 */
		explicit VersionDefinition(const ELF<C> & elf, uintptr_t strtaboff = 0, void * ptr = nullptr)
		  : Accessor<typename Def::Verdef>{elf, reinterpret_cast<typename Def::Verdef *>(ptr)}, strtaboff{strtaboff} {}

		/*! \brief Version revision */
		uint16_t revision() const {
			return this->_data->vd_version;
		}

		/*! \brief Version information */
		uint16_t flags() const {
			return this->_data->vd_flags;
		}

		/*! \brief Weak linkage */
		bool weak() const {
			return this->_data->vd_weak == 1;
		}

		/*! \brief Is this the version definition of the file? */
		bool base() const {
			return this->_data->vd_base == 1;
		}

		/*! \brief Version index (as used in version symbol table) */
		uint16_t version_index() const {
			return this->_data->vd_ndx;
		}

		/*! \brief hash value of definition name */
		uint32_t hash() const {
			return this->_data->vd_hash;
		}

		/*! \brief List of version definition auxiliary information */
		List<Auxiliary> auxiliary() const {
			uintptr_t first_adr = reinterpret_cast<uintptr_t>(this->_data) + this->_data->vd_aux;
			typename Def::Verdaux * first = this->_data->vd_aux == 0 ? nullptr : reinterpret_cast<typename Def::Verdaux *>(first_adr);
			return { Auxiliary{this->_elf, strtaboff}, first, nullptr };
		}

		/*! \brief Number of version definition auxiliary information */
		uint16_t auxiliaries() const {
			return this->_data->vd_cnt;
		}
	};

	/*! \brief Version needed entry */
	struct VersionNeeded : Accessor<typename Def::Verneed> {
		/*! \brief Auxiliary information for version needed */
		struct Auxiliary : Accessor<typename Def::Vernaux> {
			/*! \brief String table offset */
			const uintptr_t strtaboff;

			/*! \brief Construct auxiliary entry for version definition
			 * \param elf ELF object to which this entry belongs to
			 * \param strtab String table offset for this entry
			 */
			Auxiliary(const ELF<C> & elf, uintptr_t strtaboff)
			  : Accessor<typename Def::Vernaux>{elf}, strtaboff{strtaboff} {}

			/*! \brief hash value of dependency name */
			uint32_t hash() const {
				return this->_data->vna_hash;
			}

			/*! \brief Dependency specific information */
			uint16_t flags() const {
				return this->_data->vna_flags;
			}

			/*! \brief Weak linkage */
			bool weak() const {
				return this->_data->vna_weak == 1;
			}

			/*! \brief Version index (as used in version symbol table) */
			uint16_t version_index() const {
				return this->_data->vna_other;
			}

			/*! \brief Dependency name */
			const char * name() const {
				return this->_elf.string(strtaboff, this->_data->vna_name);
			}

		 private:
			friend class List<VersionNeeded::Auxiliary>;
			friend class Iterator<VersionNeeded::Auxiliary>;

			/*! \brief Next element
			 * \param i the i-th next successor (only `1` is valid!)
			 * \return pointer to next element or `nullptr` if end
			 */
			typename Def::Vernaux * next(size_t i = 1) const {
				assert(i == 1);
				return this->_data->vna_next == 0 ? nullptr : reinterpret_cast<typename Def::Vernaux*>(reinterpret_cast<uintptr_t>(this->_data) + this->_data->vna_next);
			}
		};

		friend class List<VersionNeeded>;
		friend class Iterator<VersionNeeded>;

		/*! \brief Next element
		 * \param i the i-th next successor (only `1` is valid!)
		 * \return pointer to next element or `nullptr` if end
		 */
		typename Def::Verneed * next(size_t i = 1) const {
			assert(i == 1);
			uintptr_t next_adr = reinterpret_cast<uintptr_t>(this->_data) + this->_data->vn_next;
			return this->_data->vn_next == 0 ? nullptr : reinterpret_cast<typename Def::Verneed*>(next_adr);
		}

	public:
		/*! \brief String table index */
		const uintptr_t strtaboff;

		/*! \brief Construct version needed entry
		 * \param elf ELF object to which this entry belongs to
		 * \param strtab String table offset for this entry
		 * \param ptr Pointer to the memory containting the current needed version
		 */
		VersionNeeded(const ELF<C> & elf, uint16_t strtab, void * ptr = nullptr)
		  : VersionNeeded{elf, elf.sections[strtab], ptr} {}

		/*! \brief Construct version needed entry
		 * \param elf ELF object to which this entry belongs to
		 * \param strtab String table section for this entry
		 * \param ptr Pointer to the memory containting the current needed version
		 */
		VersionNeeded(const ELF<C> & elf, const Section & strtab, void * ptr = nullptr)
		  : VersionNeeded{elf, strtab.offset(), ptr} {
			assert(strtab.type() == Def::SHT_STRTAB);
		}

		/*! \brief Construct version needed entry
		 * \param elf ELF object to which this entry belongs to
		 * \param strtaboff Offset to string table for this entry
		 * \param ptr Pointer to the memory containting the current needed version
		 */
		explicit VersionNeeded(const ELF<C> & elf, uintptr_t strtaboff = 0, void * ptr = nullptr)
		  : Accessor<typename Def::Verneed>{elf, reinterpret_cast<typename Def::Verneed *>(ptr)}, strtaboff{strtaboff} {}

		/*! \brief Version for this dependency */
		typename Def::verneed_version version() const {
			return this->_data->vn_version;
		}

		/*! \brief filename for this dependency */
		const char * file() const {
			assert(strtaboff != 0);
			return reinterpret_cast<const char *>(this->_elf.data(strtaboff + this->_data->vn_file));
		}

		/*! \brief List of version dependency auxiliary information */
		List<Auxiliary> auxiliary() const {
			uintptr_t first_adr = reinterpret_cast<uintptr_t>(this->_data) + this->_data->vn_aux;
			typename Def::Vernaux * first = this->_data->vn_aux == 0 ? nullptr : reinterpret_cast<typename Def::Vernaux *>(first_adr);
			return { Auxiliary{this->_elf, strtaboff}, first, nullptr };
		}

		/*! \brief Number of version dependency auxiliary information */
		uint16_t auxiliaries() const {
			return this->_data->vn_cnt;
		}
	};

	/*! \brief Helper to access the Dynamic Section */
	struct DynamicTable : public Array<Dynamic> {
		/*! \brief Translate from virtual memory to file offset (required if not mapped according to segments) */
		const bool translate_address;
		/*! \brief Offsets are absolute address (= non dynamic Elf) */
		const bool absolute_address;

		/*! \brief Filter list entries */
		struct Entry : public Dynamic {
			/*! \brief Filter Tag */
			const typename Def::dyn_tag filter;

			/*! \brief Construct a new filtered dynamic entry
			 * \param elf ELF object to which this entry belongs to
			 * \param strtab String table offset for this entry
			 */
			Entry(const ELF<C> & elf, uintptr_t strtaboff = 0, typename Def::dyn_tag filter = Def::DT_NULL)
			  : Dynamic{elf, strtaboff}, filter{filter} {}

		 private:
			friend class DynamicTable;
			friend class List<DynamicTable::Entry>;
			friend class Iterator<DynamicTable::Entry>;

			/*! \brief Next element
			 * \param i the i-th next successor (only `1` is valid!)
			 * \return pointer to next element or `nullptr` if end
			 */
			const typename Def::Dyn * next(size_t i = 1) const {
				assert(i == 1);
				return find(this->_data + 1, filter);
			}

			/*! \brief Get element matching filter */
			static const typename Def::Dyn * find(const typename Def::Dyn * d, typename Def::dyn_tag filter) {
				for (; d->d_tag != Def::DT_NULL; d++)
					if (d->d_tag == filter)
						return d;
				return nullptr;
			}
		};

		/*! \brief Dynamic table
		 * similar to dynamic array, but offering easy access functions to its contents
		 */
		DynamicTable(const ELF<C> & elf, const Section & section)
		  : DynamicTable{elf, section.data(), section.dynamic_entries(), elf.sections[section.link()].offset(), section.virt_addr() != section.offset(), this->_elf.header.type() == Def::ET_EXEC} {
			assert(section.type() == Def::SHT_DYNAMIC);
			assert(elf.sections[section.link()].type() == Def::SHT_STRTAB);
		}

		/*! \brief Raw dynamic table
		 * \param elf Pointer to elf
		 * \param dyntab Pointer to dynamic table
		 * \param dyntabentries Number of entries in dynamic table
		 * \param strtaboff Offset to associated string table
		 * \param translate_address Difference between virtual address (used in dynamic) and file offset needs fix of offset
		 */
		DynamicTable(const ELF<C> & elf, void * dyntab, size_t dyntabentries, uintptr_t strtaboff, bool translate_address, bool absolute_address)
		  : Array<Dynamic>{Dynamic{elf, strtaboff}, dyntab, dyntabentries}, translate_address{translate_address}, absolute_address(absolute_address) {}

		/*! \brief Empty (non-existing) dynamic table */
		explicit DynamicTable(const ELF<C> & elf)
		  : Array<Dynamic>{Dynamic{elf}, 0, 0}, translate_address{false}, absolute_address{false}  {}

		/*! \brief Get the corresponding ELF */
		const ELF<C> & elf() const {
			return this->_accessor._elf;
		}

		/*! \brief Pointer to the global offset table */
		const char * get_soname() const {
			for (const auto & dyn : *this)
				if (dyn.tag() == Def::DT_SONAME)
					return dyn.string();
			return nullptr;
		}

		/*! \brief get list of dependency library filename */
		List<Entry> get_needed() const {
			return get_entry(Def::DT_NEEDED);
		}

		/*! \brief get list of dependency lookup path (rpath) */
		List<Entry> get_rpath() const {
			return get_entry(Def::DT_RPATH);
		}

		/*! \brief get list of dependency lookup path (runpath) */
		List<Entry> get_runpath() const {
			return get_entry(Def::DT_RUNPATH);
		}

		/*! \brief get flags */
		uintptr_t flags(bool one = false) const {
			return operator[](one ? Def::DT_FLAGS_1 : Def::DT_FLAGS);
		}

		/*! \brief Get contents of symbol table section as \ref Array of \ref Symbol elements  */
		Array<Symbol> get_symbols() const {
			uintptr_t strtab = 0;
			void * symtab = nullptr;
			size_t symtabnum = 0;

			for (const auto &dyn: *this) {
				switch (dyn.tag()) {
					case Def::DT_STRTAB:
						strtab = fix_offset(dyn.value());;
						break;
					case Def::DT_SYMTAB:
						symtab = data(dyn.value());
						break;
					case Def::DT_SYMENT:
						assert(dyn.value() == sizeof(typename Def::Sym));
						break;
					case Def::DT_HASH:
						symtabnum = reinterpret_cast<const ELF_Def::Hash_header*>(data(dyn.value()))->nchain;
						break;
					case Def::DT_GNU_HASH:
						symtabnum = gnu_hash_size(reinterpret_cast<const ELF_Def::GnuHash_header*>(data(dyn.value())));
						break;
				default:
						continue;
				}
			}
			assert(symtab != 0 && strtab != 0);
			return { Symbol{elf(), strtab}, symtab, symtabnum };
		}

		/*! \brief Get contents of symbol table section
		 */
		SymbolTable get_symbol_table() const {
			uintptr_t strtab = 0;
			void * symtab = nullptr;
			size_t symtabnum = 0;
			typename Def::shdr_type section_type = Def::SHT_DYNSYM;
			void * header = nullptr;
			const uint16_t * versions = nullptr;

			for (const auto &dyn: *this) {
				switch (dyn.tag()) {
					case Def::DT_STRTAB:
						strtab = fix_offset(dyn.value());
						break;
					case Def::DT_SYMTAB:
						symtab = data(dyn.value());
						break;
					case Def::DT_SYMENT:
						assert(dyn.value() == sizeof(typename Def::Sym));
						break;
					case Def::DT_VERSYM:
						versions = reinterpret_cast<const uint16_t *>(data(dyn.value()));
						break;
					case Def::DT_HASH:
						// Gnu hash is superior
						if (section_type == Def::SHT_DYNSYM) {
							section_type = Def::SHT_HASH;
							header = data(dyn.value());
							symtabnum = reinterpret_cast<const ELF_Def::Hash_header*>(header)->nchain;
						}
						break;
					case Def::DT_GNU_HASH:
						section_type = Def::SHT_GNU_HASH;
						header = data(dyn.value());
						symtabnum = gnu_hash_size(reinterpret_cast<const ELF_Def::GnuHash_header*>(header));
						break;
					default:
						continue;
				}
			}
			assert(symtab != nullptr && strtab != 0);
			assert(header != nullptr);  // hash table is mandatory
			return SymbolTable{elf(), section_type, header, symtab, symtabnum, versions, strtab};
		}

		/*! \brief Get contents of version definition section as \ref List of \ref VersionDefinition elements  */
		List<VersionDefinition> get_version_definition() const {
			uintptr_t strtab = 0;
			void * verdef = nullptr;
			uintptr_t verdefnum = 0;

			for (const auto & dyn : *this) {
				switch (dyn.tag()) {
					case Def::DT_STRTAB:
						strtab = fix_offset(dyn.value());
						break;
					case Def::DT_VERDEF:
						verdef = data(dyn.value());
						break;
					case Def::DT_VERDEFNUM:
						verdefnum = dyn.value();
						break;
					default:
						continue;
				}
			}

			if (verdef == 0) {
				assert(verdefnum == 0);
				return { VersionDefinition{elf()}, nullptr, nullptr };
			} else {
				const auto & l = List<VersionDefinition>{ VersionDefinition{elf(), strtab}, verdef, nullptr };
				assert(l.count() == verdefnum);
				return l;
			}
		}

		/*! \brief Get contents of version needed section as \ref List of \ref VersionNeeded elements  */
		List<VersionNeeded> get_version_needed() const {
			uintptr_t strtab = 0;
			void * verneed = nullptr;
			uintptr_t verneednum = 0;

			for (const auto & dyn : *this) {
				switch (dyn.tag()) {
					case Def::DT_STRTAB:
						strtab = fix_offset(dyn.value());
						break;
					case Def::DT_VERNEED:
						verneed = data(dyn.value());
						break;
					case Def::DT_VERNEEDNUM:
						verneednum = dyn.value();
						break;
					default:
						continue;
				}
			}

			if (verneed == 0) {
				assert(verneednum == 0);
				return { VersionNeeded{elf()}, nullptr, nullptr };
			} else {
				const auto & l = List<VersionNeeded>{ VersionNeeded{elf(), strtab}, verneed, nullptr };
				assert(l.count() == verneednum);
				return l;
			}
		}

		/*! \brief Get relocations in dynamic section (excluding procedure linkage table) */
		Array<Relocation> get_relocations() const {
			uintptr_t strtab = 0;                       // Offset of string table
			uintptr_t symtab = 0;                       // Offset of symbol table
			void * rel = nullptr;                       // Pointer to thel PLT relocation table
			typename Def::dyn_tag type = Def::DT_NULL;  // Type of relocation table (REL or RELA)
			size_t relsz = 0;                           // Size of relocation table
			size_t relent = 0;                          // Size of relocation table entry

			for (const auto &dyn: *this) {
				switch (dyn.tag()) {
					case Def::DT_STRTAB:
						strtab = fix_offset(dyn.value());
						break;
					case Def::DT_SYMTAB:
						symtab = fix_offset(dyn.value());
						break;
					case Def::DT_SYMENT:
						assert(dyn.value() == sizeof(typename Def::Sym));
						break;
					case Def::DT_REL:
						assert(type == Def::DT_NULL || type == Def::DT_REL);
						type = Def::DT_REL;
						rel = data(dyn.value());
						break;
					case Def::DT_RELA:
						assert(type == Def::DT_NULL || type == Def::DT_RELA);
						type = Def::DT_RELA;
						rel = data(dyn.value());
						break;
					case Def::DT_RELSZ:
						assert(type == Def::DT_NULL || type == Def::DT_REL);
						type = Def::DT_REL;
						relsz = dyn.value();
						break;
					case Def::DT_RELASZ:
						assert(type == Def::DT_NULL || type == Def::DT_RELA);
						type = Def::DT_RELA;
						relsz = dyn.value();
						break;
					case Def::DT_RELENT:
						assert(type == Def::DT_NULL || type == Def::DT_REL);
						type = Def::DT_REL;
						relent = dyn.value();
						assert(relent == sizeof(typename Def::Rel));
						break;
					case Def::DT_RELAENT:
						assert(type == Def::DT_NULL || type == Def::DT_RELA);
						type = Def::DT_RELA;
						relent = dyn.value();
						assert(relent == sizeof(typename Def::Rela));
						break;
					default:
						continue;
				}
			}

			if (type == Def::DT_NULL) {
				assert(rel == nullptr && relsz == 0 && relent == 0);
				return { Relocation{elf()}, nullptr, 0 };
			} else {
				assert(rel != nullptr && symtab != 0 && strtab != 0 && relent != 0);
				assert(type == Def::DT_REL || type == Def::DT_RELA);
				return { Relocation{elf(), symtab, strtab, type == Def::DT_RELA}, rel, relsz / relent };
			}
		}

		/*! \brief Get relocations for the procedure linkage table */
		Array<Relocation> get_relocations_plt() const {
			uintptr_t strtab = 0;                         // Offset of string table
			uintptr_t symtab = 0;                         // Offset of symbol table
			void * jmprel = nullptr;                      // Pointer to thel PLT relocation table
			typename Def::dyn_tag pltrel = Def::DT_NULL;  // Type of PLT relocation table (REL or RELA)
			size_t pltrelsz = 0;                          // Size of PLT relocation table

			for (const auto &dyn: *this) {
				switch (dyn.tag()) {
					case Def::DT_STRTAB:
						strtab = fix_offset(dyn.value());
						break;
					case Def::DT_SYMTAB:
						symtab = fix_offset(dyn.value());
						break;
					case Def::DT_JMPREL:
						jmprel = data(dyn.value());
						break;
					case Def::DT_PLTREL:
						pltrel = static_cast<typename Def::dyn_tag>(dyn.value());
						break;
					case Def::DT_PLTRELSZ:
						pltrelsz = dyn.value();
						break;
					default:
						continue;
				}
			}

			switch (pltrel) {
				case Def::DT_NULL:
					assert(jmprel == nullptr && pltrelsz == 0);
					break;
				case Def::DT_REL:
					assert(jmprel != nullptr && symtab != 0 && strtab != 0);
					return { Relocation{elf(), symtab, strtab, false}, jmprel, pltrelsz / sizeof(typename Def::Rel) };
				case Def::DT_RELA:
					assert(jmprel != nullptr && symtab != 0 && strtab != 0);
					return { Relocation{elf(), symtab, strtab, true}, jmprel, pltrelsz / sizeof(typename Def::Rela) };
				default:
					assert(false);
			}
			return { Relocation{elf()}, nullptr, 0 };
		}

		// Function pointer type
		typedef void (*func_init_t)(int argc, const char **argv, const char **envp);
		typedef void (*func_fini_t)();

		/*! \brief Pre-Init functions array */
		Array<Accessor<func_init_t>> get_preinit_array(uintptr_t offset = 0) const {
			return get_func<func_init_t>(Def::DT_PREINIT_ARRAY, Def::DT_PREINIT_ARRAYSZ, offset);
		}

		/*! \brief Get initialization function pointer */
		func_init_t get_init_function(uintptr_t offset = 0) const {
			for (const auto & dyn : *this)
				if (dyn.tag() == Def::DT_INIT)
					return reinterpret_cast<func_init_t>(dyn.value() + offset);
			return nullptr;
		}

		/*! \brief Init functions array */
		Array<Accessor<func_init_t>> get_init_array(uintptr_t offset = 0) const {
			return get_func<func_init_t>(Def::DT_INIT_ARRAY, Def::DT_INIT_ARRAYSZ, offset);
		}

		/*! \brief run initialization */
		void init(int argc, const char **argv, const char **envp, uintptr_t offset = 0) const {
			for (const auto & f : get_preinit_array(offset))
				f.data()(argc, argv, envp);

			auto f = get_init_function(offset);
			if (f != nullptr)
				f(argc, argv, envp);

			for (const auto & f : get_init_array(offset))
				f.data()(argc, argv, envp);
		}

		/*! \brief De-init functions array */
		Array<Accessor<func_fini_t>> get_fini_array(uintptr_t offset = 0) const {
			return get_func<func_fini_t>(Def::DT_FINI_ARRAY, Def::DT_FINI_ARRAYSZ, offset);
		}

		/*! \brief Get deinitialization function pointer */
		func_fini_t get_fini_function(uintptr_t offset = 0) const {
			for (const auto & dyn : *this)
				if (dyn.tag() == Def::DT_FINI)
					return reinterpret_cast<func_fini_t>(dyn.value() + offset);
			return nullptr;
		}

		/*! \brief run deinitialization */
		void fini(uintptr_t offset = 0) const {
			for (const auto & f : get_fini_array(offset))
				f.data()();

			auto f = get_fini_function(offset);
			if (f != nullptr)
				f();
		}


		/*! \brief Contents of the global offset table */
		Array<Accessor<void*>> get_global_offset_table() const {
			void * got = nullptr;
			size_t size = 0;
			size_t entry_size = 0;

			for (const auto & dyn : *this) {
				switch (dyn.tag()) {
					case Def::DT_PLTGOT:
						got = data(dyn.value());
						break;
					case Def::DT_PLTRELSZ:
						size = dyn.value();
						break;
					case Def::DT_PLTREL:
						switch(dyn.value()) {
							case Def::DT_REL:
								entry_size = sizeof(typename Def::Rel);
								break;
							case Def::DT_RELA:
								entry_size = sizeof(typename Def::Rela);
								break;
							default:
								assert(false);
						}
						break;
					default:
						continue;
				}
			}

			assert(size == 0 || entry_size != 0);
			return { Accessor<void*>{elf()}, got, size > 0 ? 3 + size / entry_size : 0 };
		}

		/*! \brief Pointer to the global offset table */
		void** get_global_offset_table_pointer() const {
			for (const auto & dyn : *this)
				if (dyn.tag() == Def::DT_PLTGOT)
					return reinterpret_cast<void**>(data(dyn.value()));
			return nullptr;
		}

		/*! \brief Access (first) dynamic entry
		 * \param tag dynamic tag to search
		 * \return Dynamic entry
		 */
		inline Dynamic operator[](typename Def::dyn_tag tag) const {
			for (const auto & dyn : *this)
				if (dyn.tag() == tag)
					return dyn;
			return Dynamic{elf()};  // 0 == UNDEF
		}

	 private:
		friend struct Segment;

		/*! \brief Helper to determine the size of entries in gnu hash table */
		static size_t gnu_hash_size(const ELF_Def::GnuHash_header* header) {
			const elfptr_t * bloom = reinterpret_cast<const elfptr_t *>(header + 1);
			const uint32_t * buckets = reinterpret_cast<const uint32_t *>(bloom + header->bloom_size);
			size_t n = 0;
			for (uint32_t i = 0; i < header->nbuckets; i++)
				if (buckets[i] > n)
					n = buckets[i];

			if (n == 0)
				return header->symoffset;  // TODO: If there are only undefined symbols, this will be set to `1` -- we cannot determine the size...

			for (const uint32_t * chain = buckets + header->nbuckets - header->symoffset; (chain[n] & 1) == 0; n++) {}
			return n + 1;
		}

		/*! \brief Helper to get filtered list */
		List<Entry> get_entry(typename Def::dyn_tag filter) const {
			return { Entry{elf(), this->_accessor.strtaboff, filter}, const_cast<void *>(reinterpret_cast<const void *>(Entry::find(this->_accessor._data, filter))), nullptr };
		}

		/*! \brief Helper to get function pointer array */
		template<typename F>
		Array<Accessor<F>> get_func(typename Def::dyn_tag tag_start, typename Def::dyn_tag tag_size, uintptr_t offset = 0) const {
			void * start = nullptr;
			size_t size = 0;
			for (const auto & dyn : *this) {
				if (dyn.tag() == tag_start)
					start = offset == 0 ? data(dyn.value()) : reinterpret_cast<void*>(offset + dyn.value());
				else if (dyn.tag() == tag_size)
					size = dyn.value();
			}
			return { Accessor<F>{elf()}, start, size / sizeof(void*) };
		}

		/*! \brief add offset to function pointer */
		template<typename F>
		static F func_offset(F f, uintptr_t offset) {
			return reinterpret_cast<F>(reinterpret_cast<uintptr_t>(f) + offset);
		}

		/*! \brief translate virtual address according to load segments into offsets*/
		static uintptr_t translate(const ELF<C> & elf, uintptr_t offset) {
			for (const auto & s : elf.segments)
				if (s.type() == Def::PT_LOAD && offset >= s.virt_addr() && offset <= s.virt_addr() + s.size())  {
					assert(offset + s.offset() >= s.virt_addr());
					return offset + s.offset() - s.virt_addr();
				}
			// Memory not available (BSS?)
			assert(false);
			return 0;
		}

		/*! \brief fix virtual address offset if not mapped according to load segment
		 *  \param offset value from dynamic table
		 *  \return relative offset to start of elf
		 */
		inline uintptr_t fix_offset(uintptr_t offset) const {
			if (translate_address)
				return translate(elf(), offset);
			else if (absolute_address)
				return offset - elf().start();
			else
				return offset;
		}

		/*! \brief get ELF memory address
		 *  \param offset value from dynamic table
		 *  \return pointer to element in (current) memory
		 */
		inline void * data(uintptr_t offset) const {
			if (translate_address)
				return elf().data(translate(elf(), offset));
			else if (absolute_address)
				return reinterpret_cast<void*>(offset);
			else
				return elf().data(offset);
		}
	};

	/*! \brief Entry of section header table */
	struct Section : Accessor<typename Def::Shdr> {
		/*! \brief Construct new section header entry
		 * \param elf ELF object to which this entry belongs to
		 * \param ptr Pointer to the memory containting the current section table entry
		 */
		explicit Section(const ELF<C> & elf, void * ptr = nullptr)
		  : Accessor<typename Def::Shdr>{elf, reinterpret_cast<typename Def::Shdr *>(ptr)} {}

		/*! \brief Section type */
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
		uintptr_t virt_addr() const {
			return this->_data->sh_addr;
		}

		/*! \brief Section file offset */
		uintptr_t offset() const {
			return this->_data->sh_offset;
		}

		/*! \brief Pointer to section contents
		 * \param displacement displacement in memory
		 * \return pointer to data
		 */
		void * data(uintptr_t displacement = 0) const {
			return this->_elf.data(this->_data->sh_offset + displacement);
		}

		/*! \brief Section size in bytes */
		size_t size() const {
			return this->_data->sh_size;
		}

		/*! \brief Entry size if section holds table */
		size_t entry_size() const {
			return this->_data->sh_entsize;
		}

		size_t entries() const {
			return entry_size() == 0 ? 0 : (size() / entry_size());
		}

		/*! \brief Section alignment */
		size_t alignment() const {
			return this->_data->sh_addralign;
		}

		/*! \brief Link to another section */
		uint16_t link() const {
			assert(this->_data->sh_link <= Def::SHN_HIRESERVE);
			return static_cast<uint16_t>(this->_data->sh_link);
		}

		/*! \brief Additional section information */
		uint32_t info() const {
			return this->_data->sh_info;
		}

		/*! \brief Section name */
		const char * name() const {
			return this->_elf.string(this->_elf.header.e_shstrndx, this->_data->sh_name);
		}

		/*! \brief Get contents of notes section as \ref List of \ref Note elements */
		List<Note> get_notes() const {
			assert(type() == Def::SHT_NOTE);
			return get_list<Note>();
		}

		/*! \brief Get contents of symbol version section as symbol index array
		 * \note Each version entry corresponds to the entry with the same index in the linked symbol table
		 */
		const uint16_t * get_versions() const {
			assert(type() == Def::SHT_GNU_VERSYM);
			assert(entry_size() == sizeof(uint16_t));
			return static_cast<uint16_t *>(data());
		}

		/*! \brief Get contents of version definition section as \ref List of \ref VersionDefinition elements  */
		List<VersionDefinition> get_version_definition() const {
			assert(type() == Def::SHT_GNU_VERDEF);
			return get_list<VersionDefinition>(true);
		}

		/*! \brief Get contents of version needed section as \ref List of \ref VersionNeeded elements  */
		List<VersionNeeded> get_version_needed() const {
			assert(type() == Def::SHT_GNU_VERNEED);
			return get_list<VersionNeeded>(true);
		}

		/*! \brief Get contents of symbol table section as \ref Array of \ref Symbol elements  */
		Array<Symbol> get_symbols() const {
			assert(type() == Def::SHT_SYMTAB || type() == Def::SHT_DYNSYM);
			return get_array<Symbol>();
		}

		/*! \brief Get contents of symbol table section
		 * \param version Pointer to version section (if available)
		 */
		SymbolTable get_symbol_table(const Section * const version = nullptr) const {
			assert(type() == Def::SHT_SYMTAB || type() == Def::SHT_DYNSYM || type() == Def::SHT_HASH || type() == Def::SHT_GNUHASH);
			return SymbolTable{this->_elf, *this, version};
		}

		/*! \brief Get contents of dynamic secion */
		Array<Dynamic> get_dynamic() const {
			assert(type() == Def::SHT_DYNAMIC);
			return { Dynamic{this->_elf, link()}, data(), dynamic_entries() };
		}

		/*! \brief Get contents of dynamic secion */
		DynamicTable get_dynamic_table() const {
			return DynamicTable{this->_elf, *this};
		}

		/*! \brief Get contents of relocation section */
		Array<Relocation> get_relocations() const {
			if (type() == Def::SHT_NULL) {
				return { Relocation{this->_elf}, nullptr, 0 };
			} else {
				assert(type() == Def::SHT_REL || type() == Def::SHT_RELA);
				return { Relocation{this->_elf, link(), type() == Def::SHT_RELA}, data(), entries() };
			}
		}

		/*! \brief Array with elements
		 * List of elements with fixed size and known length, hence random access is possible
		 * \return Array object
		 */
		template<typename ACCESSOR>
		inline Array<ACCESSOR> get_array() const {
			if (type() == Def::SHT_NULL) {
				return { ACCESSOR{this->_elf}, nullptr, 0 };
			} else {
				assert(entry_size() == ACCESSOR(this->_elf, link()).element_size());
				return { ACCESSOR{this->_elf, link()}, data(), entries() };
			}
		}

		/*! \brief Sequential list with elements
		 * List of elements with arbitrary size, hence only sequential access is possible
		 * \note `count()` and []-access has O(n) costs
		 * \param last_is_nullptr if true, then last element is indicated by `next() == nullptr`
		 * \return List object
		 */
		template<typename ACCESSOR>
		inline List<ACCESSOR> get_list(bool last_is_nullptr = false) const {
			if (type() == Def::SHT_NULL) {
				return { ACCESSOR{this->_elf}, nullptr, nullptr };
			} else {
				assert(entry_size() == 0);
				return { ACCESSOR{this->_elf, link()}, data(), last_is_nullptr ? nullptr : this->_elf.data(offset() + size()) };
			}
		}

	 private:
		friend struct DynamicTable;
		size_t dynamic_entries() const {
			typename Def::Dyn * dyn = reinterpret_cast<typename Def::Dyn *>(data());
			assert(entry_size() == sizeof(*dyn));
			size_t entries = 0;
			for (size_t limit = this->entries() - 1; entries < limit && dyn[entries].d_tag != Def::DT_NULL; entries++) {}
			return entries + 1;
		}
	};


	/*! \brief Header */
	const Header &header;

	/*! \brief Segment entries (from program header table) */
	Array<Segment> segments;

	/*! \brief Section entries (from section header table) */
	Array<Section> sections;

	/*! \brief Construct new ELF object
	 * \paran address Pointer to start adress of memory mapped file
	 */
	ELF(uintptr_t address)
	 :  header{*reinterpret_cast<Header*>(address)},
	    segments{Segment{*this}, data(header.e_phoff), header.e_phnum},
	    sections{Section{*this}, data(header.e_shoff), header.e_shnum} {
		assert(address != 0);
		assert(sizeof(Header) == header.e_ehsize);
		assert(sizeof(typename Def::Phdr) == header.e_phentsize);
		assert(sizeof(typename Def::Shdr) == header.e_shentsize);
	}

	/*! \brief Construct new ELF object
	 * \paran address Pointer to start adress of memory mapped file
	 */
	ELF(void * address)
	  : ELF{reinterpret_cast<uintptr_t>(address)} {};

#ifdef VIRTUAL
	/*! \brief Virtual destructor for polymorphic object
	 */
	virtual ~ELF() = default;
#endif

	/*! \brief get class of ELF object */
	static constexpr typename Def::ident_class elfclass() {
		return C;
	}

	/*! \brief Pointer to ELF data in memory
	 * \param displacement offset in ELF
	 * \return pointer to data
	 */
	inline void * data(uintptr_t displacement) const {
		return reinterpret_cast<void *>(start() + displacement);
	}

	/*! \brief Check if this file seems to be valid (using file size and offsets)
	 * \param file_size Length of memory mapped file
	 */
	bool valid(size_t file_size) const {
		if (file_size < sizeof(Header)
		 || !header.valid()
		 || file_size < header.e_ehsize
		 || file_size < header.e_phoff + header.e_phentsize * header.e_phnum
		 || file_size < header.e_shoff + header.e_shentsize * header.e_shnum)
			return false;

		for (const auto & section : sections)
			if (section.type() != Def::SHT_NOBITS && file_size < section.offset() + section.size())
				return false;

		for (const auto & segment : segments)
			if (file_size < segment.offset() + segment.size())
				return false;

		return true;
	}

	/*! \brief Access dynamic section
	 * \param mapped the elf file is mapped according to the segments
	 */
	DynamicTable dynamic(bool mapped = false) const {
		for (const auto &s : segments)
			if (s.type() == Def::PT_DYNAMIC)
				return s.get_dynamic_table(mapped);
		return DynamicTable{*this};
	}

	/*! \brief Interpreter (dynamic linker) */
	const char * interpreter() const {
		for (const auto &s : segments)
			if (s.type() == Def::PT_INTERP)
				return s.path();
		return nullptr;
	}

	/*! \brief Get symbol
	 * \param section symbol table section
	 * \param index index of symbol in table
	 * \return Symbol
	 */
	Symbol symbol(const Section & section, uint32_t index) const {
		assert(section.type() == Def::SHT_SYMTAB || section.type() == Def::SHT_DYNSYM);
		assert(section.entries() > index);
		const auto strtab = section.link();
		assert(strtab.type() == Def::SHT_STRTAB);
		return Symbol{*this, strtab, section.data(index * sizeof(typename Def::Sym))};
	}

	/*! \brief Get symbol
	 * \param section_index symbol table section index
	 * \param index index of symbol in table
	 * \return Symbol
	 */
	Symbol symbol(uint16_t section_index, uint32_t index) const {
		return symbol(sections[section_index], index);
	}

	/*! \brief Get string
	 * \param section_offset offset to string table section
	 * \param offset offset of string in table
	 * \return String
	 */
	const char * string(uintptr_t section_offset, uint32_t offset) const {
		return reinterpret_cast<const char *>(data(section_offset + offset));
	}

	/*! \brief Get string
	 * \param section string table section
	 * \param offset offset of string in table
	 * \return String
	 */
	const char * string(const Section & section, uint32_t offset) const {
		assert(section.type() == Def::SHT_STRTAB);
		return reinterpret_cast<const char *>(section.data(offset));
	}

	/*! \brief Get string
	 * \param section_index string table section index
	 * \param offset offset of string in table
	 * \return String
	 */
	const char * string(uint16_t section_index, uint32_t offset) const {
		return string(sections[section_index], offset);
	}

	/*! \brief Calculate size of Elf
	 * \param only_allocated limit to allocated data
	 * \return size
	 */
	size_t size(bool only_allocated = false) const {
		// Elf Header
		size_t size = header.e_ehsize;

		// Program Header Table
		size_t ph_size =  header.e_phoff + header.e_phnum * header.e_phentsize;
		if (ph_size > size)
			size = ph_size;

		// Segments (in Program Header Table)
		for (const auto & s : segments) {
			if (only_allocated && s.type() != Def::PT_LOAD)
				continue;
			size_t seg_size = s.offset() + s.size();
			if (seg_size > size)
				size = seg_size;
		}

		if (!only_allocated) {
			// Section Header Table
			// (usually this is located at the end)
			size_t sh_size =  header.e_shoff + header.e_shnum * header.e_shentsize;
			if (sh_size > size)
				size = sh_size;

			// Sections
			for (const auto & s : sections) {
				size_t sec_size = s.offset() + s.size();
				if (sec_size > size)
					size = sec_size;
			}
		}

		return size;
	}
};

// Declare types for fast access
namespace ELF_Def {

template<size_t B>
struct AddressWidth { typedef ELF<ELFCLASS::ELFCLASSNONE> type; };

template<>
struct AddressWidth<4> { typedef ELF<ELFCLASS::ELFCLASS32> type; };

template<>
struct AddressWidth<8> { typedef ELF<ELFCLASS::ELFCLASS64> type; };

} // ELF_Def

using Elf = typename ELF_Def::AddressWidth<sizeof(void*)>::type;
using Elf32 = ELF<ELFCLASS::ELFCLASS32>;
using Elf64 = ELF<ELFCLASS::ELFCLASS64>;
