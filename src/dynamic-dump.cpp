#include "dump.hpp"

int main(int argc, char * argv[]) {
	if (argc != 2)
		cerr << "Usage: " << argv[0] << " ELF-FILE" << endl;
	else if (dump(argv[1], false))
		return 0;
	return 1;
}
