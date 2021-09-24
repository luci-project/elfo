#pragma once

#ifdef USE_DLH
#include <dlh/file.hpp>
#else
#include <fstream>
using namespace std;
#endif
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>

#include <elfo/elf.hpp>

#include "_str_const.hpp"
#include "_str_ident.hpp"

#define PAD(LEN) dec << left << setfill(' ') << setw(LEN)
#define RESET() dec << left << setw(0)
#define HEX() "0x" << hex << left << setw(0)
#define DEC() RESET()
#define HEXPADSHORT(LEN) hex << right << setfill('0') << setw(LEN)
#define HEXPAD(LEN) "0x" << HEXPADSHORT(LEN)
#define DECPAD(LEN) dec << right << setfill(' ') << setw(LEN)

template<ELFCLASS C>
class Dump {
	template <class T> using Array = typename ELF<C>::template Array<T>;
	template <class T> using List = typename ELF<C>::template List<T>;
	using Section = typename ELF<C>::Section;
	using Note = typename ELF<C>::Note;
	using Dynamic = typename ELF<C>::Dynamic;
	using Symbol = typename ELF<C>::Symbol;
	using VersionDefinition = typename ELF<C>::VersionDefinition;
	using VersionNeeded = typename ELF<C>::VersionNeeded;
	const ELF<C> elf;

	void notes_helper(const Note & note) const {
		if (elf.header.type() != ELF<C>::ET_CORE) {
			if (note.name() != nullptr && strcmp(note.name(), "GNU") == 0) {
				switch (note.type()) {
					case ELF<C>::NT_GNU_ABI_TAG:
					{
						assert(note.size() == 16);
						cout << "NT_GNU_ABI_TAG: ";
						auto desc = reinterpret_cast<const uint32_t *>(note.description());
						switch (desc[0]) {
							case 0: cout << "Linux" ; break;
							case 1: cout << "GNU" ; break;
							case 2: cout << "Solaris" ; break;
							case 3: cout << "FreeBSD" ; break;
							default: cout << "Unknown (" << desc[0] << ")"; break;
						}
						cout << " " << DEC() << desc[1] << "." << desc[2] << "." << desc[3];
						return;
					}

					case ELF<C>::NT_GNU_HWCAP:
					{
						cout << "NT_GNU_HWCAP:";
						auto desc = reinterpret_cast<const uint32_t *>(note.description());
						for (size_t i = 0; i < note.size() / sizeof(uint32_t); i++) {
							cout << " " << HEXPAD(8) << desc[i];
						}
						return;
					}

					case ELF<C>::NT_GNU_BUILD_ID:
					{
						cout << "NT_GNU_BUILD_ID: ";
						auto desc = reinterpret_cast<const uint8_t *>(note.description());
						for (size_t i = 0; i < note.size(); i++) {
							cout << HEXPADSHORT(2) << static_cast<uint32_t>(desc[i]);
						}
						return;
					}

					case ELF<C>::NT_GNU_GOLD_VERSION:
					{
						cout << "NT_GNU_GOLD_VERSION: ";
						auto desc = reinterpret_cast<const uint8_t *>(note.description());
						for (size_t i = 0; i < note.size(); i++)
							cout << HEXPADSHORT(2) << static_cast<uint32_t>(desc[i]);
						return;
					}

					case ELF<C>::NT_GNU_PROPERTY_TYPE_0:
					{
						cout << "NT_GNU_PROPERTY_TYPE_0:";
						auto desc = reinterpret_cast<const uint32_t *>(note.description());
						for (size_t i = 0; i < note.size() / sizeof(uint32_t); i++) {
							cout << " " << HEXPAD(8) << desc[i];
						}
						return;
					}
				}
			} else {
				switch (note.type()) {
					case ELF<C>::NT_VERSION:
						cout << "NT_ARCH: " << reinterpret_cast<const char *>(note.description());
						return;

					case ELF<C>::NT_ARCH:
						cout << "NT_ARCH: " << reinterpret_cast<const char *>(note.description());
						return;
				}
			}
		}
		cout << note.type() << ":";
		auto desc = reinterpret_cast<const uint8_t *>(note.description());
		for (size_t i = 0; i < note.size(); i++)
			cout << " " << HEXPADSHORT(2) << static_cast<uint32_t>(desc[i]);
	}

 public:
	Dump(const char * buffer) : elf(reinterpret_cast<uintptr_t>(buffer)) {}

