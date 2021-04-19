#pragma once

#include <cstdint>

#include "const.hpp"
#include "ident.hpp"
#include "types.hpp"

namespace ELF_Def {

template<ELFCLASS C>
struct Structures : public Identification, public Constants, public Types<C> {
	using elfptr_t = typename Structures::Elf_Addr;
	using elfrel_t = typename Structures::Elf_Rel;
	using elfoff_t = typename Structures::Elf_Off;

	/*! \brief ELF file header */
	struct Ehdr {
		union {
			unsigned char e_ident[16];
			Identification identification;
		};
		ehdr_type e_type;
		ehdr_machine e_machine;
		ehdr_version e_version;
		elfptr_t e_entry;
		elfoff_t e_phoff;
		elfoff_t e_shoff;
		uint32_t e_flags;
		uint16_t e_ehsize;
		uint16_t e_phentsize;
		uint16_t e_phnum;
		uint16_t e_shentsize;
		uint16_t e_shnum;
		uint16_t e_shstrndx;
	} __attribute__((packed));


	/*! \brief Section header */
	union Shdr_flags {
		elfptr_t value;
		struct {
			elfptr_t write            : 1;
			elfptr_t alloc            : 1;
			elfptr_t execinstr        : 1;
			elfptr_t                  : 1;
			elfptr_t merge            : 1;
			elfptr_t strings          : 1;
			elfptr_t info_link        : 1;
			elfptr_t link_order       : 1;
			elfptr_t os_nonconforming : 1;
			elfptr_t group            : 1;
			elfptr_t tls              : 1;
			elfptr_t compressed       : 1;
			elfptr_t                  : 8;
			elfptr_t os               : 8;
			elfptr_t proc             : 4;
		} __attribute__((packed));
	};

	struct Shdr {
		uint32_t sh_name;
		shdr_type sh_type;
		Shdr_flags sh_flags;
		elfptr_t sh_addr;
		elfoff_t sh_offset;
		elfptr_t sh_size;
		uint32_t sh_link;
		uint32_t sh_info;
		elfptr_t sh_addralign;
		elfptr_t sh_entsize;
	} __attribute__((packed));


	/*! \brief Section compression header */
	struct Chdr;

	/*! \brief Segment header */
	union Phdr_flags {
		uint32_t value;
		struct {
			uint32_t x       :  1;
			uint32_t w       :  1;
			uint32_t r       :  1;
			uint32_t         : 17;
			uint32_t os      :  8;
			uint32_t proc    :  4;
		} __attribute__((packed));
	};

	struct Phdr;


	/*! \brief Symbol table*/
	struct Sym;

	union Sym_info {
		unsigned char value;
		struct {
			sym_type type : 4;
			sym_bind bind : 4;
		} __attribute__((packed));
	};

	union Sym_other {
		unsigned char value;
		struct {
			sym_visibility visibility : 2;
			unsigned char             : 0;
		} __attribute__((packed));
	};


	/*! \brief Relocation */
	union Rel_info;

	struct Rel {
		elfptr_t r_offset;
		Rel_info r_info;
	} __attribute__((packed));


	struct Rela {
		elfptr_t r_offset;
		Rel_info r_info;
		elfrel_t r_addend;
	} __attribute__((packed));


	/*! \brief Dyanamic */
	struct Dyn {
		elfrel_t d_tag;
		union {
			elfptr_t d_val;
			elfptr_t d_ptr;
		} d_un;
	} __attribute__((packed));


	/*! \brief Note section */
	struct Nhdr {
		uint32_t n_namesz;  ///< Length of the note's name.
		uint32_t n_descsz;  ///< Length of the note's descriptor
		nhdr_type n_type;   ///< Type of the note.
	};


