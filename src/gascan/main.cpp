
/**
 * @file main.cpp
 * @brief main implementation
 */

#include <gas.h>

#include <stdio.h>
#include <fcntl.h>

#include <string>
#include <iostream>

using namespace std;

extern "C"
{
void gas_print (chunk* c);
}

#if HAVE_EXPAT
int xml2gas (string input, string output, bool verbose);
#endif

void print_gas_file (string fname)
{
    int fd;
    chunk *c;

    fd = open(fname.c_str(), O_RDONLY);
    c = gas_read_fd(fd);
    close(fd);

    gas_print(c);

    gas_destroy(c);
}

void die (string msg)
{
    cerr << msg << endl;
    exit(1);
}

int main (int argc, char **argv)
{
    if (argc < 2) {
        die("invalid usage");
    }

    string cmd = argv[1];
    if (cmd == "print") {
        if (argc != 3) {
            die("file not given");
        }
        print_gas_file(argv[2]);
#if HAVE_EXPAT
    } else if (cmd == "xml2gas") {
        if (argc != 4) {
            die("input and output not given");
        }
        xml2gas(argv[2], argv[3], false);
#endif
    } else {
        die("invalid command");
    }
    return 0;
}

// vim: sw=4 fdm=marker
