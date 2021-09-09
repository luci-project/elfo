#pragma once

#ifdef USE_DLH
#include <dlh/types.hpp>
#else
#include <cstdint>
#endif

namespace ELF_Def {

struct Constants {
	enum ehdr_type : uint16_t {
		ET_NONE   = 0,
		ET_REL    = 1,
		ET_EXEC   = 2,
		ET_DYN    = 3,
		ET_CORE   = 4,
		ET_LOOS   = 0xFE00,
		ET_HIOS   = 0xFEFF,
		ET_LOPROC = 0xFF00,
		ET_HIPROC = 0xFFFF
	};

	enum ehdr_machine : uint16_t {
		EM_NONE           = 0,
		EM_M32            = 1,
		EM_SPARC          = 2,
		EM_386            = 3,
		EM_68K            = 4,
		EM_88K            = 5,
		EM_486            = 6,
		EM_860            = 7,
		EM_MIPS           = 8,
		EM_S370           = 9,
		EM_MIPS_RS3_LE    = 10,
		EM_PARISC         = 15,
		EM_VPP550         = 17,
		EM_SPARC32PLUS    = 18,
		EM_960            = 19,
		EM_PPC            = 20,
		EM_PPC64          = 21,
		EM_S390           = 22,
		EM_SPU            = 23,
		EM_V800           = 36,
		EM_FR20           = 37,
		EM_RH32           = 38,
		EM_MCORE          = 39,
		EM_RCE            = 39,
		EM_ARM            = 40,
		EM_OLD_ALPHA      = 41,
		EM_SH             = 42,
		EM_SPARCV9        = 43,
		EM_TRICORE        = 44,
		EM_ARC            = 45,
		EM_H8_300         = 46,
		EM_H8_300H        = 47,
		EM_H8S            = 48,
		EM_H8_500         = 49,
		EM_IA_64          = 50,
		EM_MIPS_X         = 51,
		EM_COLDFIRE       = 52,
		EM_68HC12         = 53,
		EM_MMA            = 54,
		EM_PCP            = 55,
		EM_NCPU           = 56,
		EM_NDR1           = 57,
		EM_STARCORE       = 58,
		EM_ME16           = 59,
		EM_ST100          = 60,
		EM_TINYJ          = 61,
		EM_X86_64         = 62,
		EM_PDSP           = 63,
		EM_PDP10          = 64,
		EM_PDP11          = 65,
		EM_FX66           = 66,
		EM_ST9PLUS        = 67,
		EM_ST7            = 68,
		EM_68HC16         = 69,
		EM_68HC11         = 70,
		EM_68HC08         = 71,
		EM_68HC05         = 72,
		EM_SVX            = 73,
		EM_ST19           = 74,
		EM_VAX            = 75,
		EM_CRIS           = 76,
		EM_JAVELIN        = 77,
		EM_FIREPATH       = 78,
		EM_ZSP            = 79,
		EM_MMIX           = 80,
		EM_HUANY          = 81,
		EM_PRISM          = 82,
		EM_AVR            = 83,
		EM_FR30           = 84,
		EM_D10V           = 85,
		EM_D30V           = 86,
		EM_V850           = 87,
		EM_M32R           = 88,
		EM_MN10300        = 89,
		EM_MN10200        = 90,
		EM_PJ             = 91,
		EM_OPENRISC       = 92,
		EM_ARC_A5         = 93,
		EM_XTENSA         = 94,
		EM_VIDEOCORE      = 95,
		EM_TMM_GPP        = 96,
		EM_NS32K          = 97,
		EM_TPC            = 98,
		EM_SNP1K          = 99,
		EM_ST200          = 100,
		EM_IP2K           = 101,
		EM_MAX            = 102,
		EM_CR             = 103,
		EM_F2MC16         = 104,
		EM_MSP430         = 105,
		EM_BLACKFIN       = 106,
		EM_SE_C33         = 107,
		EM_SEP            = 108,
		EM_ARCA           = 109,
		EM_UNICORE        = 110,
		EM_EXCESS         = 111,
		EM_DXP            = 112,
		EM_ALTERA_NIOS2   = 113,
		EM_CRX            = 114,
		EM_XGATE          = 115,
		EM_C166           = 116,
		EM_M16C           = 117,
		EM_DSPIC30F       = 118,
		EM_CE             = 119,
		EM_M32C           = 120,
		EM_TSK3000        = 131,
		EM_RS08           = 132,
		EM_ECOG2          = 134,
		EM_SCORE          = 135,
		EM_SCORE7         = 135,
		EM_DSP24          = 136,
		EM_VIDEOCORE3     = 137,
		EM_LATTICEMICO32  = 138,
		EM_SE_C17         = 139,
		EM_TI_C6000       = 140,
		EM_TI_C2000       = 141,
		EM_TI_C5500       = 142,
		EM_MMDSP_PLUS     = 160,
		EM_CYPRESS_M8C    = 161,
		EM_R32C           = 162,
		EM_TRIMEDIA       = 163,
		EM_QDSP6          = 164,
		EM_8051           = 165,
		EM_STXP7X         = 166,
		EM_NDS32          = 167,
		EM_ECOG1          = 168,
		EM_ECOG1X         = 168,
		EM_MAXQ30         = 169,
		EM_XIMO16         = 170,
		EM_MANIK          = 171,
		EM_CRAYNV2        = 172,
		EM_RX             = 173,
		EM_METAG          = 174,
		EM_MCST_ELBRUS    = 175,
		EM_ECOG16         = 176,
		EM_CR16           = 177,
		EM_ETPU           = 178,
		EM_SLE9X          = 179,
		EM_L1OM           = 180,
		EM_INTEL181       = 181,
		EM_INTEL182       = 182,
		EM_AVR32          = 185,
		EM_STM8           = 186,
		EM_TILE64         = 187,
		EM_TILEPRO        = 188,
		EM_MICROBLAZE     = 189,
		EM_CUDA           = 190,
		EM_TILEGX         = 191,
		EM_CLOUDSHIELD    = 192,
		EM_COREA_1ST      = 193,
		EM_COREA_2ND      = 194,
		EM_ARC_COMPACT2   = 195,
		EM_OPEN8          = 196,
		EM_RL78           = 197,
		EM_VIDEOCORE5     = 198,
		EM_78KOR          = 199,
		EM_56800EX        = 200,
		EM_BA1            = 201,
		EM_BA2            = 202,
		EM_XCORE          = 203,
		EM_MCHP_PIC       = 204,
		EM_INTEL205       = 205,
		EM_INTEL206       = 206,
		EM_INTEL207       = 207,
		EM_INTEL208       = 208,
		EM_INTEL209       = 209,
		EM_KM32           = 210,
		EM_KMX32          = 211,
		EM_KMX16          = 212,
		EM_KMX8           = 213,
		EM_KVARC          = 214,
		EM_CDP            = 215,
		EM_COGE           = 216,
		EM_COOL           = 217,
		EM_NORC           = 218,
		EM_CSR_KALIMBA    = 219,
		EM_Z80            = 220,
		EM_VISIUM         = 221,
		EM_FT32           = 222,
		EM_MOXIE          = 223,
		EM_AMDGPU         = 224,
		EM_RISCV          = 243,
		EM_LANAI          = 244,
		EM_CEVA           = 245,
		EM_CEVA_X2        = 246,
		EM_BPF            = 247,
		EM_GRAPHCORE_IPU  = 248,
		EM_IMG1           = 249,
		EM_NFP            = 250,
		EM_CSKY           = 252,
	};

