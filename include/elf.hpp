		#pragma once

#include <cassert>
#include <string>
#include <utility>

#include "elf_def/ident.hpp"
#include "elf_def/types.hpp"
#include "elf_def/const.hpp"
#include "elf_def/struct.hpp"
#include "elf_def/hash.hpp"


/*! \brief Parser for data in the Executable and Linking Format
 * \tparam C 32- or 64-bit elf class
 */
template<ELFCLASS C>
struct ELF : public ELF_Def::Structures<C> {
	using Def = typename ELF_Def::Structures<C>;
	using elfptr_t = typename Def::Elf_Addr;

	/*! \brief Start address of ELF in memory */
	const uintptr_t start;

	/*! \brief Accessor to wrap elements of a given data type
	 * \tparam DT data type of element
	 */
	template <typename DT>
	struct Accessor {
		/*! \brief Parent object */
		const ELF<C> & _elf;

		/*! \brief Pointer to payload */
		const DT * _data = nullptr;

		/*! \brief Constructor */
		explicit Accessor(const ELF<C> & elf) : _elf(elf) {}

		/*! \brief Copy-constructor */
		explicit Accessor(const Accessor<DT> & accessor) : _elf(accessor._elf), _data(accessor._data) {}

		/*! \brief Elf object */
		const ELF<C> & elf() const {
			return _elf;
		}

		/*! \brief Retrieve payload */
		const DT * ptr() const {
			return _data;
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
	};

	/*! \brief Array-like access to data with fixed element size using accessor
	 * \tparam A class
	 */
	template <typename A>
	class Array {
	protected:
		/*! \brief Data type of element */
		using V = decltype(A::_data);

		/*! \brief Accessor template for each element */
		const A _accessor;

		/*! \brief Pointer to first element */
		V const _values;

		/*! \brief Pointer to first element */
		const size_t _entries;

	 public:
		/*! \brief Construct new Array access
		 * \param accessor \ref Accessor template
		 * \param ptr Pointer to first element
		 * \param entries number of elements
		 */
		Array(const A & accessor, uintptr_t ptr, size_t entries) : _accessor(accessor), _values(reinterpret_cast<V const>(ptr)), _entries(entries) {}

		/*! \brief Array-like access
		 * \param idx index
		 * \return Accessor for element
		 */
		A operator[](size_t idx) const {
			assert(idx < _entries);
			A ret = _accessor;
			ret._data = _values + idx;
			return ret;
		}

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
			return reinterpret_cast<uintptr_t>(_values);
		}

		/*! \brief Number of elements in array
		 */
		size_t count() const {
			return _entries;
		}

		/*! \brief Are there any elements in the array?
		 * \return `true` if there is at least one element
		 */
		bool empty() const {
			return _entries == 0;
		}

		/*! \brief Get index of element
		 * \param element Element in array to get index of
		 * \note you have to ensure that this element exists in the array
		 * \return index
		 */
		size_t index(const A & element) const {
			return (reinterpret_cast<size_t>(element._data) - reinterpret_cast<size_t>(_values)) / sizeof(*element._data);
		}

		/*! \brief Iterator */
		class Iterator {
			/*! \brief Pointer to current element */
			V p;

			/*! \brief Accessor Template for elements */
			const A & a;

		 public:
			/*! \brief Iterator constructor
			 * \param p pointer to current element
			 * \param a accessor template
			 */
			Iterator(const V & p, const A & a): p(p), a(a){}

			/*! \brief Next element */
			Iterator operator++() {
				++p;
				return *this;
			}

			/*! \brief Compare current iterator element */
			bool operator!=(const Iterator & o) const {
				return p != o.p;
			}

			/*! \brief Get current element */
			const A operator*() const {
				A ret = a;
				ret._data = p;
				return ret;
			}
		};

		/*! \brief Get Iterator for first element */
		Iterator begin() const {
			return Iterator(_values, _accessor);
		}

		/*! \brief Get Iterator for last element */
		Iterator end() const {
			return Iterator(_values + _entries, _accessor);
		}
	};