	void dynamic(const Array<Dynamic> & dynamic) const {
		cout << "  Tag                Type                 Name/Value" << endl;
		for (auto & dyn : dynamic) {
			cout << "  " << HEXPAD(16) << dyn.ptr()->d_tag
			     << " " << PAD(21) << dyn.tag() << RESET();
			switch (dyn.tag()) {
				case ELF<C>::DT_NEEDED:
					cout << "Shared library: [" << dyn.string() << "]";
					break;

				case ELF<C>::DT_SONAME:
					cout << "Library soname: [" << dyn.string() << "]";
					break;

				case ELF<C>::DT_RPATH:
				case ELF<C>::DT_RUNPATH:
					cout << "Library search path: [" << dyn.string() << "]";
					break;

				case ELF<C>::DT_FLAGS:
					cout << HEXPAD(8) << dyn.value() << RESET();
					for (auto flag : _enum_values_dyn_val_flags) {
						if ((dyn.value() & static_cast<uintptr_t>(flag)) != 0)
							cout << " " << flag;
					}
					break;

				case ELF<C>::DT_FLAGS_1:
					cout << HEXPAD(8) << dyn.value() << RESET();
					for (auto flag : _enum_values_dyn_val_flags_1) {
						if ((dyn.value() & static_cast<uintptr_t>(flag)) != 0)
							cout << " " << flag;
					}
					break;

				case ELF<C>::DT_FEATURE_1:
					cout << HEXPAD(8) << dyn.value() << RESET();
					for (auto flag : _enum_values_dyn_val_feature_1) {
						if ((dyn.value() & static_cast<uintptr_t>(flag)) != 0)
							cout << " " << flag;
					}
					break;

				case ELF<C>::DT_PLTREL:
					cout << static_cast<typename ELF<C>::dyn_tag>(dyn.value());
					break;

				case ELF<C>::DT_PLTRELSZ:
				case ELF<C>::DT_RELASZ:
				case ELF<C>::DT_RELAENT:
				case ELF<C>::DT_STRSZ:
				case ELF<C>::DT_RELSZ:
				case ELF<C>::DT_RELENT:
				case ELF<C>::DT_INIT_ARRAYSZ:
				case ELF<C>::DT_FINI_ARRAYSZ:
				case ELF<C>::DT_PREINIT_ARRAYSZ:
				case ELF<C>::DT_GNU_CONFLICTSZ:
				case ELF<C>::DT_GNU_LIBLISTSZ:
				case ELF<C>::DT_PLTPADSZ:
				case ELF<C>::DT_MOVESZ:
				case ELF<C>::DT_SYMENT:
				case ELF<C>::DT_SYMINSZ:
				case ELF<C>::DT_SYMINENT:
					cout << DEC() << dyn.value() << " (bytes)";
					break;

				case ELF<C>::DT_NUM:
				case ELF<C>::DT_RELACOUNT:
				case ELF<C>::DT_RELCOUNT:
				case ELF<C>::DT_VERDEFNUM:
				case ELF<C>::DT_VERNEEDNUM:
					cout << DEC() << dyn.value();
					break;

				default:
					cout << HEX() << dyn.value();
					break;
			}
			cout << endl;
		}
		cout << RESET() << endl;
	}

	template <class RELOC>
	void relocations(const Array<RELOC> & relocations) const {
		cout << "  Offset             Info               Type                Symbol's Value     Target (Symbol's Name + Addend)" << endl;
		for (auto & rel : relocations) {
			cout << "  " << HEXPAD(16) << rel.offset()
			     << " " << HEXPAD(16) << rel.info()
			     << " ";
			auto type = rel.type();
			switch (elf.header.machine()) {
				case ELF<C>::EM_386:
				case ELF<C>::EM_486:
					cout << PAD(19) << static_cast<typename ELF<C>::rel_386>(type);
					break;
				case ELF<C>::EM_X86_64:
					cout << PAD(19) << static_cast<typename ELF<C>::rel_x86_64>(type);
					break;
				default:
					cout << HEXPAD(19) << type << "     ";
			}
			auto sym = rel.symbol();
			auto addend = rel.addend();
			if (sym.valid()) {
				cout << " " << HEXPAD(16) << sym.value()
				     << " " << sym.name();
				if (addend >= 0) {
					cout << " + ";
				} else {
					cout << " - ";
					addend *= -1;
				}
			} else {
				cout << "                    ";
			}
			cout << DEC() << addend << endl;
		}
		cout << RESET() << endl;
	}