	enum ehdr_version : uint32_t {
		EV_NONE    = 0,
		EV_CURRENT = 1,
	};

	enum shdr_type : uint32_t {
		SHT_NULL           = 0,
		SHT_PROGBITS       = 1,
		SHT_SYMTAB         = 2,
		SHT_STRTAB         = 3,
		SHT_RELA           = 4,
		SHT_HASH           = 5,
		SHT_DYNAMIC        = 6,
		SHT_NOTE           = 7,
		SHT_NOBITS         = 8,
		SHT_REL            = 9,
		SHT_SHLIB          = 10,
		SHT_DYNSYM         = 11,
		SHT_INIT_ARRAY     = 14,
		SHT_FINI_ARRAY     = 15,
		SHT_PREINIT_ARRAY  = 16,
		SHT_GROUP          = 17,
		SHT_SYMTAB_SHNDX   = 18,
		SHT_NUM            = 19,
		SHT_LOOS           = 0x60000000,
		SHT_GNU_ATTRIBUTES = 0x6ffffff5,
		SHT_GNU_HASH       = 0x6ffffff6,
		SHT_GNU_LIBLIST    = 0x6ffffff7,
		SHT_CHECKSUM       = 0x6ffffff8,
		SHT_LOSUNW         = 0x6ffffffa,
		SHT_GNU_VERDEF     = 0x6ffffffd,
		SHT_GNU_VERNEED    = 0x6ffffffe,
		SHT_GNU_VERSYM     = 0x6fffffff,
		SHT_HISUNW         = 0x6fffffff,
		SHT_HIOS           = 0x6fffffff,
		SHT_LOPROC         = 0x70000000,
		SHT_X86_64_UNWIND  = 0x70000001,
		SHT_HIPROC         = 0x7FFFFFFF,
		SHT_LOUSER         = 0x80000000,
		SHT_HIUSER         = 0xFFFFFFFF,
	};