	/*! \brief Sequential access (single linked-list style) to data with variable element size using accessor
	 * \tparam A class
	 */
	template <typename A>
	class List {
		/*! \brief Data type of element */
		using V = decltype(A::_data);

		/*! \brief Accessor template for each element */
		const A _accessor;

		/*! \brief Pointer to first element of the list*/
		const V _begin;

		/*! \brief Pointer to the indicator of the end of the list
		 * \note `nullptr` can be valid
		 */
		const V _end;

	 public:
		/*! \brief Construct new List access
		 * \param accessor \ref Accessor template
		 * \param begin Pointer to first element
		 * \param end Indicator for end of list
		 */
		List(const A & accessor, const V begin, const V end) : _accessor(accessor), _begin(begin), _end(end) {}

		/*! \brief Array-like access
		 * \note O(n) complexity!
		 * \param idx index
		 * \return Accessor for element
		 */
		A operator[](size_t idx) const {
			for (auto & entry : *this)
				if (idx-- == 0)
					return entry;
			assert(false);
			return _accessor;
		}

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
			return reinterpret_cast<uintptr_t>(_begin);
		}

		/*! \brief Number of elements in array
		 * \note O(n) complexity!
		 */
		size_t count() const {
			size_t entries = 0;
			for (auto & entry : *this)
				entries++;
			return entries;
		}

		/*! \brief Are there any elements in the array?
		 * \return `true` if there is at least one element
		 */
		bool empty() const {
			for (auto & entry : *this)
				return true;
			return false;
		}

		/*! \brief Iterator */
		class Iterator {
			/*! \brief Accessor Template for elements */
			A a;

		 public:
			/*! \brief Iterator constructor
			 * \param p pointer to current element
			 * \param a accessor template
			 */
			Iterator(const A & a, const V ptr) : a(a) {
				this->a._data = ptr;
			}

			/*! \brief Next element
			 * \note Element must provide a `next` method!
			 */
			Iterator operator++() {
				assert(a._data != a.next());
				a._data = a.next();
				return *this;
			}

			/*! \brief Compare current iterator element */
			bool operator!=(const Iterator & o) const {
				return a._data != o.a._data;
			}

			/*! \brief Get current element */
			const A operator*() const {
				return a;
			}
		};

		/*! \brief Get Iterator for first element */
		Iterator begin() const {
			return Iterator(_accessor, _begin);
		}

		/*! \brief Get Iterator identicating end of list */
		Iterator end() const {
			return Iterator(_accessor, _end);
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

		/*! \brief Object file version */
		uintptr_t entry() const {
			return this->e_entry;
		}

		/*! \brief Object file version */
		uint32_t flags() const {
			return this->e_flags;
		}
	};