	void symbols(const Array<Symbol> & symbols) const {
		cout << "   Num Value              Size  Type           Bind         Vis          Ndx Name" << endl;
		for (auto & sym: symbols) {
			cout << " " << DECPAD(5) << symbols.index(sym)
			     << " " << HEXPAD(16) << sym.value()
			     << " " << DECPAD(5) << sym.size()
			     << " " << PAD(14) << sym.type()
			     << " " << PAD(12) << sym.bind()
			     << " " << PAD(12) << sym.visibility()
			     << " ";
			switch (sym.section_index()) {
				case ELF<C>::SHN_UNDEF:
					cout << "UND";
					break;
				case ELF<C>::SHN_ABS:
					cout << "ABS";
					break;
				case ELF<C>::SHN_COMMON:
					cout << "CMN";
					break;
				case ELF<C>::SHN_XINDEX:
					cout << "XDX";
					break;
				default:
					cout << DECPAD(3) << sym.section_index();
			}
			cout << " " << RESET() << sym.name()
				 << endl;
		}
		cout << RESET() << endl;
	}

	void notes(const List<Note> & notes) const {
		cout << "  Owner                Data size  Description" << endl;
		for (auto & note : notes) {
			cout << "  " << PAD(20);
			if (note.name() == nullptr)
				cout << "(NONE)";
			else
				cout << note.name();
			cout << " " << HEXPAD(8) << note.size() << " ";
			notes_helper(note);
			cout << endl;
		}
		cout << RESET() << endl;
	}


	void versions(const uint16_t * version, size_t entries) const {
		cout << "   Num: Index";
		for (size_t i = 0; i < entries; i++) {
			if (i % 10 == 0)
				cout << endl;
			cout << "  " << DECPAD(4) << i << ":"
			     << " " << PAD(2) << (version[i] & 0x7fff)
			     << (version[i] & 0x8000 ? 'h' : ' ');
		}
		cout << RESET() << endl
		     << endl;
	}

	void version_definition(const List<VersionDefinition> & verdef) const {
		for (auto & v : verdef) {
			cout << "  " << HEXPAD(4) << (v.address() - verdef.address())
			     << " Ref: " << v.revision()
			     << "  Flags: " << (v.base() == 1 ? "base" : (v.weak() == 1 ? "weak " : "none")) << " (" << HEXPAD(4) << v.flags() << ")"
			     << "  Index: " << v.version_index()
			     << "  Auxiliary count: " << v.auxiliaries()
			     << endl;
			size_t i = 0;
			for (auto & aux : v.auxiliary()) {
				cout << "  " << HEXPAD(4) << (aux.address() - verdef.address());
				if (i == 0) {
					cout << "    Name: " << aux.name()
					     << " (" << HEXPAD(8) << v.hash() << ")"
					     << endl;
				} else {
					cout << "    Parent " << i
					     << ": " << aux.name()
					     << endl;
				}
				i++;
			}
			assert(i >= 1);
		}
		cout << RESET() << endl;
	}

	void version_needed(const List<VersionNeeded> & verneed) const {
		for (auto & v : verneed) {
			cout << "  " << HEXPAD(4) << (v.address() - verneed.address())
			     << " Version: " << v.version()
			     << "  File: " << v.file()
			     << "  Auxiliary count: " << v.auxiliaries()
			     << endl;
			for (auto & aux : v.auxiliary()) {
				cout << "  " << HEXPAD(4) << (aux.address() - verneed.address())
				     << "   Name: " << aux.name()
				     << " (" << HEXPAD(8) << aux.hash() << ")"
				     << "  Flags: " << (aux.weak() == 1 ? "weak" : "none") << " (" << HEXPAD(4) << aux.flags() << ")"
				     << "  Index: " << DEC() << aux.version_index()
				     << endl;
			}
		}
		cout << RESET() << endl;
	}

