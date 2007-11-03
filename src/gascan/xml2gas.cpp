
/**
 * @file xml2gas.cpp
 * @brief xml2gas implementation
 */

#include <gas.h>
#include <expat.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>
#include <iostream>

using namespace std;
chunk* cur = NULL;

int depth;

int indent_i;
#define indent() for (indent_i = 0; indent_i < depth; indent_i++){printf("  ");}

extern "C"
{
void gas_print (chunk* c);
}

void start (void *data, const char *el, const char **attr)
{
    int i;

    //indent();
    //printf("%s", el);
    chunk* n = gas_new_named(el);

    for (i = 0; attr[i]; i += 2) {
        //printf(" %s='%s'", attr[i], attr[i+1]);
        gas_set_attribute_ss(n, attr[i], attr[i+1]);
    }

    //printf("\n");

    gas_add_child(cur, n);
    cur = n;

    depth++;
}

void character_data (void *data, const char *str, int length)
{
    int total_new_len = length + cur->payload_size;
    cur->payload = realloc(cur->payload, total_new_len + 1);
    memcpy(((GASubyte*)cur->payload) + cur->payload_size, str, length);
    ((char*)cur->payload)[total_new_len] = 0;
    cur->payload_size = total_new_len;
}

void end (void *data, const char *el)
{
    cur = cur->parent;
    depth--;
}


int xml2gas (string input, string output, bool verbose)
{
    cur = gas_new_named("root");

    //char *xml_data = "<a><b/><c/></a>";
    XML_Parser parser;
    parser = XML_ParserCreate(NULL);
    XML_SetElementHandler(parser, &start, &end);
    XML_SetCharacterDataHandler(parser, &character_data);

    char buf[1024];
    ssize_t bytes_read;
    //int fd = open("test.dae", O_RDONLY);
    int fd = open(input.c_str(), O_RDONLY);
    printf("fd: %d\n", fd);
    while ((bytes_read = read(fd, buf, sizeof(buf))) > 0) {
        //printf("bytes_read: %ld\n", bytes_read);
        XML_Parse(parser, buf, bytes_read, 0);
    }
    close(fd);

    XML_ParserFree(parser);

    gas_update(cur);
    if (verbose) {
        puts("printing gas");
        gas_print(cur);
    }
    puts("saving gas");
    fd = open(output.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0600);
    // i do not need to write the top root element,
    // so when reading the file back, there may be multiple top chunks
    unsigned int i;
    for (i = 0; i < cur->nb_children; i++) {
        printf("total size: %ld\n", gas_total_size(cur->children[i]));
        gas_write_fd(cur->children[i], fd);
    }
    close(fd);

    return 1;
}

// vim: sw=4 fdm=marker
