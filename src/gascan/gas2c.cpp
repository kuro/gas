
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

inline void print_char (int output, char c)
{
    if (isprint(c) && ! isspace(c)) {
        dprintf(output, "%c", c);
    } else {
        dprintf(output, ".");
    }
}

int gas2c (int argc, char **argv)
{
    GASnum bytes_remaining;
    int input, output;
    string varname = "gas";
    GASubyte buf[8];
    GASubyte size_buf[32];
    int bytes_read;
    GASunum size;

    input = 0;
    output = 1;

    switch (argc) {
    case 2:
        // use defaults, and only varname
        break;
    case 3:
        if (string(argv[2]) != "-") {
            input = open(argv[2], O_RDONLY);
        }
        break;
    case 4:
        if (string(argv[2]) != "-") {
            input = open(argv[2], O_RDONLY);
        }
        if (string(argv[3]) != "-") {
            output = open(argv[3], O_WRONLY|O_CREAT, 0644);
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

    size = gas_read_encoded_num_fd(input);
    bytes_remaining = size;

    dprintf(output, "unsigned char %s[] = {\n", varname.c_str());

/* write the first line, which contains the number {{{*/
    // write out the number
    GASunum size_size = gas_write_encoded_num_buf(size_buf, size);
    bytes_remaining -= 8 - size_size;

    // just the number
    dprintf(output, "    ");
    for (unsigned int i = 0; i < size_size; i++) {
        dprintf(output, "0x%02x, ", size_buf[i]);
    }

    // remaining bytes on number line (char data)
    bytes_read = read(input, buf, 8 - size_size);
    for (unsigned int i = 0; i < 8 - size_size; i++) {
        dprintf(output, "0x%02x, ", buf[i]);
    }

    dprintf(output, "  /* ");
    // just the number
    for (unsigned int i = 0; i < size_size; i++) {
        print_char(output, size_buf[i]);
    }
    // remaining bytes on number line (char data)
    for (unsigned int i = 0; i < 8 - size_size; i++) {
        print_char(output, buf[i]);
    }
    dprintf(output, " */\n");
/*}}}*/
/* write remaining complete lines {{{*/
    while (bytes_remaining > 8) {
        bytes_remaining -= 8;

        bytes_read = read(input, buf, 8);
        dprintf(output, "    ");
        for (int i = 0; i < 8; i++) {
            dprintf(output, "0x%02x, ", buf[i]);
        }
        dprintf(output, "  /* ");
        for (int i = 0; i < 8; i++) {
            print_char(output, buf[i]);
        }
        dprintf(output, " */\n");
    }
/*}}}*/
/* write final line {{{*/
    GASnum i;
    dprintf(output, "    ");
    read(input, buf, bytes_remaining);
    for (i = 0; i < bytes_remaining - 1; i++) {
        dprintf(output, "0x%02x, ", buf[i]);
    }
    dprintf(output, "0x%02x  ", buf[i]);
    for (i = 0; i < 8 - bytes_remaining; i++) {
        dprintf(output, "      ");
    }
    dprintf(output, "  /* ");
    for (i = 0; i < bytes_remaining; i++) {
        print_char(output, buf[i]);
    }
    dprintf(output, " */\n");
/*}}}*/

    // footer
    dprintf(output, "};\n");

    // cleanup
    if (input == 0) {
        close(input);
    }
    if (output == 1) {
        close(output);
    }

    return 0;
}

// vim: sw=4 fdm=marker