	void elf_header() const {
		// Header
		cout << "ELF Header " << (elf.header.valid() ? "(valid)" : "(invalid!)") << endl
		     << "  Magic:  ";
		for (auto & v: elf.header.e_ident)
			cout << " " << HEXPADSHORT(2) << static_cast<unsigned>(v);
		cout << endl
		     << "  File class:                        " << elf.header.ident_class() << endl
		     << "  Data encoding:                     " << elf.header.ident_data() << endl
		     << "  File Version:                      " << elf.header.ident_version() << endl
		     << "  OS/ABI:                            " << elf.header.ident_abi() << endl
		     << "  ABI Version:                       " << DEC() << static_cast<unsigned>(elf.header.ident_abiversion()) << endl;

		cout << "  Type:                              " << elf.header.type() << endl
		     << "  Machine:                           " << elf.header.machine() << endl
		     << "  Version:                           " << elf.header.version() << endl
		     << "  Entry point address:               " << reinterpret_cast<void*>(elf.header.entry()) << endl
		     << "  Start of program headers:          " << DEC() << elf.header.e_phoff << " (bytes into file)" << endl
		     << "  Start of section headers:          " << DEC() << elf.header.e_shoff << " (bytes into file)" << endl
		     << "  Flags:                             " << DEC() << elf.header.flags() << endl
		     << "  Size of this header:               " << DEC() << elf.header.e_ehsize << " (bytes)" << endl
		     << "  Size of program headers:           " << DEC() << elf.header.e_phentsize << " (bytes)" << endl
		     << "  Number of program headers:         " << DEC() << elf.header.e_phnum << endl
		     << "  Size of section headers:           " << DEC() << elf.header.e_shentsize << " (bytes)" << endl
		     << "  Number of section headers:         " << DEC() << elf.header.e_shnum << endl
		     << "  Section header string table index: " << DEC() << elf.header.e_shstrndx << endl
		     << "  Size:                              " << DEC() << elf.size() << " (bytes)" << endl
		     << RESET() << endl;
	}


	void section_header() const {
		cout << "Section Headers:" << endl
		     << "  [Nr] Name                Type             Address            Off      Size     EnSz Flg Lk Inf Al" << endl;

		// Section
		for (auto & section: elf.sections) {
			char flagsbuf[15];
			char * flags = flagsbuf;
			if (section.writeable())        *(flags++) = 'W';
			if (section.allocate())         *(flags++) = 'A';
			if (section.executable())       *(flags++) = 'X';
			if (section.merge())            *(flags++) = 'M';
			if (section.strings())          *(flags++) = 'S';
			if (section.info_link())        *(flags++) = 'I';
			if (section.link_order())       *(flags++) = 'L';
			if (section.os_nonconforming()) *(flags++) = 'O';
			if (section.group())            *(flags++) = 'G';
			if (section.tls())              *(flags++) = 'T';
			if (section.compressed())       *(flags++) = 'C';
			*flags = '\0';

			cout << "  [" << DECPAD(2) << elf.sections.index(section) << "]"
			     << " " << PAD(19) << section.name()
			     << " " << PAD(16) << section.type()
			     << " " << HEXPAD(16) << section.virt_addr()
			     << " " << HEXPAD(6) << section.offset()
			     << " " << HEXPAD(6) << section.size()
			     << " " << HEXPAD(2) << section.entry_size()
			     << " " << DECPAD(3) << flagsbuf
			     << " " << DECPAD(2) << section.link()
			     << " " << DECPAD(3) << section.info()
			     << " " << DECPAD(2) << section.alignment()
			     << endl;
		}
		cout << " Key to Flags:" << endl
		     << "  W (write), A (alloc), X (execute), M (merge), S (strings), I (info), L (link order)," << endl
		     << "  O (extra OS processing required), G (group), T (TLS), C (compressed)" << endl
		     << RESET() << endl;
	}


	void segment_header() const {
		// Segment
		cout << "Program Headers:" << endl
		     << "  Nr Type              Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align" << endl;
		for (auto & segment: elf.segments) {
			cout << "  " << DECPAD(2) << elf.segments.index(segment)
			     << " " << PAD(17) << segment.type()
			     << " " << HEXPAD(6) << segment.offset()
			     << " " << HEXPAD(16) << segment.virt_addr()
			     << " " << HEXPAD(16) << segment.phys_addr()
			     << " " << HEXPAD(6) << segment.size()
			     << " " << HEXPAD(6) << segment.virt_size()
			     << " " << RESET()
			     << (segment.readable() ? "R" : " ")
			     << (segment.writeable() ? "W" : " ")
			     << (segment.executable() ? "E" : " ")
			     << " " << HEX() << segment.alignment() << endl;
			if (segment.type() == ELF<C>::PT_INTERP)
				cout << "         [Requesting program interpreter: " << RESET() << segment.path() << "]" << endl;
		}
		cout << RESET() << endl;
	}

