
/**
 * @file main.cpp
 * @brief main implementation
 */

#include <gas/fdio.h>

#include <stdio.h>
#include <fcntl.h>

#include <string>
#include <iostream>

using namespace std;

extern "C"
{
void gas_print (chunk* c);
}

int test_main (int argc, char **argv);

#if HAVE_QT4
int xml2gas_main (int argc, char **argv);
int qtedit_main (int argc, char **argv);
#endif


int gas2c (int argc, char **argv);

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
        die("invalid usage: command not given");
    }

    string cmd = argv[1];
    if (cmd == "print") {
        if (argc != 3) {
            die("file not given");
        }
        print_gas_file(argv[2]);
    } else if (cmd == "test") {
        test_main(argc-1, &argv[1]);
#if HAVE_QT4
    } else if (cmd == "xml2gas") {
        xml2gas_main(argc-1, &argv[1]);
    } else if (cmd == "edit") {
        qtedit_main(argc-1, &argv[1]);
#endif
    } else if (cmd == "gas2c") {
        gas2c(argc-1, &argv[1]);
    } else {
        die("invalid command");
    }
    return 0;
}

// vim: sw=4 fdm=marker
