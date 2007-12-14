
/**
 * @file test_tree.c
 * @brief test_tree implementation
 */

#include <gas/gas.h>
#include <gas/ntstring.h>

#include <stdio.h>

#include <string.h>
#include <linux/types.h>

void gas_print (chunk* c);

void test001 (void)
{
    chunk* root = gas_new_named("header");
    gas_set_attribute_ss(root, "class", "samurai");
    gas_set_attribute_ss(root, "checksum", "11235");

    uint32_t num = 0x00636261;
    gas_set_payload(root, sizeof(num), &num);
    gas_set_attribute_s(root, "num", sizeof(num), &num);


    chunk* message = gas_new_named("message");
    gas_set_attribute_ss(message, "reason", "because i can");
    gas_set_attribute_ss(message, "project", "GekkoWare");
    gas_set_payload(message, strlen("hello world"), "hello world");

    gas_add_child(root, message);

    gas_update(root);
    gas_print(root);

    GASunum len;
    uint32_t num_out;
    char c[1024];
    char *c_out;
    memset(c, 0, sizeof(c));
    c[64] = 0;
    gas_get_attribute_s(root, "class", &len, 0, sizeof(c));
    printf("class was %ld bytes and \"%s\"\n", len, c);
    c_out = gas_get_attribute_ss(root, "class");
    printf("class was again %ld bytes and \"%s\"\n", len, c_out);
    free(c_out);
    gas_get_attribute_s(root, "num", &num_out, 0, sizeof(num_out));
    printf("num (0x%x) was 0x%x\n", num, num_out);

    gas_destroy(root);
}

int main (void)
{
    test001();

    return 0;
}

// vim: sw=4 fdm=marker
