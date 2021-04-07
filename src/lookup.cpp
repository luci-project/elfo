#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <iomanip>


#include <vector>

#include "elf_dyn.hpp"

#include "_str_const.hpp"
#include "_str_ident.hpp"

template <ELFCLASS C, class RELOC>
void elfreloc(const ELF_Dyn<C> & elf, const typename ELF_Dyn<C>::Symbol & sym, const typename ELF_Dyn<C>::template Array<RELOC> & relocations) {
	for (auto & rel : relocations)
		if (rel.symbol() == sym) {
			std::cout << " Relocation: Offset 0x" << std::hex << rel.offset() << std::endl
			          << "             Type ";
			auto type = rel.type();
			switch (elf.header.machine()) {
				case ELF<C>::EM_386:
				case ELF<C>::EM_486:
					std::cout << static_cast<typename ELF<C>::rel_386>(type);
					break;
				case ELF<C>::EM_X86_64:
					std::cout << static_cast<typename ELF<C>::rel_x86_64>(type);
					break;
				default:
					std::cout << std::hex << type;
			}
			std::cout << std::endl
			          << "             Addend " << std::dec << rel.addend() << std::endl;
		}
}

template<ELFCLASS C>
void elfsymbol(const ELF_Dyn<C> & elf, const typename ELF_Dyn<C>::Symbol & sym) {
	auto index = elf.symbols.index(sym);
	std::cout << "Symbol [" << index << "] '" << sym.name() << "':" << std::endl
	          << "      Value: 0x" << std::hex << std::right << std::setfill('0') << std::setw(16) << sym.value() << std::endl
	          << "       Size: " << std::dec << std::left << std::setw(0) << sym.size() << " Bytes" << std::endl
	          << "       Type: " << sym.type() << std::endl
	          << "       Bind: " << sym.bind() << std::endl
	          << " Visibility: " << sym.visibility() << std::endl
	          << "    Section: ";
	switch (sym.section_index()) {
		case ELF_Dyn<C>::SHN_UNDEF:  std::cout << "UND"; break;
		case ELF_Dyn<C>::SHN_ABS:    std::cout << "ABS"; break;
		case ELF_Dyn<C>::SHN_COMMON: std::cout << "CMN"; break;
		case ELF_Dyn<C>::SHN_XINDEX: std::cout << "XDX"; break;
		default: std::cout << sym.section_index() << " (" << elf.sections[sym.section_index()].name() << ")";
	}

	auto version = elf.symbols.version(index);
	std::cout << std::endl
	          << "    Version: " << version << " (" << elf.version_name(version) << ")" << std::endl;

	elfreloc<C, typename ELF_Dyn<C>::Relocation>(elf, sym, elf.relocations);
	elfreloc<C, typename ELF_Dyn<C>::RelocationWithAddend>(elf, sym, elf.relocations_addend);

	std::cout << std::endl;
}

template<ELFCLASS C>
bool elflookup(void * addr, size_t length, const std::vector<const char*> & symbols) {
	ELF_Dyn<C> elf(reinterpret_cast<uintptr_t>(addr));
	if (!elf.valid(length)) {
		std::cerr << "No valid ELF file!" << std::endl;
		return false;
	}

	bool success = true;
	if (symbols.empty()) {
		for (auto & symbol : elf.symbols) {
			if (symbol.valid()) {
				elfsymbol(elf, symbol);
			}
		}
		std::cout << "(" << elf.symbols.count() << " dynamic symbols in file)" << std::endl;
	} else {
		size_t found = 0;
		for (auto & sym : symbols) {
			// Check version
			auto symstr = std::string(sym);
			auto split = symstr.find_last_of('@');
			std::string name = symstr.substr(0, split);

			uint32_t version = ELF_Dyn<C>::VER_NDX_UNKNOWN;
			if (split != symstr.npos) {
				std::string version_name = symstr.substr(split + 1);
				version = elf.version_index(version_name);
				if (version == ELF_Dyn<C>::VER_NDX_UNKNOWN) {
					std::cerr << "Unknown version '" << version_name << "' for symbol '" << sym << "' -- skipping!" << std::endl;
					success = false;
					continue;
				}
			}

			// Find symbol
			auto idx = elf.symbols.index(name, version);
			if (idx != ELF_Dyn<C>::STN_UNDEF) {
				elfsymbol(elf, elf.symbols[idx]);
				found++;
			} else {
				std::cerr << "Symbol '" << sym << "' not found!" << std::endl;
				success = false;
			}
		}
		std::cout << "(found " << found << " of " << symbols.size() << " given dynamic symbols in file)" << std::endl;
	}
	return success;
}

bool lookup(void * addr, size_t length, const std::vector<const char*> & symbols){
	// Read ELF Identification
	ELF_Ident * ident = reinterpret_cast<ELF_Ident *>(addr);
	if (length < sizeof(ELF_Ident) || !ident->valid()) {
		std::cerr << "No valid ELF identification header!" << std::endl;
		return false;
	} else if (!ident->data_supported()) {
		std::cerr << "Unsupported encoding (must be " << ident->data_host() << ")!" << std::endl;
		return false;
	} else {
		switch (ident->elfclass()) {
			case ELFCLASS::ELFCLASS32:
				return elflookup<ELFCLASS::ELFCLASS64>(addr, length, symbols);

			case ELFCLASS::ELFCLASS64:
				return elflookup<ELFCLASS::ELFCLASS64>(addr, length, symbols);

			default:
				std::cerr << "Unsupported class '" << ident->elfclass() << "'" << std::endl;
				return false;
		}
	}
}

int main(int argc, const char *argv[]) {
	// Check arguments
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " ELF-FILE [SYMBOL[S]]" << std::endl;
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
	void * addr = ::mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED) {
		::perror("mmap");
		::close(fd);
		return EXIT_FAILURE;
	}

	// Lookup symbols
	bool success = lookup(addr, length, std::vector<const char*>(argv + 2, argv + argc));

	// Cleanup
	::munmap(addr, length);
	::close(fd);
	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