	void section_segment_mapping() const {
		cout << " Section to Segment Nr mapping:" << endl
		     << "  Nr Sections" << endl;
		for (auto & segment: elf.segments) {
			cout << "  " << DECPAD(2) << elf.segments.index(segment);
			for (auto & section: elf.sections)
				if (segment.offset() <= section.offset() && section.offset() < segment.offset() + segment.size() && strlen(section.name()) > 0)
					cout << " " << section.name();
			cout << endl;
		}
		cout << RESET() << endl;
	}

	void contents(bool full = true) {
		elf_header();
		if (full)
			section_header();
		segment_header();
		if (full)
			section_segment_mapping();

		if (full) {
			for (auto & section: elf.sections)
				switch(section.type()) {
					case ELF<C>::SHT_REL:
						{
							auto rel = section.template get_array<typename ELF<C>::RelocationWithoutAddend>();
							cout << "Relocation (without addend) section [" << elf.sections.index(section) << "] '" << section.name() << "' at offset " << HEX() << section.offset() << " contains " << DEC() << rel.count() << " entries:" << endl;
							relocations(rel);
						}
						break;
					case ELF<C>::SHT_RELA:
						{
							auto rel = section.template get_array<typename ELF<C>::RelocationWithAddend>();
							cout << "Relocation (with addend) section [" << elf.sections.index(section) << "] '" << section.name() << "' at offset " << HEX() << section.offset() << " contains " << DEC() << rel.count() << " entries:" << endl;
							relocations(rel);
						}
						break;
					case ELF<C>::SHT_DYNSYM:
						cout << "Dynamic ";
						[[fallthrough]];
					case ELF<C>::SHT_SYMTAB:
						{
							auto sym = section.get_symbols();
							cout << "Symbol table [" << elf.sections.index(section) << "] '" << section.name() << "' at offset " << HEX() << section.offset() << " contains " << DEC() << sym.count() << " entries:" << endl;
							symbols(sym);
						}
						break;
					case ELF<C>::SHT_DYNAMIC:
						{
							auto dyn = section.get_dynamic();
							cout << "Dynamic section [" << elf.sections.index(section) << "] '" << section.name() << "' at offset " << HEX() << section.offset() << " contains " << DEC() << dyn.count() << " entries:" << endl;
							dynamic(dyn);
						}
						break;
					case ELF<C>::SHT_NOTE:
						{
							auto n = section.get_notes();
							cout << "Notes section [" << elf.sections.index(section) << "] '" << section.name() << "' at offset " << HEX() << section.offset() << " contains " << DEC() << n.count() << " entries:" << endl;
							notes(n);
						}
						break;
					case ELF<C>::SHT_GNU_VERSYM:
						{
							cout << "Version symbol [" << elf.sections.index(section) << "] '" << section.name() << "' at offset " << HEX() << section.offset() << " contains " << DEC() << section.entries() << " entries:" << endl;
							versions(section.get_versions(), section.entries());
						}
						break;
					case ELF<C>::SHT_GNU_VERDEF:
						{
							auto verdef = section.get_version_definition();
							cout << "Version definition [" << elf.sections.index(section) << "] '" << section.name() << "' at offset " << HEX() << section.offset() << " contains " << DEC() << verdef.count() << " entries:" << endl;
							version_definition(verdef);
						}
						break;
					case ELF<C>::SHT_GNU_VERNEED:
						{
							auto verneed = section.get_version_needed();
							cout << "Version dependency [" << elf.sections.index(section) << "] '" << section.name() << "' at offset " << HEX() << section.offset() << " contains " << DEC() << verneed.count() << " entries:" << endl;
							version_needed(verneed);
						}
						break;
				}
		} else {
			const auto & dyn = elf.dynamic();
			if (!dyn.empty()) {
				cout << "Dynamic section contains " << DEC() << dyn.count() << " entries:" << endl;
				dynamic(dyn);

				auto sym = dyn.get_symbols();
				cout << "Dynamic Symbol table contains " << sym.count() << " entries:" << endl;
				symbols(sym);

				auto rel = dyn.get_relocations();
				if (!rel.empty()) {
					cout << "Dynamic relocation table (excluding PLT) contains " << DEC() << rel.count() << " entries:" << endl;
					relocations(rel);
				}

				auto relplt = dyn.get_relocations_plt();
				if (!relplt.empty()) {
					cout << "PLT relocation table contains " << DEC() << relplt.count() << " entries:" << endl;
					relocations(relplt);
				}

				auto got = dyn.get_global_offset_table();
				if (!got.empty()) {
					cout << "Global offset table contains " << got.count() << " entries:" << endl;
					for (const auto & e : got)
						cout << "  GOT[" << got.index(e) << "] = " << e.data() << endl;
					cout << endl;
				}

				auto preinit_array = dyn.get_preinit_array();
				auto init = dyn.get_init_function();
				auto init_array = dyn.get_init_array();
				auto fini_array = dyn.get_fini_array();
				auto fini = dyn.get_fini_function();

				if (!preinit_array.empty() || init != nullptr || !init_array.empty() || !fini_array.empty() || fini != nullptr) {
					cout << "(De-)Initialize -- " << (preinit_array.count() + (init == nullptr ? 0 : 1) + init_array.count() + fini_array.count() + (fini == nullptr ? 0 : 1)) << " functions:" << endl;
					for (const auto & i : preinit_array)
						cout << "  - PREINIT_ARRAY " << (void*)i.data() << endl;
					if (init != nullptr)
						cout << "  - INIT " << (void*)init << endl;
					for (const auto & i : init_array)
						cout << "  - INIT_ARRAY " << (void*)i.data() << endl;
					for (const auto & i : fini_array)
						cout << "  - FINI_ARRAY " << (void*)i.data() << endl;
					if (fini != nullptr)
						cout << "  - FINI " << (void*)fini << endl;
					cout << endl;
				}

				auto verdef = dyn.get_version_definition();
				if (!verdef.empty()) {
					cout << "Version definition contains " << verdef.count() << " entries:" << endl;
					version_definition(verdef);
				}

				auto verneed = dyn.get_version_needed();
				if (!verneed.empty()) {
					cout << "Version dependency contains " << verneed.count() << " entries:" << endl;
					version_needed(verneed);
				}

				auto needed = dyn.get_needed();
				if (!needed.empty()) {
					cout << "Binary has " << needed.count() << " library dependencies:" << endl;
					for (const auto & n : needed)
						cout << "  - " << n.string() << endl;
					cout << endl;
				}

				auto rpath = dyn.get_rpath();
				auto runpath = dyn.get_runpath();
				if (!rpath.empty() || !runpath.empty()) {
					cout << "Binary has " << (rpath.count() + runpath.count()) << " library lookup paths:" << endl;
					for (const auto & r : rpath)
						cout << "  - RPATH " << r.string() << endl;
					for (const auto & r : runpath)
						cout << "  - RUNPATH " << r.string() << endl;
					cout << endl;
				}

			}
		}
	}
};