	enum chdr_type : uint32_t {
		ELFCOMPRESS_ZLIB   = 1,           ///< ZLIB/DEFLATE algorithm
		ELFCOMPRESS_LOOS   = 0x60000000,  ///< Start of OS-specific
		ELFCOMPRESS_HIOS   = 0x6fffffff,  ///< End of OS-specific
		ELFCOMPRESS_LOPROC = 0x70000000,  ///< Start of processor-specific
		ELFCOMPRESS_HIPROC = 0x7fffffff,  ///< End of processor-specific
	};

	enum phdr_type : uint32_t {
		PT_NULL         = 0,
		PT_LOAD         = 1,
		PT_DYNAMIC      = 2,
		PT_INTERP       = 3,
		PT_NOTE         = 4,
		PT_SHLIB        = 5,
		PT_PHDR         = 6,
		PT_TLS          = 7,
		PT_NUM          = 8,
		PT_LOOS         = 0x60000000,
		PT_GNU_EH_FRAME = 0x6474e550,
		PT_GNU_STACK    = 0x6474e551,
		PT_GNU_RELRO    = 0x6474e552,
		PT_GNU_PROPERTY = 0x6474e553,
		PT_LOSUNW       = 0x6ffffffa,
		PT_SUNWBSS      = 0x6ffffffa,
		PT_SUNWSTACK    = 0x6ffffffb,
		PT_HISUNW       = 0x6fffffff,
		PT_HIOS         = 0x6fffffff,
		PT_LOPROC       = 0x70000000,
		PT_HIPROC       = 0x7FFFFFFF,
	};

	enum sym_bind {
		STB_LOCAL     = 0,
		STB_GLOBAL    = 1,
		STB_WEAK      = 2,
		STB_LOOS      = 10,
		STB_HIOS      = 12,
		STB_MULTIDEF  = 13,
		STB_LOPROC    = 13,
		STB_HIPROC    = 15,
	};

	enum sym_type {
		STT_NOTYPE             = 0,
		STT_OBJECT             = 1,
		STT_FUNC               = 2,
		STT_SECTION            = 3,
		STT_FILE               = 4,
		STT_COMMON             = 5,
		STT_TLS                = 6,
		STT_GNU_IFUNC          = 10,
		STT_LOOS               = 10,
		STT_HIOS               = 12,
		STT_LOPROC             = 13,
		STT_HIPROC             = 15,
	};

	enum sym_visibility {
		STV_DEFAULT    = 0,
		STV_INTERNAL   = 1,
		STV_HIDDEN     = 2,
		STV_PROTECTED  = 3,
	};

	/*! \brief Symbol table index for undefined symbol */
	static const uint32_t STN_UNDEF = 0;

