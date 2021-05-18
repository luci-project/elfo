
#include "elf.hpp"

#include <sys/auxv.h>

#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char * argv[]) {
	if (argc != 2) {
		cerr << "Usage: " << argv[0] << " OUTPUT-FILE" << endl;
		return 1;
	}

	auto vdso = reinterpret_cast<uintptr_t>(getauxval(AT_SYSINFO_EHDR));
	auto size = Elf(vdso).size();

	std::ofstream(argv[1], ios::binary).write(reinterpret_cast<char*>(vdso), size);

	return 0;
}
