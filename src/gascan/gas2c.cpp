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
 * @file gas2c.cpp
 * @brief gas2c implementation
 */

#include <gas/fdio.h>
#include <gas/bufio.h>

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <string>
#include <iostream>

using namespace std;

void die (string msg);

extern "C"
{
GASunum encoded_size (GASunum value);
}

inline void print_char (FILE *output, char c)
{
    if (isprint(c) && ! isspace(c)) {
        fprintf(output, "%c", c);
    } else {
        fprintf(output, ".");
    }
}

int gas2c (int argc, char **argv)
{
    GASnum bytes_remaining;
    FILE *input, *output;
    string varname = "gas";
    GASubyte buf[8];
    GASubyte size_buf[32];
    int bytes_read;
    GASunum size;

    input = stdin;
    output = stdout;

    switch (argc) {
    case 2:
        // use defaults, and only varname
        break;
    case 3:
        if (string(argv[2]) != "-") {
            input = fopen(argv[2], "r");
        }
        break;
    case 4:
        if (string(argv[2]) != "-") {
            input = fopen(argv[2], "r");
        }
        if (string(argv[3]) != "-") {
            output = fopen(argv[3], "w");
        }
        break;
    default:
        fprintf(stderr,
                "%s: convert gas to C byte array       \n"
                "usage: gascan %s VARNAME              \n"
                "  OR   gascan %s VARNAME INTPUT       \n"
                "  OR   gascan %s VARNAME INTPUT OUTPUT\n"
                , argv[0], argv[0], argv[0], argv[0]
                );
        return 1;
    }

    varname = argv[1];

    size = gas_read_encoded_num_fd(fileno(input));
    bytes_remaining = size;

    fprintf(output, "unsigned char %s[] = {\n", varname.c_str());

/* write the first line, which contains the number {{{*/
    // write out the number
    GASunum size_size = gas_write_encoded_num_buf(size_buf, size);
    bytes_remaining -= 8 - size_size;

    // just the number
    fprintf(output, "    ");
    for (unsigned int i = 0; i < size_size; i++) {
        fprintf(output, "0x%02x, ", size_buf[i]);
    }

    // remaining bytes on number line (char data)
    //bytes_read = fread(input, buf, 8 - size_size);
    bytes_read = fread(buf, 8 - size_size, 1, input);
    for (unsigned int i = 0; i < 8 - size_size; i++) {
        fprintf(output, "0x%02x, ", buf[i]);
    }

    fprintf(output, "  /* ");
    // just the number
    for (unsigned int i = 0; i < size_size; i++) {
        print_char(output, size_buf[i]);
    }
    // remaining bytes on number line (char data)
    for (unsigned int i = 0; i < 8 - size_size; i++) {
        print_char(output, buf[i]);
    }
    fprintf(output, " */\n");
/*}}}*/
/* write remaining complete lines {{{*/
    while (bytes_remaining > 8) {
        bytes_remaining -= 8;

        //bytes_read = read(input, buf, 8);
        bytes_read = fread(buf, 8, 1, input);
        fprintf(output, "    ");
        for (int i = 0; i < 8; i++) {
            fprintf(output, "0x%02x, ", buf[i]);
        }
        fprintf(output, "  /* ");
        for (int i = 0; i < 8; i++) {
            print_char(output, buf[i]);
        }
        fprintf(output, " */\n");
    }
/*}}}*/
/* write final line {{{*/
    GASnum i;
    fprintf(output, "    ");
    //read(input, buf, bytes_remaining);
    fread(buf, bytes_remaining, 1, input);
    for (i = 0; i < bytes_remaining - 1; i++) {
        fprintf(output, "0x%02x, ", buf[i]);
    }
    fprintf(output, "0x%02x  ", buf[i]);
    for (i = 0; i < 8 - bytes_remaining; i++) {
        fprintf(output, "      ");
    }
    fprintf(output, "  /* ");
    for (i = 0; i < bytes_remaining; i++) {
        print_char(output, buf[i]);
    }
    fprintf(output, " */\n");
/*}}}*/

    // footer
    fprintf(output, "};\n");

    // cleanup
    fclose(input);
    fclose(output);

    return 0;
}

// vim: sw=4 fdm=marker