	enum sym_shndx_special : uint16_t {
		SHN_UNDEF     = 0,       ///< Undefined section
		SHN_LORESERVE = 0xff00,  ///< Start of reserved indices
		SHN_LOPROC    = 0xff00,  ///< Start of processor-specific
		SHN_LOOS      = 0xff20,  ///< Start of OS-specific
		SHN_HIPROC    = 0xff1f,  ///< End of processor-specific
		SHN_HIOS      = 0xff3f,  ///< End of OS-specific
		SHN_ABS       = 0xfff1,  ///< Associated symbol is absolute
		SHN_COMMON    = 0xfff2,  ///< Associated symbol is common
		SHN_XINDEX    = 0xffff,  ///< Index is in extra table
		SHN_HIRESERVE = 0xffff,  ///< End of reserved indices
	};

	enum rel_386 : uint32_t {
		R_386_NONE                = 0,
		R_386_32                  = 1,
		R_386_PC32                = 2,
		R_386_GOT32               = 3,
		R_386_PLT32               = 4,
		R_386_COPY                = 5,
		R_386_GLOB_DAT            = 6,
		R_386_JMP_SLOT            = 7,
		R_386_RELATIVE            = 8,
		R_386_GOTOFF              = 9,
		R_386_GOTPC               = 10,
		R_386_32PLT               = 11,
		R_386_TLS_TPOFF           = 14,
		R_386_TLS_IE              = 15,
		R_386_TLS_GOTIE           = 16,
		R_386_TLS_LE              = 17,
		R_386_TLS_GD              = 18,
		R_386_TLS_LDM             = 19,
		R_386_16                  = 20,
		R_386_PC16                = 21,
		R_386_8                   = 22,
		R_386_PC8                 = 23,
		R_386_TLS_GD_32           = 24,
		R_386_TLS_GD_PUSH         = 25,
		R_386_TLS_GD_CALL         = 26,
		R_386_TLS_GD_POP          = 27,
		R_386_TLS_LDM_32          = 28,
		R_386_TLS_LDM_PUSH        = 29,
		R_386_TLS_LDM_CALL        = 30,
		R_386_TLS_LDM_POP         = 31,
		R_386_TLS_LDO_32          = 32,
		R_386_TLS_IE_32           = 33,
		R_386_TLS_LE_32           = 34,
		R_386_TLS_DTPMOD32        = 35,
		R_386_TLS_DTPOFF32        = 36,
		R_386_TLS_TPOFF32         = 37,
		R_386_SIZE32              = 38,
		R_386_TLS_GOTDESC         = 39,
		R_386_TLS_DESC_CALL       = 40,
		R_386_TLS_DESC            = 41,
		R_386_IRELATIVE           = 42,
		R_386_GOT32X              = 43,
	};

	enum rel_x86_64 : uint64_t {
		R_X86_64_NONE             = 0,
		R_X86_64_64               = 1,
		R_X86_64_PC32             = 2,
		R_X86_64_GOT32            = 3,
		R_X86_64_PLT32            = 4,
		R_X86_64_COPY             = 5,
		R_X86_64_GLOB_DAT         = 6,
		R_X86_64_JUMP_SLOT        = 7,
		R_X86_64_RELATIVE         = 8,
		R_X86_64_GOTPCREL         = 9,
		R_X86_64_32               = 10,
		R_X86_64_32S              = 11,
		R_X86_64_16               = 12,
		R_X86_64_PC16             = 13,
		R_X86_64_8                = 14,
		R_X86_64_PC8              = 15,
		R_X86_64_DTPMOD64         = 16,
		R_X86_64_DTPOFF64         = 17,
		R_X86_64_TPOFF64          = 18,
		R_X86_64_TLSGD            = 19,
		R_X86_64_TLSLD            = 20,
		R_X86_64_DTPOFF32         = 21,
		R_X86_64_GOTTPOFF         = 22,
		R_X86_64_TPOFF32          = 23,
		R_X86_64_PC64             = 24,
		R_X86_64_GOTOFF64         = 25,
		R_X86_64_GOTPC32          = 26,
		R_X86_64_GOT64            = 27,
		R_X86_64_GOTPCREL64       = 28,
		R_X86_64_GOTPC64          = 29,
		R_X86_64_GOTPLT64         = 30,
		R_X86_64_PLTOFF64         = 31,
		R_X86_64_SIZE32           = 32,
		R_X86_64_SIZE64           = 33,
		R_X86_64_GOTPC32_TLSDESC  = 34,
		R_X86_64_TLSDESC_CALL     = 35,
		R_X86_64_TLSDESC          = 36,
		R_X86_64_IRELATIVE        = 37,
		R_X86_64_RELATIVE64       = 38,
		R_X86_64_GOTPCRELX        = 41,
		R_X86_64_REX_GOTPCRELX    = 42,
		R_X86_64_GNU_VTINHERIT    = 250,
		R_X86_64_GNU_VTENTRY      = 251,
	};

