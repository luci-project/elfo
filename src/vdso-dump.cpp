#include <elfo/elf.hpp>

#ifdef USE_DLH
#include <dlh/stream/output.hpp>
#include <dlh/auxiliary.hpp>
#else
#include <sys/auxv.h>
#include <sys/types.h>
#include <iostream>
using namespace std;
#endif
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char * argv[]) {
	if (argc != 2) {
		cerr << "Usage: " << argv[0] << " OUTPUT-FILE" << endl;
		return 1;
	}

	errno = 0;
#ifdef USE_DLH
	auto vdso = Auxiliary::vector(Auxiliary::AT_SYSINFO_EHDR).value();
#else
	auto vdso = reinterpret_cast<uintptr_t>(getauxval(AT_SYSINFO_EHDR));
#endif

	auto size = Elf(vdso).size();
	auto file = ::open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (file < 0) {
		cerr << "Unable to open " << argv[1] << ": " << ::strerror(errno) << endl;
		return 1;
	}
	size_t written = 0;
	while (written < size) {
		ssize_t tmp = ::write(file, reinterpret_cast<void*>(vdso + written), size - written);
		if (tmp < 0) {
			cerr << "Unable to write to " << argv[1] << ": " << ::strerror(errno) << endl;
			return 1;
		} else {
			written += tmp;
		}
	}
	::close(file);

	return 0;
}
