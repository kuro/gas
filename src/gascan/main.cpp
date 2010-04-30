/*
 * Copyright 2008 Blanton Black
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file main.cpp
 * @brief main implementation
 */

#include <gas/fsio.h>

#include <stdio.h>
#include <fcntl.h>

#include <string>
#include <iostream>

using namespace std;

int test_main (int argc, char **argv);

int xml2gas_main (int argc, char **argv);

#ifdef HAVE_QTGUI
int qtedit_main (int argc, char **argv);
#endif


int bin2c (int argc, char** argv);

void print_gas_file (string fname)
{
    FILE* fs;
    GASchunk *c = NULL;
    GASresult result;

    if (fname == "-") {
        fs = stdin;
    } else {
        fs = fopen(fname.c_str(), "r");
    }
    if (fs == NULL) {
        fprintf(stderr, "file not found\n");
        return;
    }
    while (!feof(fs)) {
        result = gas_read_fs(fs, &c);
        if (result != GAS_OK) {
            if (result != GAS_ERR_FILE_EOF) {
                // only print error if not eof
                fprintf(stderr, "gascan detected error while reading: %s\n",
                        gas_error_string(result));
            }
            if (c != NULL) {
                gas_destroy(c);
            }
            continue;
        }

        result = gas_print(c);
        if (result != GAS_OK) {
            fprintf(stderr, "gascan detected error while printing: %s\n",
                    gas_error_string(result));
        }

        gas_destroy(c);
        if (result != GAS_OK) {
            fprintf(stderr, "gascan detected error while destroying: %s\n",
                    gas_error_string(result));
        }

        c = NULL;
    }

    fclose(fs);
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
        if (argc == 2) {
            print_gas_file("-");
        } else {
            for (int i = 2; i < argc; i++) {
                if (argc > 3) {
                    printf("*** %s\n", argv[i]);
                }
                print_gas_file(argv[i]);
                if (argc > 3) {
                    printf("\n");
                }
            }
        }
    } else if (cmd == "test") {
        test_main(argc-1, &argv[1]);
    } else if (cmd == "xml2gas") {
        xml2gas_main(argc-1, &argv[1]);
#ifdef HAVE_QTGUI
    } else if (cmd == "edit") {
        qtedit_main(argc-1, &argv[1]);
#endif
    } else if (cmd == "bin2c") {
        bin2c(argc-1, &argv[1]);
    } else {
        die("invalid command");
    }
    return 0;
}

// vim: sw=4 fdm=marker
