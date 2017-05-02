#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <string>
#include <exception>

#define PAGE_SIZE 100
#define MEM_SIZE 2
#define BUF_SIZE (PAGE_SIZE * MEM_SIZE)

using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ios;
using std::sort;
using std::stringstream;

void mad() {
    throw std::exception();
}

void sortAndWrite(int* buf, int len, int fileID) {
    sort(buf, buf + len);
    stringstream ss1, ss2;
    ss1 << "output_binary/temp_0_" << fileID;
    ofstream out(ss1.str().c_str(), ios::out | ios:: binary);
    if (!out.is_open()) {
        cout << "failed to open output file" << endl;
        cout << "make sure that the directory \"output_binary/\" exists \n" << endl;
        mad();
    }
    out.write((char*) buf, sizeof(int) * len);
    out.close();

    ss2 << "output_debug/temp_0_" << fileID;
    ofstream out_debug(ss2.str().c_str(), ios::out);
    for (int j = 0; j < len; ++j){
        out_debug << buf[j] << endl;}
    out_debug.close();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "usage: <input>" << endl;
        return 1;
    }
    int fileID = 0;
    int* buf = new int[BUF_SIZE];
    ifstream in(argv[1], ios::in | ios::binary);
    if (!in.is_open()) {
        cout << "failed to open input file" << endl;
        mad();
    }

    while (in) {
        in.read((char*) buf, sizeof(int) * BUF_SIZE);
        int len = in.gcount() / sizeof(int);
        if (len > 0) {
            sortAndWrite(buf, len, fileID);
            ++fileID;
        }
    }
    // ASSUME that the input size is always PAGE_SIZE * 2^n
    // Then, the buffer is always full
    in.close();
    delete[] buf;

    int* buf1 = new int[PAGE_SIZE];
    int* buf2 = new int[PAGE_SIZE];
    int nRun = 0;

    // ASSUME that the input size is always PAGE_SIZE * 2^n
    // Then, the number of files is always 2^n
    for (int i = fileID; i > 1; i /= 2) {
        int j = 0; // input file index
        int c = 0; // output file index
        while (j < i) {
            stringstream ss1, ss2, ss3, ss4;
            ss1 << "output_binary/temp_" << (nRun + 1) << "_" << c;
            ss2 << "output_debug/temp_" << (nRun + 1) << "_" << (c++);
            ofstream out(ss1.str().c_str(), ios::out | ios::binary);
            ofstream out_debug(ss2.str().c_str(), ios::out);
            ss3 << "output_binary/temp_" << nRun << "_" << (j++);
            ss4 << "output_binary/temp_" << nRun << "_" << (j++);
            ifstream in1(ss3.str().c_str(), ios::in | ios::binary);
            ifstream in2(ss4.str().c_str(), ios::in | ios::binary);

            // TODO: merge-sort and output
            // USE: in1.read((char*) (buf1 + k), sizeof(int));
            // USE: out.write((char*) &value, sizeof(int));
            // USE: out_debug << value << endl;
            int len = PAGE_SIZE;
	    int num_page = (1 << (fileID / i)); // number of page in a single sorted storage for each run
	    in1.read((char*) buf1, sizeof(int) * PAGE_SIZE);
	    in2.read((char*) buf2, sizeof(int) * PAGE_SIZE);
	    int p1 = 1, p2 = 1; // page index
	    int i1 = 0, i2 = 0; // integer index
	    int f1 = 0, f2 = 0; // identificatin of EOF
	    while (1){
		if ((f1 == 0 && f2 == 0 && buf1[i1] <= buf2[i2]) || (f1 == 0 && f2 == 1)){
		    out.write((char*) &buf1[i1], sizeof(int));
		    out_debug << buf1[i1] << endl;
		    i1++;
		    if (i1 == len){
		        if (p1 < num_page){
			    in1.read((char*) buf1, sizeof(int) * PAGE_SIZE);
			    p1++;
			    i1 = 0;
			}
			else f1 = 1;
		    }
		}
		else if ((f1 == 0 && f2 == 0 && buf2[i2] < buf1[i1]) || (f1 == 1 && f2 == 0)){
		    out.write((char*) &buf2[i2], sizeof(int));
		    out_debug << buf2[i2] << endl;
		    i2++;
		    if (i2 == len){
		         if (p2 < num_page){
		             in2.read((char*) buf2, sizeof(int) * PAGE_SIZE);
		             p2++;
		             i2 = 0;
		         }
		         else f2 = 1;
		    }
		}
		else if (f1 == 1 && f2 == 1) break;
		else {
		    cout << "Error: unexpeected case";
		    break;
		}   
	    }

            in1.close();
            in2.close();
            out.close();
            out_debug.close();
        }
        ++nRun;
    }
    delete[] buf1;
    delete[] buf2;

    return 0;
}
