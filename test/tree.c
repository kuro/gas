
/**
 * @file test_tree.c
 * @brief test_tree implementation
 */

#include <gas.h>

#include <string.h>

void gas_print (chunk* c);

void test001 (void)
{
    chunk* root = gas_new_named("header");
    gas_set_attribute_string_pair(root, "class", "samurai");
    gas_set_attribute_string_pair(root, "checksum", "11235");

    uint32_t num = 0x00636261;
    gas_set_payload(root, sizeof(num), &num);


    chunk* message = gas_new_named("message");
    gas_set_attribute_string_pair(message, "reason", "because i can");
    gas_set_attribute_string_pair(message, "project", "GekkoWare");
    gas_set_payload(message, strlen("hello world"), "hello world");

    gas_add_child(root, message);

    gas_update(root);
    gas_print(root);

    gas_destroy(root);
}

int main (void)
{
    test001();

    return 0;
}

// vim: sw=4 fdm=marker
