#include "read_input_functions.h"

std::string ReadLine() {
    using namespace std;

    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    using namespace std;

    int result;
    cin >> result;
    ReadLine();
    return result;
}
