#include <iostream>
#include <iomanip>

#include <fstream>
#include <stdint.h>

#include "elf.h"

#include "_str_const.hpp"

using namespace std;

static bool dump_elfheader(const ELF64 & elf) {
	// Header
	cout << "ELF Header ";
	if (elf.header.valid()) {
		cout << " (valid):" << endl;
	} else {
		cout << " is invalid -- abort!" << endl;
		return false;
	}
	cout << "  Magic:  ";
	for (auto & v: elf.header.e_ident.values)
		cout << " " << hex << right << setfill('0') << setw(2) << static_cast<unsigned>(v) ;
	cout << endl
	     << "  File class:                        " << elf.header.ident_class() << endl
	     << "  Data encoding:                     " << elf.header.ident_data() << endl
	     << "  File Version:                      " << elf.header.ident_version() << endl
	     << "  OS/ABI:                            " << elf.header.ident_abi() << endl
	     << "  ABI Version:                       " << dec << static_cast<unsigned>(elf.header.ident_abiversion()) << endl;

	cout << "  Type:                              " << elf.header.type() << endl
	     << "  Machine:                           " << elf.header.machine() << endl
	     << "  Version:                           " << elf.header.version() << endl
	     << "  Entry point address:               " << reinterpret_cast<void*>(elf.header.entry()) << endl
	     << "  Start of program headers:          " << dec << elf.header.e_phoff << " (bytes into file)" << endl
	     << "  Start of section headers:          " << dec << elf.header.e_shoff << " (bytes into file)" << endl
	     << "  Flags:                             " << dec << elf.header.flags() << endl
	     << "  Size of this header:               " << dec << elf.header.e_ehsize << " (bytes)" << endl
	     << "  Size of program headers:           " << dec << elf.header.e_phentsize << " (bytes)" << endl
	     << "  Number of program headers:         " << dec << elf.header.e_phnum << endl
	     << "  Size of section headers:           " << dec << elf.header.e_shentsize << " (bytes)" << endl
	     << "  Number of section headers:         " << dec << elf.header.e_shnum << endl
	     << "  Section header string table index: " << dec << elf.header.e_shstrndx << endl
	     << endl;

	return true;
}


static void dump_sectionheader(const ELF64 & elf) {
	cout << "Section Headers:" << endl
	     << "  [Nr] Name                Type            Address          Off    Size   ES Flg Lk Inf Al" << endl;

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

		cout << "  [" << dec << right << setfill(' ') << setw(2) << elf.sections.index(section) << "]"
		     << " " << left << setfill(' ') << setw(19) << section.name()
		     << " " << left << setfill(' ') << setw(15) << section.type()
		     << " " << hex << right << setfill('0') << setw(16) << section.address()
		     << " " << hex << right << setfill('0') << setw(6) << section.offset()
		     << " " << hex << right << setfill('0') << setw(6) << section.size()
		     << " " << hex << right << setfill('0') << setw(2) << section.entry_size()
		     << " " << right << setfill(' ') << setw(3) << flags
		     << " " << dec << right << setfill(' ') << setw(3) << section.link()
		     << " " << dec << right << setfill(' ') << setw(3) << section.info()
		     << " " << dec << right << setfill(' ') << setw(3) << section.alignment()
		     << endl;
	}
	cout << "Key to Flags:" << endl
	     << "  W (write), A (alloc), X (execute), M (merge), S (strings), I (info), L (link order)," << endl
	     << "  O (extra OS processing required), G (group), T (TLS), C (compressed)" << endl
	     << endl;
}


