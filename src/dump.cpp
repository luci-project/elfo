#include <iostream>
#include <iomanip>

#include <fstream>
#include <stdint.h>

#include "elf.hpp"

#include "_str_const.hpp"

using namespace std;
#define PAD(LEN) dec << left << setfill(' ') << setw(LEN)
#define RESET() dec << left << setw(0)
#define HEX() "0x" << hex << left << setw(0)
#define DEC() RESET()
#define HEXPADSHORT(LEN) hex << right << setfill('0') << setw(LEN)
#define HEXPAD(LEN) "0x" << HEXPADSHORT(LEN)
#define DECPAD(LEN) dec << right << setfill(' ') << setw(LEN)


static bool dump_elfheader(const ELF64 & elf) {
	// Header
	cout << "ELF Header";
	if (elf.header.valid()) {
		cout << " (valid):" << endl;
	} else {
		cout << " is invalid -- abort!" << endl;
		return false;
	}
	cout << "  Magic:  ";
	for (auto & v: elf.header.e_ident.values)
		cout << " " << HEXPADSHORT(2) << static_cast<unsigned>(v) ;
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
	     << RESET() << endl;

	return true;
}


static void dump_sectionheader(const ELF64 & elf) {
	cout << "Section Headers:" << endl
	     << "  [Nr] Name                Type            Address            Off      Size     EnSz Flg Lk Inf Al" << endl;

	// Section
	for (auto & section: elf.sections) {
		string flags;
		if (section.writeable()) flags.append("W");
		if (section.allocate()) flags.append("A");
		if (section.executable()) flags.append("X");
		if (section.merge()) flags.append("M");
		if (section.strings()) flags.append("S");
		if (section.info_link()) flags.append("I");
		if (section.link_order()) flags.append("L");
		if (section.os_nonconforming()) flags.append("O");
		if (section.group()) flags.append("G");
		if (section.tls()) flags.append("T");
		if (section.compressed()) flags.append("C");

		cout << "  [" << DECPAD(2) << elf.sections.index(section) << "]"
		     << " " << PAD(19) << section.name()
		     << " " << PAD(15) << section.type()
		     << " " << HEXPAD(16) << section.virt_addr()
		     << " " << HEXPAD(6) << section.offset()
		     << " " << HEXPAD(6) << section.size()
		     << " " << HEXPAD(2) << section.entry_size()
		     << " " << DECPAD(3) << flags
		     << " " << DECPAD(2) << section.link()
		     << " " << DECPAD(3) << section.info()
		     << " " << DECPAD(2) << section.alignment()
		     << endl;
	}
	cout << "Key to Flags:" << endl
	     << "  W (write), A (alloc), X (execute), M (merge), S (strings), I (info), L (link order)," << endl
	     << "  O (extra OS processing required), G (group), T (TLS), C (compressed)" << endl
	     << RESET() << endl;
}


static void dump_segmentheader(const ELF64 & elf) {
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
		if (segment.type() == ELF64::PT_INTERP)
			cout << "         [Requesting program interpreter: " << RESET() << segment.path() << "]" << endl;
	}
	cout << endl
	     << " Section to Segment Nr mapping:" << endl
	     << "  Nr Sections" << endl;
	for (auto & segment: elf.segments) {
		cout << "  " << DECPAD(2) << elf.segments.index(segment);
		for (auto & section: elf.sections)
			if (segment.offset() <= section.offset() && section.offset() < segment.offset() + segment.size() && !string(section.name()).empty())
				cout << " " << section.name();
		cout << endl;
	}
	cout << RESET() << endl;
}