	enum dyn_tag : int32_t {
		DT_NULL             = 0,   ///< Marks end of dynamic section
		DT_NEEDED           = 1,   ///< Name of needed library
		DT_PLTRELSZ         = 2,   ///< Size in bytes of PLT reloc
		DT_PLTGOT           = 3,   ///< Processor defined value
		DT_HASH             = 4,   ///< Address of symbol hash table
		DT_STRTAB           = 5,   ///< Address of string table
		DT_SYMTAB           = 6,   ///< Address of symbol table
		DT_RELA             = 7,   ///< Address of Rela relocs
		DT_RELASZ           = 8,   ///< Total size of Rela relocs
		DT_RELAENT          = 9,   ///< Size of one Rela reloc
		DT_STRSZ            = 10,  ///< Size of string table
		DT_SYMENT           = 11,  ///< Size of one symbol table entry
		DT_INIT             = 12,  ///< Address of init function
		DT_FINI             = 13,  ///< Address of termination function
		DT_SONAME           = 14,  ///< Name of shared object
		DT_RPATH            = 15,  ///< Library search path (deprecated
		DT_SYMBOLIC         = 16,  ///< Start symbol search here
		DT_REL              = 17,  ///< Address of Rel relocs
		DT_RELSZ            = 18,  ///< Total size of Rel relocs
		DT_RELENT           = 19,  ///< Size of one Rel reloc
		DT_PLTREL           = 20,  ///< Type of reloc in PLT
		DT_DEBUG            = 21,  ///< For debugging; unspecified
		DT_TEXTREL          = 22,  ///< Reloc might modify .text
		DT_JMPREL           = 23,  ///< Address of PLT relocs
		DT_BIND_NOW         = 24,  ///< Process relocations of object
		DT_INIT_ARRAY       = 25,  ///< Array with addresses of init functions
		DT_FINI_ARRAY       = 26,  ///< Array with addresses of fini functions
		DT_INIT_ARRAYSZ     = 27,  ///< Size in bytes of DT_INIT_ARRAY
		DT_FINI_ARRAYSZ     = 28,  ///< Size in bytes of DT_FINI_ARRAY
		DT_RUNPATH          = 29,  ///< Library search path
		DT_FLAGS            = 30,  ///< Flags for the object being loaded
		DT_PREINIT_ARRAY    = 32,  ///< Array with addresses of preinit functions
		DT_PREINIT_ARRAYSZ  = 33,  ///< size in bytes of DT_PREINIT_ARRAY
		DT_SYMTAB_SHNDX     = 34,  ///< Address of SYMTAB_SHNDX section
		DT_NUM              = 35,  ///< Number used
		DT_LOOS             = 0x6000000D,  ///< Start of OS-specific
		DT_HIOS             = 0x6ffff000,  ///< End of OS-specific
		DT_VALRNGLO         = 0x6ffffd00,
		DT_GNU_PRELINKED    = 0x6ffffdf5,  ///< Prelinking timestamp
		DT_GNU_CONFLICTSZ   = 0x6ffffdf6,  ///<  Size of conflict section
		DT_GNU_LIBLISTSZ    = 0x6ffffdf7,  ///< Size of library list
		DT_CHECKSUM         = 0x6ffffdf8,
		DT_PLTPADSZ         = 0x6ffffdf9,
		DT_MOVEENT          = 0x6ffffdfa,
		DT_MOVESZ           = 0x6ffffdfb,
		DT_FEATURE_1        = 0x6ffffdfc,  ///< Feature selection (DTF_*)
		DT_POSFLAG_1        = 0x6ffffdfd,  ///< Flags for DT_* entries, effecting the following DT_* entry.
		DT_SYMINSZ          = 0x6ffffdfe,  ///< Size of syminfo table (in bytes)
		DT_SYMINENT         = 0x6ffffdff,  ///< Entry size of syminfo
		DT_ADDRRNGLO        = 0x6ffffe00,
		DT_GNU_HASH         = 0x6ffffef5,  ///< GNU-style hash table.
		DT_TLSDESC_PLT      = 0x6ffffef6,
		DT_TLSDESC_GOT      = 0x6ffffef7,
		DT_GNU_CONFLICT     = 0x6ffffef8,  ///< Start of conflict section
		DT_GNU_LIBLIST      = 0x6ffffef9,  ///< Library list
		DT_CONFIG           = 0x6ffffefa,  ///< Configuration information.
		DT_DEPAUDIT         = 0x6ffffefb,  ///< Dependency auditing.
		DT_AUDIT            = 0x6ffffefc,  ///< Object auditing.
		DT_PLTPAD           = 0x6ffffefd,  ///< PLT padding.
		DT_MOVETAB          = 0x6ffffefe,  ///< Move table.
		DT_SYMINFO          = 0x6ffffeff,  ///< Syminfo table.
		DT_VERSYM           = 0x6ffffff0,
		DT_RELACOUNT        = 0x6ffffff9,
		DT_RELCOUNT         = 0x6ffffffa,
		DT_FLAGS_1          = 0x6ffffffb,  ///< State flags, see DF_1_*
		DT_VERDEF           = 0x6ffffffc,  ///< Address of version definition table
		DT_VERDEFNUM        = 0x6ffffffd,  ///< Number of version definitions
		DT_VERNEED          = 0x6ffffffe,  ///< Address of table with needed versions
		DT_VERNEEDNUM       = 0x6fffffff,  ///< Number of needed versions
		DT_LOPROC           = 0x70000000,  ///< Start of processor-specific
		DT_AUXILIARY        = 0x7ffffffd,  ///< Shared object to load before self
		DT_HIPROC           = 0x7fffffff,  ///< End of processor-specific
	};