static bool dump(char * file, bool full = true) {
#if USE_DLH
	size_t length;
	char * buf = File::contents(file, length);
	if (buf == nullptr)
		return false;
#else
	// Open File
	ifstream ifs(file, ios::binary | ios::ate);
	if (!ifs.is_open()) {
		cerr << "Opening " << file << " failed!" << endl;
		return false;
	}

	// Get length of file
	ifstream::pos_type pos = ifs.tellg();
	size_t length = pos;
	cout << "File " << file << " (" << length << " Bytes)" << endl
	     << endl;

	// Read file into buffer
	char *buf = new char[length];
	ifs.seekg(0, ios::beg);
	ifs.read(buf, length);
	if (ifs.fail()) {
		cerr << "Reading " << file << " failed!" << endl;
		return false;
	}
#endif

	// Read ELF Identification
	ELF_Ident * ident = reinterpret_cast<ELF_Ident *>(buf);
	if (length < sizeof(ELF_Ident) || !ident->valid()) {
		cerr << "No valid ELF identification header in " << file << "!" << endl;
		return false;
	} else if (!ident->data_supported()) {
		cerr << "Unsupported encoding (" << ident->data() << " instead of " << ident->data_host() << ")!" << endl;
		return false;
	}

	// Dump correct class
	switch (ident->elfclass()) {
		case ELFCLASS::ELFCLASS32:
			Dump<ELFCLASS::ELFCLASS32>(buf).contents(full);
			return true;

		case ELFCLASS::ELFCLASS64:
			Dump<ELFCLASS::ELFCLASS64>(buf).contents(full);
			return true;

		default:
			cerr << "Unsupported class " << ident->elfclass() << "!" << endl;
			return false;
	}
}
