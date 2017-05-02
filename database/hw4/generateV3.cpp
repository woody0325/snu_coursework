#include <iostream>
#include <fstream>
#include <cstdlib>

#define PAGE_SIZE 100
#define RUN 3

using std::cout;
using std::endl;
using std::ofstream;
using std::ios;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "usage: <output>" << endl;
        return 1;
    }
    int len = PAGE_SIZE << RUN;

    ofstream out(argv[1], ios::out | ios::binary);
    for (int i = 0; i < len; ++i) {
        cout << i << endl;
        out.write((char*) &i, sizeof(int));
    }
    out.close();

    return 0;
}
