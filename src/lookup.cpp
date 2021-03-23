#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "elf_dyn.hpp"

template<ELFCLASS C>
bool elflookup(void * addr, size_t length, const std::vector<const char*> & symbols) {
	puts("HI");
	ELF_Dyn<C> elf(reinterpret_cast<uintptr_t>(addr), length);
	puts("HA");
	if (!elf.valid()) {
		::fputs("No valid ELF file!", stderr);
		return false;
	}
	puts("HO");
	for (auto & sym : symbols) {
		::fprintf(stdout, "Test %s = %u\n", sym, elf.symbols.index(sym));

	}
	return elf.valid();
}

bool lookup(void * addr, size_t length, const std::vector<const char*> & symbols){
	// Read ELF Identification
	ELF_Ident * ident = reinterpret_cast<ELF_Ident *>(addr);
	if (length < sizeof(ELF_Ident) || !ident->valid()) {
		::fputs("No valid ELF identification header!", stderr);
		return false;
	} else if (!ident->data_supported()) {
		::fputs("Unsupported encoding!", stderr);
		return false;
	} else {
		switch (ident->elfclass()) {
			case ELFCLASS::ELFCLASS32:
				return elflookup<ELFCLASS::ELFCLASS64>(addr, length, symbols);

			case ELFCLASS::ELFCLASS64:
				return elflookup<ELFCLASS::ELFCLASS64>(addr, length, symbols);

			default:
				fputs("Unsupported class!", stderr);
				return false;
		}
	}
}

int main(int argc, const char *argv[]) {
	// Check arguments
	if (argc < 3) {
		::fprintf(stderr, "Usage: %s ELF-FILE SYMBOL [MORE SYMBOLS]\n", argv[0]);
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
