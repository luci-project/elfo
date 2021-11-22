#include <iostream>
#include <string>

using namespace std;

string text = "The Answer to the Ultimate Question of Life, the Universe, and Everything is";

int answer() {
	return 42;
}

int main() {
	cout << text << ' ' << answer() << endl;
	return 0;
}
