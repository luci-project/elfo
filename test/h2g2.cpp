// Elfo - a lightweight parser for the Executable and Linking Format
// Copyright 2021-2023 by Bernhard Heinloth <heinloth@cs.fau.de>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <iostream>
#include <string>

static const char * text = "The Answer to the Ultimate Question of Life";
__thread int value = 42;

int answer() {
	return value;
}

int main() {
	std::string str(text);
	str += ", the Universe, and Everything is";
	std::cout << str << ' ' << answer() << std::endl;
	return 0;
}
