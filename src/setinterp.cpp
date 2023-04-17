// Elfo - a lightweight parser for the Executable and Linking Format
// Copyright 2021-2023 by Bernhard Heinloth <heinloth@cs.fau.de>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <limits.h>
#include <cstring>
#include <iostream>
#ifndef USE_DLH
using std::cerr;
using std::cout;
using std::endl;
#endif


#include <elfo/elf.hpp>

template<ELFCLASS C>
static bool setinterp(void * addr, size_t length, const char * interp) {
	ELF<C> elf(reinterpret_cast<uintptr_t>(addr));
	if (!elf.valid(length))
		cerr << "No valid ELF file!" << endl;

	char * interp_elf = const_cast<char *>(elf.interpreter());
	if (interp_elf == nullptr) {
		cerr << "No interpreter in ELF file!" << endl;
	} else if (strlen(interp_elf) < strlen(interp)) {
		cerr << "New interpreter must not exceed length of old interpreter in ELF file (or have a look at patchelf)!" << endl;
	} else {
		cout << "Changing '" << interp_elf << "' to '" << interp << "'..." << endl;
		return strncpy(interp_elf, interp, PATH_MAX) == interp_elf;
	}

	return false;
}

int main(int argc, char *argv[]) {
	// Check arguments
	if (argc != 3) {
		cerr << "Usage: " << argv[0] << " ELF-FILE INTERP" << endl;
		return EXIT_FAILURE;
	}

	// Check interp
	if (::access(argv[2], X_OK) != 0) {
		cerr << "Interpreter '" << argv[2] << "' not exectuable..." << endl;
		return EXIT_FAILURE;
	} else if (argv[2][0] != '/') {
		cerr << "Interpreter '" << argv[2] << "' is not an absolute path..." << endl;
		return EXIT_FAILURE;
	}

	// Open file
	int fd = ::open(argv[1], O_RDWR);
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
	void * addr = ::mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED) {
		::perror("mmap");
		::close(fd);
		return EXIT_FAILURE;
	}

	// Lookup interpreter
	bool success = false;
	ELF_Ident * ident = reinterpret_cast<ELF_Ident *>(addr);
	if (length < sizeof(ELF_Ident) || !ident->valid()) {
		cerr << "No valid ELF identification header!" << endl;
	} else if (!ident->data_supported()) {
		cerr << "Unsupported encoding (must be " << ELF_Ident::data_host() << ")!" << endl;
	} else {
		switch (ident->elfclass()) {
			case ELFCLASS::ELFCLASS32:
				success = setinterp<ELFCLASS::ELFCLASS64>(addr, length, argv[2]);
				break;

			case ELFCLASS::ELFCLASS64:
				success = setinterp<ELFCLASS::ELFCLASS64>(addr, length, argv[2]);
				break;

			default:
				cerr << "Unsupported class '" << ident->elfclass() << "'" << endl;
				success = false;
		}
	}

	// Cleanup
	::munmap(addr, length);
	::close(fd);
	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
