#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <string>
#include <exception>

using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ios;
using std::sort;

int* readAndSort(char* input, int& len) {
	ifstream in(input, ios::in | ios::binary);
	if (!in.is_open()) {
		cout << "ERROR: failed to open input file '" << input  << "'" << endl;
		return NULL;
	}
	in.read((char*) &len, sizeof(int));
	int* data = new int[len];
	in.read((char*) data, sizeof(int) * len);
	in.close();
	cout << "read " << len << endl;
	sort(data, data + len);
	return data;
}

void smj(int* dataR, int lenR, int* dataS, int lenS, char* output) {
	ofstream out(output, ios::out | ios::binary);
	// TODO: IMPLEMENT HERE
	// ex) out.write((char*) (dataR + i), sizeof(int));
	// ex) out.write((char*) (dataS + j), sizeof(int));
	while(1){
		if (lenR == 0 && lenS == 0) break;
		else if (lenR == 0){
			cout << *dataS << endl;
			out.write((char*) dataS, sizeof(int));
			dataS++;
			lenS--;
		}
		else if (lenS == 0){
			cout << *dataR << endl;
			out.write((char*) dataR, sizeof(int));
			dataR++;
			lenR--;
		}
		else if (*dataR <= *dataS){
			cout << *dataR << endl;
			out.write((char*) dataR, sizeof(int));
			dataR++;
			lenR--;
		}
		else if (*dataS < *dataR){
			cout << *dataS << endl;
			out.write((char*) dataS, sizeof(int));
			dataS++;
			lenS--;
		}
		else cout << "unexpected case" << endl;
	}
	out.close();
}

void sortMergeJoin(char* tableR, char* tableS, char* output) {
	int lenR, lenS;
	int* dataR = readAndSort(tableR, lenR);
	if (NULL == dataR) return;
	int* dataS = readAndSort(tableS, lenS);
	if (NULL == dataS) return;
	smj(dataR, lenR, dataS, lenS, output);
	delete[] dataR;
	delete[] dataS;
}

int main(int argc, char* argv[]) {
	if (argc != 4) {
		cout << "usage: <R> <S> <output>" << endl;
		return 1;
	}
	sortMergeJoin(argv[1], argv[2], argv[3]);
	return 0;
}
