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

		const D * ptr() const {
			return _data;
		}

		/*! \brief Absolute memory address of current element */
		uintptr_t address() const {
			return reinterpret_cast<uintptr_t>(_data);
		}
	};

	// Array (with random access) using accessor
	template <typename A>
	class Array {
		using V = decltype(A::_data);

	 public:
		const A accessor;
		V const values;
		const size_t entries;

		Array(const A & accessor, uintptr_t ptr, size_t entries) : accessor(accessor), values(reinterpret_cast<V const>(ptr)), entries(entries) {}

		A operator[](size_t idx) const {
			assert(idx < entries);
			A ret = accessor;
			ret._data = values + idx;
			return ret;
		}

		/*! \brief Absolute memory address of first element */
		uintptr_t address() const {
			return reinterpret_cast<uintptr_t>(values);
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

	// Sequential access
	template <typename A>
	class List {
		using V = decltype(A::_data);

	 public:
		const A _accessor;
		const V _begin, _end;

		List(const A & accessor, const V begin, const V end) : _accessor(accessor), _begin(begin), _end(end) {}

		A operator[](size_t idx) const {
			for (auto & entry : *this)
				if (idx-- == 0)
					return entry;
			assert(false);
			return _accessor;
		}
		/*! \brief Absolute memory address of first element */
		uintptr_t address() const {
			return reinterpret_cast<uintptr_t>(_begin);
		}

		size_t count() const {
			size_t entries = 0;
			for (auto & entry : *this)
				entries++;
			return entries;
		}

		bool empty() const {
			for (auto & entry : *this)
				return true;
			return false;
		}

		class iterator {
			A a;

		 public:
			iterator(const A & a, const V ptr) : a(a) {
				this->a._data = ptr;
			}

			iterator operator++() {
				assert(a._data != a.next());
				a._data = a.next();
				return *this;
			}

			bool operator!=(const iterator & o) const {
				return a._data != o.a._data;
			}

			const A operator*() const {
				return a;
			}
		};

		iterator begin() const {
			return iterator(_accessor, _begin);
		}

		iterator end() const {
			return iterator(_accessor, _end);
		}
	};

 public:
	const uintptr_t start;
	const size_t length;

	// Header
	struct Header : Def::Ehdr {
		/*! \brief Check if this elf identification header is valid for ELFO */
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

	struct SymbolTable : public Array<Symbol> {
		const ELF<C> & elf;
		const enum {
			NONE,
			HASH,
			GNUHASH
		} lookup_method;
		const void * header;
		const Section & symbol_section;
		const uint16_t * const versions;

		/*! \brief Symbol table
		 * similar to symbols array, but offering a symbol lookup
		 * (which can be quite fast when using hash)
		 */
		SymbolTable(const ELF<C> & elf, const Section & section, const Section * version = nullptr) : elf(elf),
		    lookup_method(section.type() == Def::SHT_GNUHASH ? GNUHASH : (section.type() == Def::SHT_HASH ? HASH : NONE)),
		    header(lookup_method == NONE ? nullptr : section.data()),
		    symbol_section(lookup_method == NONE ? section.get_symbols() : elf.sections[section.link()].get_symbols()),
		    versions(version == nullptr ? nullptr : reinterpret_cast<const uint16_t *>(version->data())),
		    Array<Symbol>(Symbol(elf, symbol_section.link()), elf.start + symbol_section.offset(), symbol_section.size() / symbol_section.entry_size()) {
				assert(version == nullptr || version->type() == Def::SHT_GNU_VERSYM);
			}

		/*! \brief Get symbol name by index */
		inline const char * name(uint32_t idx) {
			return idx == Def::STN_UNDEF ? nullptr : elf.string(symbol_section.strtab, this->values[idx].st_name);
		}

		/*! \brief Get symbol version index by symbol index
		 * \param idx symbol index
		 * \return Symbol version or VER_NDX_UNKNOWN if none
		 */
		inline const uint16_t version(uint32_t idx) {
			return versions == nullptr ? Def::VER_NDX_UNKNOWN : versions[idx];
		}

		/*! \brief Find symbol
		 * use hash if available
		 * \note Undefined symbols are usually excluded from hash hence they might not be found using this method!
		 * \param search_name symbol name to search
		 * \param required_version required version or VER_NDX_UNKNOWN if none
		 * \return index of object or STN_UNDEF
		 */
		uint32_t index(const char * search_name, uint32_t required_version = Def::VER_NDX_UNKNOWN) const {
			if (required_version != Def::VER_NDX_UNKNOWN && versions == nullptr)
				required_version = Def::VER_NDX_UNKNOWN;
			switch (lookup_method) {
				case HASH:
					return index_by_hash(search_name, version);
				case GNUHASH:
					return index_by_gnuhash(search_name, version);
				default:
					for (size_t i = 1; i < this->entries; i++)
						if (strcmp(search_name, name(i)) == 0 && (required_version == Def::VER_NDX_UNKNOWN || required_version == version(i)))
							return i;
			}
			return Def::STN_UNDEF;
		}

		/*! \brief Access symbol by char* index */
		inline Symbol operator[](const char * search_name) const {
			return this->operator[](index(search_name));  // 0 == UNDEF
		}

		/*! \brief Access symbol by string index */
		inline Symbol operator[](std::string search_name) const {
			return this->operator[](search_name.c_str());
		}

	 private:
		/*! \brief Find symbol index using ELF Hash
		 * \see https://flapenguin.me/elf-dt-hash
		 * \param search_name symbol name to search
		 * \return index of object or STN_UNDEF
		 */
		uint32_t index_by_hash(const char *search_name, uint32_t required_version) const {
			const ELF_Def::Hash_header * header = reinterpret_cast<ELF_Def::Hash_header*>(this->header);
			const uint32_t * bucket = reinterpret_cast<const uint32_t *>(header + 1);
			const uint32_t * chain = bucket + header->nbucket;

			const uint32_t h = Def::hash(search_name);

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
		uint32_t index_by_gnuhash(const char *search_name, uint32_t required_version) const {
			const ELF_Def::GnuHash_header * header = reinterpret_cast<ELF_Def::GnuHash_header*>(this->header);
			const elfptr_t * bloom = reinterpret_cast<const elfptr_t *>(header + 1);
			const uint32_t * buckets = reinterpret_cast<const uint32_t *>(bloom + header->bloom_size);
			const uint32_t * chain = buckets + header->nbuckets;

			uint32_t h1 = Def::gnuhash(search_name);
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


	/*! \brief Relocation entry (without addend) */
	struct Relocation : Accessor<typename Def::Rel> {
		const ELF<C> & elf;

		/*! \brief Corresponding symbol table index */
		const uint16_t symtab;

		Relocation(const ELF<C> & elf, uint16_t symtab = 0) : elf(elf), symtab(symtab) {}

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
			return elf.symbol(symtab, this->_data->r_info.sym);
		}

		/*! \brief Index of target symbol in corresponding symbol table */
		uint32_t symbol_index() const {
			return this->_data->r_info.sym;
		}

		/*! \brief relocation type (depending on architecture) */
		uint32_t type() const {
			return static_cast<uint32_t>(this->_data->r_info.type);
		}

		/*! \brief Addend (which is always zero for this relocation type) */
		uintptr_t addend() const {
			return 0;
		}
	};

	/*! \brief Relocation entry with addend */
	struct RelocationAddend : Accessor<typename Def::Rela> {
		const ELF<C> & elf;

		/*! \brief Corresponding symbol table index */
		const uint16_t symtab;

		RelocationAddend(const ELF<C> & elf, uint16_t symtab = 0) : elf(elf), symtab(symtab) {}

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
			return elf.symbol(symtab, this->_data->r_info.sym);
		}

		/*! \brief Index of target symbol in corresponding symbol table */
		uint32_t symbol_index() const {
			return this->_data->r_info.sym;
		}

		/*! \brief relocation type (depending on architecture) */
		uint32_t type() const {
			return static_cast<uint32_t>(this->_data->r_info.type);
		}

		/*! \brief Addend (which is always zero for this relocation type) */
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

	struct Note : Accessor<typename Def::Nhdr> {
		const ELF<C> & elf;

		Note(const ELF<C> & elf, uint16_t link = 0) : elf(elf) {
			assert(link == 0);
		}

		const char * name() const {
			return this->_data->n_namesz == 0 ? nullptr : (reinterpret_cast<const char *>(this->_data) + sizeof(typename Def::Nhdr));
		}

		const void * description() const {
			return this->_data->n_descsz == 0 ? nullptr : reinterpret_cast<const void *>(reinterpret_cast<uintptr_t>(this->_data) + sizeof(typename Def::Nhdr) + align(this->_data->n_namesz));
		}

		/*! \brief description size (in bytes!)
		 */
		size_t size() const {
			return this->_data->n_descsz;
		}

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

		template<typename T>
		static inline T align(T size) {
			return (size + 3)  & (~0x3);  // 4 byte alignment
		}
	};

	struct VersionDefinition : Accessor<typename Def::Verdef> {
		struct Auxiliary : Accessor<typename Def::Verdaux> {
			const ELF<C> & elf;
			const uint16_t strtab;

			Auxiliary(const ELF<C> & elf, uint16_t strtab) : elf(elf), strtab(strtab) {}

			/*! \brief Definition name */
			const char * name() const {
				return elf.string(strtab, this->_data->vda_name);
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
		const ELF<C> & elf;
		const uint16_t strtab;

		VersionDefinition(const ELF<C> & elf, uint16_t strtab = 0) : elf(elf), strtab(strtab) {}

		/*! \brief Version for this definition */
		uint16_t revision() const {
			return this->_data->vd_version;
		}

		uint16_t flags() const {
			return this->_data->vd_flags;
		}

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
			return List<Auxiliary>(Auxiliary(elf, strtab), first, nullptr);
		}

		/*! \brief Number of version definition auxiliary information */
		uint16_t auxiliaries() const {
			return this->_data->vd_cnt;
		}
	};

	struct VersionNeeded : Accessor<typename Def::Verneed> {
		struct Auxiliary : Accessor<typename Def::Vernaux> {
			const ELF<C> & elf;
			const uint16_t strtab;

			Auxiliary(const ELF<C> & elf, uint16_t strtab) : elf(elf), strtab(strtab) {}

			/*! \brief hash value of dependency name */
			uint32_t hash() const {
				return this->_data->vna_hash;
			}

			uint16_t flags() const {
				return this->_data->vna_flags;
			}

			bool weak() const {
				return this->_data->vna_weak == 1;
			}

			/*! \brief Version index (as used in version symbol table) */
			uint16_t version_index() const {
				return this->_data->vna_other;
			}

			/*! \brief Dependency name */
			const char * name() const {
				return elf.string(strtab, this->_data->vna_name);
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
		const ELF<C> & elf;
		const uint16_t strtab;

		VersionNeeded(const ELF<C> & elf, uint16_t strtab = 0) : elf(elf), strtab(strtab) {}

		/*! \brief Version for this dependency */
		typename Def::verneed_version version() const {
			return this->_data->vn_version;
		}

		/*! \brief filename for this dependency */
		const char * file() const {
			return elf.string(strtab, this->_data->vn_file);
		}

		/*! \brief List of version dependency auxiliary information */
		List<Auxiliary> auxiliary() const {
			uintptr_t first_adr = reinterpret_cast<uintptr_t>(this->_data) + this->_data->vn_aux;
			typename Def::Vernaux * first = this->_data->vn_aux == 0 ? nullptr : reinterpret_cast<typename Def::Vernaux *>(first_adr);
			return List<Auxiliary>(Auxiliary(elf, strtab), first, nullptr);
		}

		/*! \brief Number of version dependency auxiliary information */
		uint16_t auxiliaries() const {
			return this->_data->vn_cnt;
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
		uintptr_t virt_addr() const {
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

		List<Note> get_notes() const {
			assert(type() == Def::SHT_NOTE);
			return get_list<Note>();
		}

		List<VersionDefinition> get_version_definition() const {
			assert(type() == Def::SHT_GNU_VERDEF);
			return get_list<VersionDefinition>(true);
		}

		List<VersionNeeded> get_version_needed() const {
			assert(type() == Def::SHT_GNU_VERNEED);
			return get_list<VersionNeeded>(true);
		}

		Array<Symbol> get_symbols() const {
			assert(type() == Def::SHT_SYMTAB || type() == Def::SHT_DYNSYM);
			return get_array<Symbol>();
		}

		SymbolTable get_symbol_table(const Section * const version = nullptr) const {
			assert(type() == Def::SHT_SYMTAB || type() == Def::SHT_DYNSYM || type() == Def::SHT_HASH || type() == Def::SHT_GNUHASH);
			return SymbolTable(elf, *this, version);
		}

		Array<Dynamic> get_dynamic() const {
			assert(type() == Def::SHT_DYNAMIC);
			// Calculate length
			uintptr_t ptr = elf.start + offset();
			typename Def::Dyn * dyn = reinterpret_cast<typename Def::Dyn *>(ptr);
			assert(entry_size() == sizeof(*dyn));
			size_t entries = 0;
			for (; entries < (size() / entry_size()) - 1 && dyn[entries].d_tag != Def::DT_NULL; entries++) {}
			return Array<Dynamic>(Dynamic(elf, link()), ptr, entries + 1);
		}

		/*! \brief Array with elements
		 * List of elements with fixed size and known length, hence random access is possible
		 * \return Array object
		 */
		template<typename ACCESSOR>
		Array<ACCESSOR> get_array() const {
			assert(entry_size() == sizeof(*ACCESSOR::_data));
			return Array<ACCESSOR>(ACCESSOR(elf, link()), elf.start + offset(), size() / entry_size());
		}

		/*! \brief Sequential list with elements
		 * List of elements with arbitrary size, hence only sequential access is possible
		 * \note `count()` and []-access has O(n) costs
		 * \param last_is_nullptr if true, then last element is indicated by `next() == nullptr`
		 * \return List object
		 */
		template<typename ACCESSOR>
		List<ACCESSOR> get_list(bool last_is_nullptr = false) const {
			using V = decltype(ACCESSOR::_data);
			assert(entry_size() == 0);
			uintptr_t begin_adr = elf.start + offset();
			return List<ACCESSOR>(ACCESSOR(elf, link()), reinterpret_cast<const V>(begin_adr), last_is_nullptr ? nullptr : reinterpret_cast<const V>(begin_adr + size()));
		}
	};


	// Header
	const Header &header;

	Array<Segment> segments;

	Array<Section> sections;

	ELF(uintptr_t start, size_t length) : start(start), length(length),
	    header(*reinterpret_cast<Header*>(start)),
	    segments(Segment(*this), start + header.e_phoff, header.e_phnum),
	    sections(Section(*this), start + header.e_shoff, header.e_shnum) {
		assert(sizeof(Header) == header.e_ehsize);
		assert(sizeof(typename Def::Phdr) == header.e_phentsize);
		assert(sizeof(typename Def::Shdr) == header.e_shentsize);
	}

	/*! \brief Check if this file seems to be valid (using file size and offsets) */
	bool valid() {
		if (length < sizeof(Header)
		 || !header.valid()
		 || length < header.size()
		 || length < header.e_phoff + header.e_phentsize * header.e_phnum
		 || length < header.e_shoff + header.e_shentsize * header.e_shnum)
			return false;

		for (auto & section : sections)
			if (length < section.offset() + section.size())
				return false;

		for (auto & segment : segments)
			if (length < segment.offset() + segment.size())
				return false;

		return true;
	}

	template<typename ACCESSOR>
	Array<ACCESSOR> get(const ACCESSOR & t, uintptr_t offset, size_t size) const {
		return Array<ACCESSOR>(t, offset, size);
	}

	template<typename ACCESSOR>
	Array<ACCESSOR> get() const {
		return Array<ACCESSOR>(ACCESSOR(*this), 0, 0);
	}

	Section section_by_offset(uintptr_t offset) const {
		for (auto &s : sections)
			if (s.offset() == offset)
				return s;
		return sections[0];
	}

	Symbol symbol(const Section & section, uint32_t index) const {
		return section.get_symbols()[index];
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
	 * \return Symbol Table
	 */
	SymbolTable dynamic_symbol_table() const {
		uintptr_t offsets[3] = { 0 };

		for (auto &dyn: dynamic()) {
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
			}
		}

		for (auto & offset : offsets)
			if (offset != 0)
				return SymbolTable(*this, section_by_offset(offset));

		return nullptr;
	}
};

using ELF32 = ELF<ELFCLASS::ELFCLASS32>;
using ELF64 = ELF<ELFCLASS::ELFCLASS64>;