static void dump_segmentheader(const ELF64 & elf) {
	// Segment
	cout << "Program Headers:" << endl
	     << "  Nr Type              Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align" << endl;
	for (auto & segment: elf.segments) {
		cout << "  "  << dec << right << setfill('0') << setw(2) << elf.segments.index(segment)
		     << " " << left << setfill(' ') << setw(17) << segment.type()
		     << " 0x" << hex << right << setfill('0') << setw(6) << segment.offset()
		     << " 0x" << hex << right << setfill('0') << setw(16) << segment.virt_addr()
		     << " 0x" << hex << right << setfill('0') << setw(16) << segment.phys_addr()
		     << " 0x" << hex << right << setfill('0') << setw(6) << segment.size()
		     << " 0x" << hex << right << setfill('0') << setw(6) << segment.virt_size()
		     << " " << setw(0)
		     << (segment.readable() ? "R" : " ") 
		     << (segment.writeable() ? "W" : " ")
		     << (segment.executable() ? "E" : " ")
		     << " 0x" << hex << left << segment.alignment() << endl;
		if (segment.type() == ELF64::PT_INTERP)
			cout << "         [Requesting program interpreter: " << setw(0) << segment.path() << "]" << endl;
	}
	cout << endl
	     << " Section to Segment mapping:" << endl
	     << "  Segment Sections..." << endl;
	for (auto & segment: elf.segments) {
		cout << "   " << dec << right << setfill('0') << setw(2) << elf.segments.index(segment) << "    ";
		for (auto & section: elf.sections)
			if (segment.offset() <= section.offset() && section.offset() < segment.offset() + segment.size())
				cout << " " << section.name();
		cout << endl;
	}
	cout << endl;
}


static void dump_dynamic(const ELF64 & elf) {
	auto dynamic = elf.dynamic();
	if (dynamic.count() > 0) {
		auto offset = reinterpret_cast<uintptr_t>(dynamic.values) - elf.start;
		cout << setw(0) << "Dynamic section at offset 0x" << hex << offset << " contains " << dec << dynamic.count() << " entries:" << endl
		     << "  Tag                Type                 Name/Value" << endl;
		for (auto & dyn : dynamic) {
			cout << " 0x" << right << setfill('0') << setw(16) << hex << dyn._data->d_tag
			     << " " << left << setfill(' ') << setw(21) << dyn.tag() << setw(0);
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
					cout << "0x" << hex << right << setfill('0') << setw(8) << dyn.value() << setw(0) ;
					for (auto flag : _enum_values_dyn_val_flags) {
						if ((dyn.value() & static_cast<uintptr_t>(flag)) != 0)
							cout << " " << flag;
					}
					break;

				case ELF64::DT_FLAGS_1: 
					cout << "0x" << hex << right << setfill('0') << setw(8) << dyn.value() << setw(0) ;
					for (auto flag : _enum_values_dyn_val_flags_1) {
						if ((dyn.value() & static_cast<uintptr_t>(flag)) != 0)
							cout << " " << flag;
					}
					break;

				case ELF64::DT_FEATURE_1: 
					cout << "0x" << hex << right << setfill('0') << setw(8) << dyn.value() << setw(0) ;
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
					cout << dec << dyn.value() << " (bytes)";
					break;
				
				case ELF64::DT_NUM:
				case ELF64::DT_RELACOUNT:
				case ELF64::DT_RELCOUNT:
				case ELF64::DT_VERDEFNUM:
				case ELF64::DT_VERNEEDNUM:
					cout << dec << dyn.value();
					break;
				
				default:
					cout << hex << "0x" << dyn.value();
					break;
			}
			cout << endl;
		}
		cout << endl;
	}
}


static void dump_relocations(const ELF64 & elf) {
	for (auto & section: elf.sections) {
		if (section.type() == ELF64::SHT_REL || section.type() == ELF64::SHT_RELA) { 
			cout << setw(0) << "Relocation section '" << section.name() << "' at offset 0x" << hex << section.offset() << " contains " << dec << 0<< " entries:" << endl;
//						 << "  Tag                Type                 Name/Value" << endl;
				// TODO...
		}
	}
}


int main(int argc, char * argv[]) {
	for (int a = 0; a < argc; a++) {
		ifstream ifs(argv[a], ios::binary | ios::ate);
		ifstream::pos_type pos = ifs.tellg();

		size_t length = pos;

		char *buf = new char[length];
		ifs.seekg(0, ios::beg);
		ifs.read(buf, length);

		if (buf != nullptr && length > 16 /* && reinterpret_cast<ELF_hdr_ident*>(buf)->ei_class == ELF_hdr_ident::ELFCLASS64*/) {
			cout << "Loading " << argv[a] << " (" << length << " Bytes)" << endl;
			ELF64 elf(reinterpret_cast<uintptr_t>(buf), length);

			// Output similar to `readelf -a -W`
			if (dump_elfheader(elf)) {
				dump_sectionheader(elf);
				dump_segmentheader(elf);
				dump_dynamic(elf);
				dump_relocations(elf);
				// TODO...
			} else {
				cout << "Not a valid ELF file!" << endl;
			}
		}
	}
	return 0;
}
