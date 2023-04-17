// Elfo - a lightweight parser for the Executable and Linking Format
// Copyright 2021-2023 by Bernhard Heinloth <heinloth@cs.fau.de>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "dump.hpp"

int main(int argc, char * argv[]) {
	if (argc != 2)
		cerr << "Usage: " << argv[0] << " ELF-FILE" << endl;
	else if (dump(argv[1], false))
		return 0;
	return 1;
}
