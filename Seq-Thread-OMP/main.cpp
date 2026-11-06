#include <iostream>
#include <omp.h> // brew install libomp for Mac, toolchain will need to be updated if on CLION
#include <thread>
#include <string>
#include <sstream>
#include <iomanip>

#include "input.h"

using namespace std;


int main(int argc, char* argv[]) {
    input config;
    string err;

    if (!parse_input(cin, config, err)) {
        cerr << "STATUS: INVALID_INPUT (" << err << ")" << endl;
        return 1;
    }

    cout << "STATUS: OK" << endl;
    print_config(config);

    return 0;
}