	/*! \brief Flag bits for DT_FLAGS */
	enum dyn_val_flags {
		DF_ORIGIN     = 0x00000001,  ///< Object may use DF_ORIGIN
		DF_SYMBOLIC   = 0x00000002,  ///< Symbol resolutions starts here
		DF_TEXTREL    = 0x00000004,  ///< Object contains text relocations
		DF_BIND_NOW   = 0x00000008,  ///< No lazy binding for this object
		DF_STATIC_TLS = 0x00000010,  ///< Module uses the static TLS model
	};

	/*! \brief Flag bits for DT_FLAGS_1 */
	enum dyn_val_flags_1 {
		DF_1_NOW        = 0x00000001,  ///< Set RTLD_NOW for this object.
		DF_1_GLOBAL     = 0x00000002,  ///< Set RTLD_GLOBAL for this object.
		DF_1_GROUP      = 0x00000004,  ///< Set RTLD_GROUP for this object.
		DF_1_NODELETE   = 0x00000008,  ///< Set RTLD_NODELETE for this object.*/
		DF_1_LOADFLTR   = 0x00000010,  ///< Trigger filtee loading at runtime.*/
		DF_1_INITFIRST  = 0x00000020,  ///< Set RTLD_INITFIRST for this object*/
		DF_1_NOOPEN     = 0x00000040,  ///< Set RTLD_NOOPEN for this object.
		DF_1_ORIGIN     = 0x00000080,  ///< $ORIGIN must be handled.
		DF_1_DIRECT     = 0x00000100,  ///< Direct binding enabled.
		DF_1_TRANS      = 0x00000200,
		DF_1_INTERPOSE  = 0x00000400,  ///< Object is used to interpose.
		DF_1_NODEFLIB   = 0x00000800,  ///< Ignore default lib search path.
		DF_1_NODUMP     = 0x00001000,  ///< Object can't be dldump'ed.
		DF_1_CONFALT    = 0x00002000,  ///< Configuration alternative created.*/
		DF_1_ENDFILTEE  = 0x00004000,  ///< Filtee terminates filters search.
		DF_1_DISPRELDNE = 0x00008000,  ///< Disp reloc applied at build time.
		DF_1_DISPRELPND = 0x00010000,  ///< Disp reloc applied at run-time.
		DF_1_NODIRECT   = 0x00020000,  ///< Object has no-direct binding.
		DF_1_IGNMULDEF  = 0x00040000,
		DF_1_NOKSYMS    = 0x00080000,
		DF_1_NOHDR      = 0x00100000,
		DF_1_EDITED     = 0x00200000,  ///< Object is modified after built.
		DF_1_NORELOC    = 0x00400000,
		DF_1_SYMINTPOSE = 0x00800000,  ///< Object has individual interposers.
		DF_1_GLOBAUDIT  = 0x01000000,  ///< Global auditing required.
		DF_1_SINGLETON  = 0x02000000,  ///< Singleton symbols are used.
		DF_1_STUB       = 0x04000000,
		DF_1_PIE        = 0x08000000,
	};