	// Segments (Program header table)
	struct Segment : Accessor<typename Def::Phdr> {
		/*! \brief Constructor for new Segment entry */
		explicit Segment(const ELF<C> & elf) : Accessor<typename Def::Phdr>(elf) {}

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
			return reinterpret_cast<void*>(this->_elf.start + this->_data->p_offset);
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
			return type() == Def::PT_INTERP ? reinterpret_cast<const char *>(this->_elf.start + this->_data->p_offset) : nullptr;
		}
	};

	struct Section;

	/*! \brief Symbol */
	struct Symbol : Accessor<typename Def::Sym> {
		/*! \brief Index of string table */
		const uint16_t strtab;

		/*! \brief Construct symbol
		 * \param elf ELF object to which this symbol belongs to
		 * \param strtab String table index associated with the symbol table for this symbol
		 */
		explicit Symbol(const ELF<C> & elf, uint16_t strtab = 0) : Accessor<typename Def::Sym>(elf), strtab(strtab) {}

		/*! \brief Is the symbol valid? */
		bool valid() const {
			return strtab != 0 && (value() != 0 || size() != 0 || info() != 0 || other() != 0);
		}

		/*! \brief Symbol name */
		const char * name() const {
			return this->_elf.string(strtab, this->_data->st_name);
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
		const ELF<C> & _elf;
		const typename Def::shdr_type section_type;
		const void * header;
		const uint16_t * const versions;
		using Array<Symbol>::operator[];
		using Array<Symbol>::index;

		/*! \brief Symbol table
		 * similar to symbols array, but offering a symbol lookup
		 * (which can be quite fast when using hash)
		 */
		SymbolTable(const ELF<C> & elf, const Section & section) : SymbolTable(elf, section, elf.sections[0]) {}

		/*! \brief Symbol table with version information
		 */
		SymbolTable(const ELF<C> & elf, const Section & section, const Section & version_section) :
		    SymbolTable(elf, (section.type() == Def::SHT_GNU_HASH) || (section.type() == Def::SHT_HASH), section, version_section) {}

		/*! \brief Empty (non-existing) symbol table
		 */
		explicit SymbolTable(const ELF<C> & elf) : Array<Symbol>(Symbol(elf, 0), 0, 0), _elf(elf), section_type(Def::SHT_NULL), header(nullptr), versions(nullptr) {}

		/*! \brief Elf object */
		const ELF<C> & elf() const {
			return _elf;
		}

		/*! \brief Get symbol name by index */
		inline const char * name(uint32_t idx) const {
			return idx == Def::STN_UNDEF ? nullptr : _elf.string(this->_accessor.strtab, this->_values[idx].st_name);
		}

		/*! \brief Get symbol version index by symbol index
		 * \param idx symbol index
		 * \return Symbol version or VER_NDX_UNKNOWN if none
		 */
		inline const uint16_t version(uint32_t idx) const{
			return versions == nullptr ? Def::VER_NDX_UNKNOWN : versions[idx];
		}

		/*! \brief Find symbol
		 * use hash if available
		 * \note Undefined symbols are usually excluded from hash hence they might not be found using this method!
		 * \param search_name symbol name to search
		 * \param required_version required version or VER_NDX_UNKNOWN if none
		 * \return index of object or STN_UNDEF
		 */
		size_t index(const char * search_name, uint16_t required_version = Def::VER_NDX_UNKNOWN) const {
			if (required_version != Def::VER_NDX_UNKNOWN && versions == nullptr)
				required_version = Def::VER_NDX_UNKNOWN;
			switch (section_type) {
				case Def::SHT_HASH:
					return index_by_hash(search_name, required_version);
				case Def::SHT_GNU_HASH:
					return index_by_gnuhash(search_name, required_version);
				case Def::SHT_DYNSYM:
				case Def::SHT_SYMTAB:
					for (size_t i = 1; i < this->_entries; i++)
						if (this->strcmp(search_name, name(i)) == 0 && (required_version == Def::VER_NDX_UNKNOWN || required_version == version(i)))
							return i;
				default:
					return Def::STN_UNDEF;
			}
		}

		/*! \brief Find symbol
		 * use hash if available
		 * \note Undefined symbols are usually excluded from hash hence they might not be found using this method!
		 * \param search_name symbol name to search
		 * \param required_version required version or VER_NDX_UNKNOWN if none
		 * \return index of object or STN_UNDEF
		 */
		size_t index(const std::string & search_name, uint16_t required_version = Def::VER_NDX_UNKNOWN) const {
			return index(search_name.c_str(), required_version);
		}

		/*! \brief Access symbol by char* index */
		inline Symbol operator[](const char * search_name) const {
			return operator[](index(search_name));  // 0 == UNDEF
		}

		/*! \brief Access symbol by string index */
		inline Symbol operator[](const std::string & search_name) const {
			return operator[](search_name.c_str());
		}

	 private:
		SymbolTable(const ELF<C> & elf, bool use_hash, const Section & section, const Section & version_section) :
		    SymbolTable(elf, section.type(), use_hash ? section.data() : nullptr, use_hash ? elf.sections[section.link()] : section, version_section) {}

		SymbolTable(const ELF<C> & elf, const typename Def::shdr_type section_type, void * header, const Section & symbol_section, const Section & version_section) :
		    Array<Symbol>(Symbol(elf, symbol_section.link()), elf.start + symbol_section.offset(), symbol_section.size() / symbol_section.entry_size()),
		    _elf(elf), section_type(section_type), header(header), versions(version_section.type() == Def::SHT_GNU_VERSYM ? version_section.get_versions() : nullptr) {
			assert(section_type == Def::SHT_GNU_HASH || section_type == Def::SHT_HASH || section_type == Def::SHT_DYNSYM || section_type == Def::SHT_SYMTAB);
			assert(section_type == Def::SHT_DYNSYM || section_type == Def::SHT_SYMTAB || header != nullptr);
		}

		/*! \brief Find symbol index using ELF Hash
		 * \see https://flapenguin.me/elf-dt-hash
		 * \param search_name symbol name to search
		 * \return index of object or STN_UNDEF
		 */
		uint32_t index_by_hash(const char *search_name, uint16_t required_version) const {
			const ELF_Def::Hash_header * header = reinterpret_cast<const ELF_Def::Hash_header*>(this->header);
			const uint32_t * bucket = reinterpret_cast<const uint32_t *>(header + 1);
			const uint32_t * chain = bucket + header->nbucket;

			const uint32_t h = ELF_Def::hash(search_name);

			for (uint32_t i = bucket[h % (header->nbucket)]; i; i = chain[i])
				if (!SymbolTable::strcmp(search_name, SymbolTable::name(i)) && (required_version == Def::VER_NDX_UNKNOWN || required_version == version(i)))
					return i;

			return Def::STN_UNDEF;
		}

		/*! \brief Find symbol index using ELF Hash
		 * \see https://blogs.oracle.com/solaris/gnu-hash-elf-sections-v2
		 * \see https://flapenguin.me/elf-dt-gnu-hash
		 * \param search_name symbol name to search
		 * \return index of object or STN_UNDEF
		 */
		uint32_t index_by_gnuhash(const char *search_name, uint16_t required_version) const {
			const ELF_Def::GnuHash_header * header = reinterpret_cast<const ELF_Def::GnuHash_header*>(this->header);
			const elfptr_t * bloom = reinterpret_cast<const elfptr_t *>(header + 1);
			const uint32_t * buckets = reinterpret_cast<const uint32_t *>(bloom + header->bloom_size);
			const uint32_t * chain = buckets + header->nbuckets;

			uint32_t h1 = ELF_Def::gnuhash(search_name);
			const uint32_t c = sizeof(elfptr_t) * 8;
			const elfptr_t one = 1;
			const elfptr_t mask = (one << (h1 % c))
			                    | (one << ((h1 >> header->bloom_shift) % c));

			elfptr_t n = (h1 / c) % header->bloom_size;
			if ((bloom[n] & mask) != mask)
				return Def::STN_UNDEF;

			n = buckets[h1 % header->nbuckets];
			if (n == 0)
				return Def::STN_UNDEF;

			const uint32_t * hashval = chain + (n - header->symoffset);

			for (h1 &= ~1; true; n++) {
				uint32_t h2 = *hashval++;
				if ((h1 == (h2 & ~1)) && !SymbolTable::strcmp(search_name, SymbolTable::name(n)) && (required_version == Def::VER_NDX_UNKNOWN || required_version == version(n)))
					return n;
				if (h2 & 1)
					break;
			}
			return Def::STN_UNDEF;
		}

		static inline int strcmp(const char *s1, const char *s2) {
			if (s1 == nullptr || s2 == nullptr)
				return 0;
			while (*s1 == *s2++)
				if (*s1++ == '\0')
					return 0;
			return static_cast<int>(*s1) - static_cast<int>(*(s2-1));
		}
	};

	/*! \brief Relocation entry without addend */
	struct Relocation : Accessor<typename Def::Rel> {
		/*! \brief Corresponding symbol table index */
		const uint16_t symtab;

		/*! \brief Construct relocation entry (without addend)
		 * \param elf ELF object to which this relocation belongs to
		 * \param symtab Symbol table index for this relocation
		 */
		explicit Relocation(const ELF<C> & elf, uint16_t symtab = 0) : Accessor<typename Def::Rel>(elf), symtab(symtab) {}

		/*! \brief Valid relocation */
		bool valid() const {
			return symtab != 0;
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
			return this->_elf.symbol(symtab, this->_data->r_info.sym);
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
		/*! \brief Corresponding symbol table index */
		const uint16_t symtab;

		/*! \brief Construct relocation entry (with addend)
		 * \param elf ELF object to which this relocation belongs to
		 * \param symtab Symbol table index for this relocation
		 */
		explicit RelocationWithAddend(const ELF<C> & elf, uint16_t symtab = 0) : Accessor<typename Def::Rela>(elf), symtab(symtab) {}

		/*! \brief Valid relocation */
		bool valid() const {
			return symtab != 0;
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
			return this->_elf.symbol(symtab, this->_data->r_info.sym);
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

	/*! \brief Dynamic table entry */
	struct Dynamic : Accessor<typename Def::Dyn> {
		/*! \brief Index of string table */
		const uint16_t strtab;

		/*! \brief construct dynamic table entry
		 * \param elf ELF object to which this symbol belongs to
		 * \brief strtab Index of string table
		 */
		explicit Dynamic(const ELF<C> & elf, uint16_t strtab = 0) : Accessor<typename Def::Dyn>(elf), strtab(strtab) {}

		/*! \brief Valid dynamic table? */
		bool valid() const {
			return strtab != 0;
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
			return this->_elf.string(strtab, this->_data->d_un.d_val);
		}
	};

	/*! \brief GNU Note entry */
	struct Note : Accessor<typename Def::Nhdr> {
		/*! \brief Construct note entry
		 * \param elf ELF object to which this note belongs to
		 * \param link Associated section index (must be `0`)
		 */
		explicit Note(const ELF<C> & elf, uint16_t link = 0) : Accessor<typename Def::Nhdr>(elf) {
			assert(link == 0);
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

		/*! \brief Next element
		 * \return pointer to next element
		 */
		typename Def::Nhdr * next() const {
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
			/*! \brief String table index */
			const uint16_t strtab;

			/*! \brief Construct auxiliary entry for version definition
			 * \param elf ELF object to which this entry belongs to
			 * \param strtab String table index for this erntry
			 */
			Auxiliary(const ELF<C> & elf, uint16_t strtab) : Accessor<typename Def::Verdaux>(elf), strtab(strtab) {}

			/*! \brief Definition name */
			const char * name() const {
				return this->_elf.string(strtab, this->_data->vda_name);
			}

		 private:
			friend class List<VersionDefinition::Auxiliary>;

			/*! \brief Next element
			 * \return pointer to next element or `nullptr` if end
			 */
			typename Def::Verdaux * next() const {
				return this->_data->vda_next == 0 ? nullptr : reinterpret_cast<typename Def::Verdaux*>(reinterpret_cast<uintptr_t>(this->_data) + this->_data->vda_next);
			}
		};

		friend class List<VersionDefinition>;

		/*! \brief Next element
		 * \return pointer to next element or `nullptr` if end
		 */
		typename Def::Verdef * next() const {
			uintptr_t next_adr = reinterpret_cast<uintptr_t>(this->_data) + this->_data->vd_next;
			return this->_data->vd_next == 0 ? nullptr : reinterpret_cast<typename Def::Verdef*>(next_adr);
		}

	public:
		/*! \brief String table index */
		const uint16_t strtab;

		/*! \brief Construct version definition entry
		 * \param elf ELF object to which this entry belongs to
		 * \param strtab String table index for this erntry
		 */
		explicit VersionDefinition(const ELF<C> & elf, uint16_t strtab = 0) : Accessor<typename Def::Verdef>(elf), strtab(strtab) {}

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
			return List<Auxiliary>(Auxiliary(this->_elf, strtab), first, nullptr);
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
			/*! \brief String table index */
			const uint16_t strtab;

			/*! \brief Construct auxiliary entry for version definition
			 * \param elf ELF object to which this entry belongs to
			 * \param strtab String table index for this erntry
			 */
			Auxiliary(const ELF<C> & elf, uint16_t strtab) : Accessor<typename Def::Vernaux>(elf), strtab(strtab) {}

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
				return this->_elf.string(strtab, this->_data->vna_name);
			}

		 private:
			friend class List<VersionNeeded::Auxiliary>;

			/*! \brief Next element
			 * \return pointer to next element or `nullptr` if end
			 */
			typename Def::Vernaux * next() const {
				return this->_data->vna_next == 0 ? nullptr : reinterpret_cast<typename Def::Vernaux*>(reinterpret_cast<uintptr_t>(this->_data) + this->_data->vna_next);
			}
		};

		friend class List<VersionNeeded>;

		/*! \brief Next element
		 * \return pointer to next element or `nullptr` if end
		 */
		typename Def::Verneed * next() const {
			uintptr_t next_adr = reinterpret_cast<uintptr_t>(this->_data) + this->_data->vn_next;
			return this->_data->vn_next == 0 ? nullptr : reinterpret_cast<typename Def::Verneed*>(next_adr);
		}

	public:
		/*! \brief String table index */
		const uint16_t strtab;

		/*! \brief Construct version needed entry
		 * \param elf ELF object to which this entry belongs to
		 * \param strtab String table index for this erntry
		 */
		explicit VersionNeeded(const ELF<C> & elf, uint16_t strtab = 0) : Accessor<typename Def::Verneed>(elf), strtab(strtab) {}

		/*! \brief Version for this dependency */
		typename Def::verneed_version version() const {
			return this->_data->vn_version;
		}

		/*! \brief filename for this dependency */
		const char * file() const {
			return this->_elf.string(strtab, this->_data->vn_file);
		}

		/*! \brief List of version dependency auxiliary information */
		List<Auxiliary> auxiliary() const {
			uintptr_t first_adr = reinterpret_cast<uintptr_t>(this->_data) + this->_data->vn_aux;
			typename Def::Vernaux * first = this->_data->vn_aux == 0 ? nullptr : reinterpret_cast<typename Def::Vernaux *>(first_adr);
			return List<Auxiliary>(Auxiliary(this->_elf, strtab), first, nullptr);
		}

		/*! \brief Number of version dependency auxiliary information */
		uint16_t auxiliaries() const {
			return this->_data->vn_cnt;
		}
	};

	/*! \brief Entry of section header table */
	struct Section : Accessor<typename Def::Shdr> {
		/*! \brief Construct new section header entry
		 * \param elf ELF object to which this entry belongs to
		 */
		explicit Section(const ELF<C> & elf) : Accessor<typename Def::Shdr>(elf) {}

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

		/*! \brief Pointer to dection contents */
		void * data() const {
			return reinterpret_cast<void*>(this->_elf.start + this->_data->sh_offset);
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
		uint32_t link() const {
			return this->_data->sh_link;
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
			return SymbolTable(this->_elf, *this, version);
		}

		/*! \brief Get contents of dynamic secion */
		Array<Dynamic> get_dynamic() const {
			assert(type() == Def::SHT_DYNAMIC);
			// Calculate length
			uintptr_t ptr = this->_elf.start + offset();
			typename Def::Dyn * dyn = reinterpret_cast<typename Def::Dyn *>(ptr);
			assert(entry_size() == sizeof(*dyn));
			size_t limit = this->entries() - 1;
			size_t entries = 0;
			for (; entries < limit && dyn[entries].d_tag != Def::DT_NULL; entries++) {}
			return Array<Dynamic>(Dynamic(this->_elf, link()), ptr, entries + 1);
		}

		/*! \brief Array with elements
		 * List of elements with fixed size and known length, hence random access is possible
		 * \return Array object
		 */
		template<typename ACCESSOR>
		Array<ACCESSOR> get_array() const {
			if (type() == Def::SHT_NULL) {
				return Array<ACCESSOR>(ACCESSOR(this->_elf, link()), 0, 0);
			} else {
				assert(entry_size() == sizeof(*ACCESSOR::_data));
				return Array<ACCESSOR>(ACCESSOR(this->_elf, link()), this->_elf.start + offset(), entries());
			}
		}

		/*! \brief Sequential list with elements
		 * List of elements with arbitrary size, hence only sequential access is possible
		 * \note `count()` and []-access has O(n) costs
		 * \param last_is_nullptr if true, then last element is indicated by `next() == nullptr`
		 * \return List object
		 */
		template<typename ACCESSOR>
		List<ACCESSOR> get_list(bool last_is_nullptr = false) const {
			if (type() == Def::SHT_NULL) {
				return List<ACCESSOR>(ACCESSOR(this->_elf, 0), nullptr, nullptr);
			} else {
				using V = decltype(ACCESSOR::_data);
				assert(entry_size() == 0);
				uintptr_t begin_adr = this->_elf.start + offset();
				return List<ACCESSOR>(ACCESSOR(this->_elf, link()), reinterpret_cast<const V>(begin_adr), last_is_nullptr ? nullptr : reinterpret_cast<const V>(begin_adr + size()));
			}
		}
	};


	/*! \brief Header */
	const Header &header;

	/*! \brief Segment entries (from program header table) */
	Array<Segment> segments;

	/*! \brief Section entries (from section header table) */
	Array<Section> sections;

	/*! \brief Construct new ELF object
	 * \paran start Pointer to start adress of memory mapped file
	 */
	ELF(uintptr_t start) : start(start),
	    header(*reinterpret_cast<Header*>(start)),
	    segments(Segment(*this), start + header.e_phoff, header.e_phnum),
	    sections(Section(*this), start + header.e_shoff, header.e_shnum) {
		assert(sizeof(Header) == header.e_ehsize);
		assert(sizeof(typename Def::Phdr) == header.e_phentsize);
		assert(sizeof(typename Def::Shdr) == header.e_shentsize);
	}

	/*! \brief get class of ELF object */
/*	static typename Def::ident_class class() {
		return C;
	}
*/
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

		for (auto & section : sections)
			if (section.type() != Def::SHT_NOBITS && file_size < section.offset() + section.size())
				return false;

		for (auto & segment : segments)
			if (file_size < segment.offset() + segment.size())
				return false;

		return true;
	}

 //protected:
	template<typename ACCESSOR>
	Array<ACCESSOR> get(const ACCESSOR & t, uintptr_t offset, size_t size) const {
		return Array<ACCESSOR>(t, offset, size);
	}

	template<typename ACCESSOR>
	Array<ACCESSOR> get() const {
		return Array<ACCESSOR>(ACCESSOR(*this), 0, 0);
	}

	/*! \brief Get section by offset
	 * \note required by \ref Dynamic
	 * \param offset relative address of start of section
	 * \return Section or pointer to NULL section if not found
	 */
	const Section section_by_offset(uintptr_t offset) const {
		for (auto &s : sections)
			if (s.offset() == offset)
				return s;
			else if (s.offset() > offset)
				break;
		return sections[0];
	}

	/*! \brief Get symbol
	 * \param section symbol table section
	 * \param index index of symbol in table
	 */
	Symbol symbol(const Section & section, uint32_t index) const {
		return section.get_symbols()[index];
	}

	/*! \brief Get symbol
	 * \param section_index symbol table section index
	 * \param index index of symbol in table
	 */
	Symbol symbol(uint16_t section_index, uint32_t index) const {
		return symbol(sections[section_index], index);
	}

	/*! \brief Get string
	 * \param section string table section
	 * \param offset offset of string in table
	 */
	const char * string(const Section & section, uint32_t offset) const {
		assert(section.type() == Def::SHT_STRTAB);
		return reinterpret_cast<const char *>(start + section.offset() + offset);
	}

	/*! \brief Get string
	 * \param section_index string table section index
	 * \param offset offset of string in table
	 */
	const char * string(uint16_t section_index, uint32_t offset) const {
		return string(sections[section_index], offset);
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