	/*! \brief Version definition */
	struct Verdef {
		uint16_t vd_version;     ///< Version revision
		union {
			uint16_t vd_flags;   ///< Version information flags
			struct {
				uint16_t vd_base : 1;  ///< Version definition of the file itself.
				uint16_t vd_weak : 1;  ///< Weak version identifier.
				uint16_t         : 0;
			} __attribute__((packed));
		};
		uint16_t vd_ndx;         ///< Version Index (in versym section)
		uint16_t vd_cnt;         ///< Number of associated aux entries
		uint32_t vd_hash;        ///< Version name hash value
		uint32_t vd_aux;         ///< Offset in bytes to verdaux array
		uint32_t vd_next;        ///< Offset in bytes to next verdef entry
	} __attribute__((packed));

	/*! \brief Auxialiary version information. */
	struct Verdaux {
	  uint32_t vda_name;   ///< Version or dependency names
	  uint32_t vda_next;   ///< Offset in bytes to next verdaux entry
	} __attribute__((packed));


	/*! \brief Version dependency */
	struct Verneed {
		verneed_version vn_version;  ///< Version of structure
		uint16_t vn_cnt;             ///< Number of associated aux entries
		uint32_t vn_file;            ///< Offset of filename for this dependency
		uint32_t vn_aux;             ///< Offset in bytes to vernaux array
		uint32_t vn_next;            ///< Offset in bytes to next verneed entry
	};

	/*! \brief Auxiliary needed version information */
	struct Vernaux {
		uint32_t vna_hash;          ///< Hash value of dependency name */
		union {
			uint16_t vna_flags;         ///< Dependency specific information */
			struct {
				uint16_t          : 1;
				uint16_t vna_weak : 1;
				uint16_t          : 0;
			} __attribute__((packed));
		};
		uint16_t vna_other;         ///< Unused
		uint32_t vna_name;          ///< Dependency name string offset
		uint32_t vna_next;          ///< Offset in bytes to next vernaux entry
	};
};


template<>
struct Structures<ELFCLASS::ELFCLASS32>::Chdr {
	chdr_type ch_type;      ///< Compression format
	uint32_t ch_size;       ///< Uncompressed data size
	uint32_t ch_addralign;  ///< Uncompressed data alignment
} __attribute__((packed));

template<>
struct Structures<ELFCLASS::ELFCLASS64>::Chdr {
	chdr_type ch_type;      ///< Compression format
	uint32_t ch_reserved;
	uint64_t ch_size;       ///< Uncompressed data size
	uint64_t ch_addralign;  ///< Uncompressed data alignment
} __attribute__((packed));


template<>
struct Structures<ELFCLASS::ELFCLASS32>::Phdr {
	phdr_type p_type;
	elfoff_t p_offset;
	elfptr_t p_vaddr;
	elfptr_t p_paddr;
	elfptr_t p_filesz;
	elfptr_t p_memsz;
	Phdr_flags p_flags;
	elfptr_t p_align;
} __attribute__((packed));

template<>
struct Structures<ELFCLASS::ELFCLASS64>::Phdr {
	phdr_type p_type;
	Phdr_flags p_flags;
	elfoff_t p_offset;
	elfptr_t p_vaddr;
	elfptr_t p_paddr;
	elfptr_t p_filesz;
	elfptr_t p_memsz;
	elfptr_t p_align;
} __attribute__((packed));


template<>
struct Structures<ELFCLASS::ELFCLASS32>::Sym {
	uint32_t st_name;
	elfptr_t st_value;
	elfptr_t st_size;
	Sym_info st_info;
	Sym_other st_other;
	uint16_t st_shndx;
} __attribute__((packed));

template<>
struct Structures<ELFCLASS::ELFCLASS64>::Sym {
	uint32_t st_name;
	Sym_info st_info;
	Sym_other st_other;
	uint16_t st_shndx;
	elfptr_t st_value;
	elfptr_t st_size;
} __attribute__((packed));


template<>
union Structures<ELFCLASS::ELFCLASS32>::Rel_info {
	elfptr_t value;
	struct {
		elfptr_t type :  8;
		elfptr_t sym  : 24;
	} __attribute__((packed));
};

template<>
union Structures<ELFCLASS::ELFCLASS64>::Rel_info {
	elfptr_t value;
	struct {
		elfptr_t type : 32;
		elfptr_t sym  : 32;
	} __attribute__((packed));
};

} // ELF_Def