	/*! \brief Flag bits for DT_FEATURE_1 */
	enum dyn_val_feature_1 {
		DTF_1_PARINIT = 0x00000001,
		DTF_1_CONFEXP = 0x00000002,
	};


	/*! \brief Notes */
	enum nhdr_type : uint32_t  {
		NT_VERSION             = 1,           ///< A version string of some sort.
		NT_PRSTATUS            = 1,           ///< Contains copy of prstatus struct
		NT_GNU_ABI_TAG         = 1,           ///< OS ABI Information
		NT_ARCH                = 2,           ///< A version string of some sort.
		NT_PRFPREG             = 2,           ///< Contains copy of fpregset struct.
		NT_FPREGSET            = 2,           ///< Contains copy of fpregset struct
		NT_GNU_HWCAP           = 2,           ///< Synthetic hwcap information.
		NT_PRPSINFO            = 3,           ///< Contains copy of prpsinfo struct
		NT_GNU_BUILD_ID        = 3,           ///< Unique build ID as generated by the GNU ld(1) --build-id option.
		NT_PRXREG              = 4,           ///< Contains copy of prxregset struct
		NT_TASKSTRUCT          = 4,           ///< Contains copy of task structure
		NT_GNU_GOLD_VERSION    = 4,           ///< Contains the GNU Gold linker version
		NT_PLATFORM            = 5,           ///< String from sysinfo(SI_PLATFORM)
		NT_GNU_PROPERTY_TYPE_0 = 5,
		NT_AUXV	               = 6,           ///< Contains copy of auxv array
		NT_GWINDOWS            = 7,           ///< Contains copy of gwindows struct
		NT_ASRS	               = 8,           ///< Contains copy of asrset struct
		NT_PSTATUS             = 10,          ///< Contains copy of pstatus struct
		NT_PSINFO              = 13,          ///< Contains copy of psinfo struct
		NT_PRCRED              = 14,          ///< Contains copy of prcred struct
		NT_UTSNAME             = 15,          ///< Contains copy of utsname struct
		NT_LWPSTATUS           = 16,          ///< Contains copy of lwpstatus struct
		NT_LWPSINFO            = 17,          ///< Contains copy of lwpinfo struct
		NT_PRFPXREG            = 20,          ///< Contains copy of fprxregset struct
		NT_SIGINFO             = 0x53494749,  ///< Contains copy of siginfo_t, size might increase
		NT_FILE                = 0x46494c45,  ///< Contains information about mapped files
		NT_PRXFPREG            = 0x46e62b7f,  ///< Contains copy of user_fxsr_struct
		NT_PPC_VMX             = 0x100,       ///< PowerPC Altivec/VMX registers
		NT_PPC_SPE             = 0x101,       ///< PowerPC SPE/EVR registers
		NT_PPC_VSX             = 0x102,       ///< PowerPC VSX registers
		NT_PPC_TAR             = 0x103,       ///< Target Address Register
		NT_PPC_PPR             = 0x104,       ///< Program Priority Register
		NT_PPC_DSCR            = 0x105,       ///< Data Stream Control Register
		NT_PPC_EBB             = 0x106,       ///< Event Based Branch Registers
		NT_PPC_PMU             = 0x107,       ///< Performance Monitor Registers
		NT_PPC_TM_CGPR         = 0x108,       ///< TM checkpointed GPR Registers
		NT_PPC_TM_CFPR         = 0x109,       ///< TM checkpointed FPR Registers
		NT_PPC_TM_CVMX         = 0x10a,       ///< TM checkpointed VMX Registers
		NT_PPC_TM_CVSX         = 0x10b,       ///< TM checkpointed VSX Registers
		NT_PPC_TM_SPR          = 0x10c,       ///< TM Special Purpose Registers
		NT_PPC_TM_CTAR         = 0x10d,       ///< TM checkpointed Target Address Register
		NT_PPC_TM_CPPR         = 0x10e,       ///< TM checkpointed Program Priority Register
		NT_PPC_TM_CDSCR        = 0x10f,       ///< TM checkpointed Data Stream Control Register
		NT_PPC_PKEY            = 0x110,       ///< Memory Protection Keys registers.
		NT_386_TLS             = 0x200,       ///< i386 TLS slots (struct user_desc)
		NT_386_IOPERM          = 0x201,       ///< x86 io permission bitmap (1=deny)
		NT_X86_XSTATE          = 0x202,       ///< x86 extended state using xsave
		NT_S390_HIGH_GPRS      = 0x300,       ///< s390 upper register halves
		NT_S390_TIMER          = 0x301,       ///< s390 timer register
		NT_S390_TODCMP         = 0x302,       ///< s390 TOD clock comparator register
		NT_S390_TODPREG        = 0x303,       ///< s390 TOD programmable register
		NT_S390_CTRS           = 0x304,       ///< s390 control registers
		NT_S390_PREFIX         = 0x305,       ///< s390 prefix register
		NT_S390_LAST_BREAK     = 0x306,       ///< s390 breaking event address
		NT_S390_SYSTEM_CALL    = 0x307,       ///< s390 system call restart data
		NT_S390_TDB            = 0x308,       ///< s390 transaction diagnostic block
		NT_S390_VXRS_LOW       = 0x309,       ///< s390 vector registers 0-15 upper half.
		NT_S390_VXRS_HIGH      = 0x30a,       ///< s390 vector registers 16-31.
		NT_S390_GS_CB          = 0x30b,       ///< s390 guarded storage registers.
		NT_S390_GS_BC          = 0x30c,       ///< s390 guarded storage broadcast control block.
		NT_S390_RI_CB          = 0x30d,       ///< s390 runtime instrumentation.
		NT_ARM_VFP             = 0x400,       ///< ARM VFP/NEON registers
		NT_ARM_TLS             = 0x401,       ///< ARM TLS register
		NT_ARM_HW_BREAK        = 0x402,       ///< ARM hardware breakpoint registers
		NT_ARM_HW_WATCH        = 0x403,       ///< ARM hardware watchpoint registers
		NT_ARM_SYSTEM_CALL     = 0x404,       ///< ARM system call number
		NT_ARM_SVE             = 0x405,       ///< ARM Scalable Vector Extension registers
		NT_VMCOREDD            = 0x700,       ///< Vmcore Device Dump Note.
	};

	// version revision
	enum verdef_version : uint16_t  {
		VER_DEF_NONE    = 0,  ///< No version
		VER_DEF_CURRENT = 1,  ///< Current version
		VER_DEF_NUM     = 2,  ///< Given version number
	};

	// Versym symbol index
	enum verdef_ndx : uint16_t {
		VER_NDX_LOCAL     = 0,       ///< Symbol is local
		VER_NDX_GLOBAL    = 1,       ///< Symbol is global
		VER_NDX_LORESERVE = 0xff00,  ///< Beginning of reserved entries
		VER_NDX_ELIMINATE = 0xff01,  ///< Symbol is to be eliminated
	};

	// version revision
	enum verneed_version : uint16_t  {
		VER_NEED_NONE    = 0,  ///< No version
		VER_NEED_CURRENT = 1,  ///< Current version
		VER_NEED_NUM     = 2,  ///< Given version number
	};
};

} // ELF_Def
