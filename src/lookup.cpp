// Elfo - a lightweight parser for the Executable and Linking Format
// Copyright 2021-2023 by Bernhard Heinloth <heinloth@cs.fau.de>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#ifdef USE_DLH
#include <dlh/container/vector.hpp>
#include <dlh/stream/output.hpp>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <cxxabi.h>
#include <iostream>
#include <iomanip>
#include <vector>
template<class T>
using Vector = std::vector<T, std::allocator<T>>;
using std::cerr;
using std::cout;
using std::dec;
using std::hex;
using std::endl;
using std::left;
using std::setw;
using std::setfill;
using std::right;
#endif

#include "elf_dyn.hpp"

#include "_str_const.hpp"
#include "_str_ident.hpp"

template<ELFCLASS C>
static void elfsymbolreloc(const ELF_Dyn<C> & elf, const typename ELF_Dyn<C>::Symbol & sym, bool plt) {
	for (auto & rel : plt ? elf.relocations_plt : elf.relocations)
		if (rel.symbol() == sym) {
			cout << (plt ? " PLT Reloc." : " Relocation") << ": Offset 0x" << hex << rel.offset() << endl
			     << "             Type ";
			auto type = rel.type();
			switch (elf.header.machine()) {
				case ELF<C>::EM_386:
				case ELF<C>::EM_486:
					cout << static_cast<typename ELF<C>::rel_386>(type);
					break;
				case ELF<C>::EM_X86_64:
					cout << static_cast<typename ELF<C>::rel_x86_64>(type);
					break;
				default:
					cout << hex << type;
			}
			cout << endl
			     << "             Addend " << dec << rel.addend() << endl;
		}
}

template<ELFCLASS C>
static void elfsymbol(const ELF_Dyn<C> & elf, const typename ELF_Dyn<C>::Symbol & sym) {
	auto index = elf.symbols.index(sym);
	cout << "Symbol [" << index << "] '" << sym.name() << "':" << endl;

#ifdef USE_DLH
	const char * sym_name = sym.name();
	if (sym_name[0] == '_' && sym_name[1] == 'Z') {
		cout << "  Demangled: " << flush;
		const char * argv[] = { "/bin/c++filt", "-i", sym_name, nullptr };
		int pid = fork();
		if (pid == 0) {
			Syscall::execv(argv[0], argv);
			exit(EXIT_FAILURE);
		} else {
			int status = 0;
			Syscall::waitpid(pid, &status, 0);
			if (WIFSIGNALED(status))
				cout << '[' << argv[0] << " aborted with signal " << WTERMSIG(status) << ']' << endl;
			else if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
				cout << '[' << argv[0] << " exited with status " << WEXITSTATUS(status) << ']' << endl;
		}
	}
#else
	int status;
	char * name = abi::__cxa_demangle(sym.name(), 0, 0, &status);
	if (status == 0 && strcmp(name, sym.name()) != 0)
		cout << "  Demangled: " << name << endl;
	free(name);
#endif

	cout << "      Value: 0x" << hex << right << setfill('0') << setw(16) << sym.value() << endl
	     << "       Size: " << dec << left << setw(0) << sym.size() << " Bytes" << endl
	     << "       Type: " << sym.type() << endl
	     << "       Bind: " << sym.bind() << endl
	     << " Visibility: " << sym.visibility() << endl
	     << "    Section: ";
	switch (sym.section_index()) {
		case ELF_Dyn<C>::SHN_UNDEF:  cout << "UND"; break;
		case ELF_Dyn<C>::SHN_ABS:    cout << "ABS"; break;
		case ELF_Dyn<C>::SHN_COMMON: cout << "CMN"; break;
		case ELF_Dyn<C>::SHN_XINDEX: cout << "XDX"; break;
		default: cout << sym.section_index() << " (" << elf.sections.at(sym.section_index()).name() << ")";
	}

	auto version = elf.symbols.version(index);
	cout << endl
	     << "    Version: " << version << " (" << elf.version_name(version) << ")" << endl;
	elfsymbolreloc(elf, sym, false);
	elfsymbolreloc(elf, sym, true);
	cout << endl;
}

template<ELFCLASS C>
static  bool elflookup(void * addr, size_t length, const Vector<char*> & symbols) {
	ELF_Dyn<C> elf(reinterpret_cast<uintptr_t>(addr));
	if (!elf.valid(length)) {
		cerr << "No valid ELF file!" << endl;
		return false;
	}

	bool success = true;
	if (symbols.empty()) {
		for (auto & symbol : elf.symbols)
			if (symbol.valid())
				elfsymbol(elf, symbol);
		cout << "(" << elf.symbols.count() << " dynamic symbols in file)" << endl;
	} else {
		size_t found = 0;
		for (char * name : symbols) {
			// Check version
			uint32_t version = ELF_Dyn<C>::VER_NDX_GLOBAL;
			char * version_name = strrchr(name, '@');
			if (version_name != nullptr) {
				// Replace '@' by end delimiter
				*(version_name++) = '\0';

				version = elf.version_index(version_name);
				if (version == ELF_Dyn<C>::VER_NDX_GLOBAL) {
					cerr << "Unknown version '" << version_name << "' for symbol '" << name << "' -- skipping!" << endl;
					success = false;
					continue;
				}
			}

			// Find symbol
			auto idx = elf.symbols.index(name, version);
			if (idx != ELF_Dyn<C>::STN_UNDEF) {
				elfsymbol(elf, elf.symbols.at(idx));
				found++;
			} else {
				cerr << "Symbol '" << name << "' not found!" << endl;
				success = false;
			}
		}
		cout << "(found " << found << " of " << symbols.size() << " given dynamic symbols in file)" << endl;
	}
	return success;
}

static bool lookup(void * addr, size_t length, const Vector<char*> & symbols) {
	// Read ELF Identification
	ELF_Ident * ident = reinterpret_cast<ELF_Ident *>(addr);
	if (length < sizeof(ELF_Ident) || !ident->valid()) {
		cerr << "No valid ELF identification header!" << endl;
		return false;
	} else if (!ident->data_supported()) {
		cerr << "Unsupported encoding (must be " << ELF_Ident::data_host() << ")!" << endl;
		return false;
	} else {
		switch (ident->elfclass()) {
			case ELFCLASS::ELFCLASS32:
				return elflookup<ELFCLASS::ELFCLASS64>(addr, length, symbols);

			case ELFCLASS::ELFCLASS64:
				return elflookup<ELFCLASS::ELFCLASS64>(addr, length, symbols);

			default:
				cerr << "Unsupported class '" << ident->elfclass() << "'" << endl;
				return false;
		}
	}
}

int main(int argc, char *argv[]) {
	// Check arguments
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " ELF-FILE [SYMBOL[S]]" << endl;
		return EXIT_FAILURE;
	}

	// Open file
	int fd = ::open(argv[1], O_RDONLY);
	if (fd == -1) {
		::perror("open");
		return EXIT_FAILURE;
	}

	// Determine file size
	struct stat sb;
	if (::fstat(fd, &sb) == -1) {
		::perror("fstat");
		::close(fd);
		return EXIT_FAILURE;
	}
	size_t length = sb.st_size;

	// Map file
	void * addr = ::mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED) {
		::perror("mmap");
		::close(fd);
		return EXIT_FAILURE;
	}

	// Lookup symbols
	bool success = lookup(addr, length, Vector<char*>(argv + 2, argv + argc));

	// Cleanup
	::munmap(addr, length);
	::close(fd);
	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