static void dump_dynamic(const ELF64 & elf, const ELF64::Section & section) {
	auto dynamic = section.get_dynamic();
	if (dynamic.count() > 0) {
		auto offset = reinterpret_cast<uintptr_t>(dynamic.values) - elf.start;
		cout << "Dynamic section at offset " << HEX() << offset << " contains " << DEC() << dynamic.count() << " entries:" << endl
		     << "  Tag                Type                 Name/Value" << endl;
		for (auto & dyn : dynamic) {
			cout << "  " << HEXPAD(16) << dyn.ptr()->d_tag
			     << " " << PAD(21) << dyn.tag() << RESET();
			switch (dyn.tag()) {
				case ELF64::DT_NEEDED:
					cout << "Shared library: [" << dyn.string() << "]";
					break;

				case ELF64::DT_SONAME:
					cout << "Library soname: [" << dyn.string() << "]";
					break;

				case ELF64::DT_RPATH:
				case ELF64::DT_RUNPATH:
					cout << "Library search path: [" << dyn.string() << "]";
					break;

				case ELF64::DT_FLAGS:
					cout << HEXPAD(8) << dyn.value() << RESET();
					for (auto flag : _enum_values_dyn_val_flags) {
						if ((dyn.value() & static_cast<uintptr_t>(flag)) != 0)
							cout << " " << flag;
					}
					break;

				case ELF64::DT_FLAGS_1:
					cout << HEXPAD(8) << dyn.value() << RESET();
					for (auto flag : _enum_values_dyn_val_flags_1) {
						if ((dyn.value() & static_cast<uintptr_t>(flag)) != 0)
							cout << " " << flag;
					}
					break;

				case ELF64::DT_FEATURE_1:
					cout << HEXPAD(8) << dyn.value() << RESET();
					for (auto flag : _enum_values_dyn_val_feature_1) {
						if ((dyn.value() & static_cast<uintptr_t>(flag)) != 0)
							cout << " " << flag;
					}
					break;
				case ELF64::DT_PLTRELSZ:
				case ELF64::DT_RELASZ:
				case ELF64::DT_RELAENT:
				case ELF64::DT_STRSZ:
				case ELF64::DT_RELSZ:
				case ELF64::DT_RELENT:
				case ELF64::DT_INIT_ARRAYSZ:
				case ELF64::DT_FINI_ARRAYSZ:
				case ELF64::DT_PREINIT_ARRAYSZ:
				case ELF64::DT_GNU_CONFLICTSZ:
				case ELF64::DT_GNU_LIBLISTSZ:
				case ELF64::DT_PLTPADSZ:
				case ELF64::DT_MOVESZ:
				case ELF64::DT_SYMINSZ:
				case ELF64::DT_SYMINENT:
					cout << DEC() << dyn.value() << " (bytes)";
					break;

				case ELF64::DT_NUM:
				case ELF64::DT_RELACOUNT:
				case ELF64::DT_RELCOUNT:
				case ELF64::DT_VERDEFNUM:
				case ELF64::DT_VERNEEDNUM:
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
}

template <class R>
static void dump_relocations(const ELF64 & elf, const ELF64::Section & section) {
	ELF64::Array<R> relocations = section.get_array<R>();
	cout << "Relocation section '" << section.name() << "' at offset " << HEX() << section.offset() << " contains " << DEC() << relocations.count() << " entries:" << endl
	     << "  Offset             Info               Type               Symbol's Value     Target (Symbol's Name + Addend)" << endl;
	for (auto & rel : relocations) {
		cout << "  " << HEXPAD(16) << rel.offset()
		     << " " << HEXPAD(16) << rel.info()
		     << " ";
		auto type = rel.type();
		switch (elf.header.machine()) {
			case ELF64::EM_386:
			case ELF64::EM_486:
				cout << PAD(18) << static_cast<ELF64::rel_386>(type);
				break;
			case ELF64::EM_X86_64:
				cout << PAD(18) << static_cast<ELF64::rel_x86_64>(type);
				break;
			default:
				cout << HEXPAD(16) << type << "     ";
		}
		auto sym = rel.symbol();
		if (sym.valid()) {
			cout << " " << HEXPAD(16) << sym.value()
			     << " " << sym.name() << " + ";
		} else {
			cout << "                    ";
		}
		cout << DEC() << rel.addend() << endl;
	}
	cout << RESET() << endl;
}


static void dump_symbols(const ELF64 & elf, const ELF64::Section & section) {
	auto symbols = section.get_symbols();
	cout << "Symbol table '" << section.name() << "' at offset " << HEX() << section.offset() << " contains " << DEC() << symbols.count() << " entries:" << endl
	     << "   Num Value              Size    Type       Bind   Vis      Ndx Name" << endl;
	for (auto & sym: symbols) {
		cout << " " << DECPAD(5) << symbols.index(sym)
		     << " " << HEXPAD(16) << sym.value()
		     << " " << DECPAD(5) << sym.size()
		     << " " << PAD(11) << sym.type()
		     << " " << PAD(12) << sym.bind()
		     << " " << PAD(12) << sym.visibility()
		     << " ";
		switch (sym.section_index()) {
			case ELF64::SHN_UNDEF:
				cout << "UND";
				break;
			case ELF64::SHN_ABS:
				cout << "ABS";
				break;
			case ELF64::SHN_COMMON:
				cout << "CMN";
				break;
			case ELF64::SHN_XINDEX:
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

__attribute__((weak)) extern void foo() {}

static void dump_notes_helper(const ELF64 & elf, const ELF64::Note & note) {
	if (!elf.header.type() != ELF64::ET_CORE) {
		if (note.name() != nullptr && string(note.name()) == "GNU") {
			switch (note.type()) {
				case ELF64::NT_GNU_ABI_TAG:
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

				case ELF64::NT_GNU_HWCAP:
				{
					cout << "NT_GNU_HWCAP: ";
					auto desc = reinterpret_cast<const uint32_t *>(note.description());
					for (size_t i = 0; i < note.size() / sizeof(uint32_t); i++) {
						cout << " " << HEXPAD(8) << desc[i];
					}
					return;
				}

				case ELF64::NT_GNU_BUILD_ID:
				{
					cout << "NT_GNU_BUILD_ID: ";
					auto desc = reinterpret_cast<const uint8_t *>(note.description());
					for (size_t i = 0; i < note.size(); i++) {
						cout << HEXPADSHORT(2) << static_cast<const uint32_t>(desc[i]);
					}
					return;
				}

				case ELF64::NT_GNU_GOLD_VERSION:
				{
					cout << "NT_GNU_GOLD_VERSION: ";
					auto desc = reinterpret_cast<const uint8_t *>(note.description());
					for (size_t i = 0; i < note.size(); i++)
						cout << HEXPADSHORT(2) << static_cast<const uint32_t>(desc[i]);
					return;
				}

				case ELF64::NT_GNU_PROPERTY_TYPE_0:
				{
					cout << "NT_GNU_PROPERTY_TYPE_0: ";
					auto desc = reinterpret_cast<const uint32_t *>(note.description());
					for (size_t i = 0; i < note.size() / sizeof(uint32_t); i++) {
						cout << " " << HEXPAD(8) << desc[i];
					}
					return;
				}
			}
		} else {
			switch (note.type()) {
				case ELF64::NT_VERSION:
					cout << "NT_ARCH: " << reinterpret_cast<const char *>(note.description());
					return;

				case ELF64::NT_ARCH:
					cout << "NT_ARCH: " << reinterpret_cast<const char *>(note.description());
					return;
			}
		}
	}
	cout << note.type() << ":";
	auto desc = reinterpret_cast<const uint8_t *>(note.description());
	for (size_t i = 0; i < note.size(); i++)
		cout << " " << HEXPADSHORT(2) << static_cast<const uint32_t>(desc[i]);
}

static void dump_notes(const ELF64 & elf, const ELF64::Section & section) {
	auto notes = section.get_notes();
	cout << "Notes '" << section.name() << "' at offset " << HEX() << section.offset() << " contains " << DEC() << notes.count() << " entries:" << endl
	     << "  Owner                Data size  Description" << endl;
	for (auto & note : notes) {
		cout << "  " << PAD(20);
		if (note.name() == nullptr)
			cout << "(NONE)";
		else
			cout << note.name();
		cout << " " << HEXPAD(8) << note.size() << " ";
		dump_notes_helper(elf, note);
		cout << endl;
	}
	cout << RESET() << endl;
}

static void dump_version_definition(const ELF64 & elf, const ELF64::Section & section) {
	auto verdef = section.get_version_definition();
	cout << "Version definition '" << section.name() << "' at offset " << HEX() << section.offset() << " contains "  <<  DEC() << verdef.count() << " entries:" << endl;
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

static void dump_version_needed(const ELF64 & elf, const ELF64::Section & section) {
	auto verneed = section.get_version_needed();
	cout << "Version dependency '" << section.name() << "' at offset " << HEX() << section.offset() << " contains "  << DEC() << verneed.count() << " entries:" << endl;
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

int main(int argc, char * argv[]) {
	if (argc != 2) {
		cout << "Usage: " << argv[0] << " ELF-FILE" << endl;
		return 1;
	} else {
		ifstream ifs(argv[1], ios::binary | ios::ate);
		ifstream::pos_type pos = ifs.tellg();

		size_t length = pos;
		cout << "Loading " << argv[1] << " (" << length << " Bytes)..." << endl;

		char *buf = new char[length];
		ifs.seekg(0, ios::beg);
		ifs.read(buf, length);

		if (buf != nullptr && length > 16 /* && reinterpret_cast<ELF_hdr_ident*>(buf)->ei_class == ELF_hdr_ident::ELFCLASS64*/) {
			ELF64 elf(reinterpret_cast<uintptr_t>(buf), length);

			// Output similar to `readelf -a -W`
			if (dump_elfheader(elf)) {
				dump_sectionheader(elf);
				dump_segmentheader(elf);

				for (auto & section: elf.sections)
					switch(section.type()) {
						case ELF64::SHT_REL:
							dump_relocations<ELF64::Relocation>(elf, section);
							break;
						case ELF64::SHT_RELA:
							dump_relocations<ELF64::RelocationAddend>(elf, section);
							break;
						case ELF64::SHT_SYMTAB:
						case ELF64::SHT_DYNSYM:
							dump_symbols(elf, section);
							break;
						case ELF64::SHT_DYNAMIC:
							dump_dynamic(elf, section);
							break;
						case ELF64::SHT_NOTE:
							dump_notes(elf, section);
							break;
						case ELF64::SHT_GNU_VERDEF:
							dump_version_definition(elf, section);
							break;
						case ELF64::SHT_GNU_VERNEED:
							dump_version_needed(elf, section);
							break;
					}
			} else {
				cout << "Not a valid ELF file!" << endl;
			}
		}
		return 0;
	}
}
