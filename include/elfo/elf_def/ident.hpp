// Elfo - a lightweight parser for the Executable and Linking Format
// Copyright 2021-2023 by Bernhard Heinloth <heinloth@cs.fau.de>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

namespace ELF_Def {

/*! \brief ELF file header */
struct Identification {
	char ei_magic[4];

	enum ident_class : unsigned char {
		ELFCLASSNONE = 0,
		ELFCLASS32   = 1,
		ELFCLASS64   = 2,
	} ei_class;

	enum ident_data : unsigned char {
		ELFDATANONE = 0,
		ELFDATA2LSB = 1,
		ELFDATA2MSB = 2,
	} ei_data;

	enum ident_version : unsigned char {
		ELFVERSION_NONE    = 0,
		ELFVERSION_CURRENT = 1,
	} ei_version;

	enum ident_abi : unsigned char {
		ELFOSABI_NONE       = 0,
		ELFOSABI_SYSV       = 0,
		ELFOSABI_HPUX       = 1,
		ELFOSABI_NETBSD     = 2,
		ELFOSABI_LINUX      = 3,
		ELFOSABI_SOLARIS    = 6,
		ELFOSABI_AIX        = 7,
		ELFOSABI_IRIX       = 8,
		ELFOSABI_FREEBSD    = 9,
		ELFOSABI_TRU64      = 10,
		ELFOSABI_MODESTO    = 11,
		ELFOSABI_OPENBSD    = 12,
		ELFOSABI_OPENVMS    = 13,
		ELFOSABI_NSK        = 14,
		ELFOSABI_AROS       = 15,
		ELFOSABI_FENIXOS    = 16,
		ELFOSABI_ARM        = 97,
		ELFOSABI_STANDALONE = 255
	} ei_abi;

	unsigned char ei_abiversion;
	char padding[7];

	/*! \brief Check if this elf identification header is valid */
	bool valid() const {
		static const char ident_magic[4] = { 0x7f, 'E', 'L', 'F' };

		for (int i = 0; i < 4; i++)
			if (ei_magic[i] != ident_magic[i])
				return false;

		return ei_version == ELFVERSION_CURRENT;
	}

	/*! \brief File class */
	ident_class elfclass() const {
		return ei_class;
	}

	/*! \brief Data encoding */
	ident_data data() const {
		return ei_data;
	}

	/*! \brief Get host data encoding */
	static ident_data data_host() {
		static const int tmp = 1;
		return 1 == *reinterpret_cast<const char*>(&tmp) ? ELFDATA2LSB : ELFDATA2MSB;
	}

	bool data_supported() const {
		return data() == data_host();
	}

	/*! \brief File version */
	ident_version version() const {
		return ei_version;
	}

	/*! \brief OS ABI identification */
	ident_abi abi() const {
		return ei_abi;
	}

	/*! \brief ABI Version */
	unsigned abiversion() const {
		return static_cast<unsigned>(ei_abiversion);
	}

	/*! \brief Compare identification */
	bool operator==(const Identification & other) const {
		return valid() && other.valid()
			&& elfclass()   == other.elfclass()
			&& data()       == other.data()
			&& version()    == other.version()
			&& abi()        == other.abi()
			&& abiversion() == other.abiversion();
	}

	bool operator!=(const Identification & other) const {
		return !operator==(other);
	}
} __attribute__((packed));

}  // namespace ELF_Def

using ELF_Ident = ELF_Def::Identification;
