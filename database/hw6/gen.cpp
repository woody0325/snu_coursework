#include <iostream>
#include <fstream>
#include <cstdlib>

using std::cout;
using std::endl;
using std::ofstream;
using std::ios;

int main(int argc, char* argv[]) {
	if (argc != 4) {
		cout << "usage: <output> <count> <range>" << endl;
		return 1;
	}
	int count = atoi(argv[2]);
	int range = atoi(argv[3]);
	int x;
	ofstream out(argv[1], ios::out | ios::binary);
	// count
	cout << count << endl;
	out.write((char*) &count, sizeof(int));
	for (int i = 0; i < count; ++i) {
		x = std::rand() % range;
		// value
		cout << x << endl;
		out.write((char*) &x, sizeof(int));
	}
	out.close();
	return 0;
}

