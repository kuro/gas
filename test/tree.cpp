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
 * @file test_tree.c
 * @brief test_tree implementation
 */

#include <gas/gas.h>
#include <gas/ntstring.h>

#include <stdio.h>

#include <string.h>
//#include <linux/types.h>

void test001 (void)
{
    GASchunk* root = NULL;
    gas_new_named(&root, "header");
    gas_set_attribute_ss(root, "class", "samurai");
    gas_set_attribute_ss(root, "checksum", "11235");

    uint32_t num = 0x00636261;
    gas_set_payload(root, &num, sizeof(num));
    gas_set_attribute_s(root, "num", &num, sizeof(num));


    GASchunk* message = NULL;
    gas_new_named(&message, "message");
    gas_set_attribute_ss(message, "reason", "because i can");
    gas_set_attribute_ss(message, "project", "GekkoWare");
    gas_set_payload(message, "hello world", strlen("hello world"));

    gas_add_child(root, message);

    gas_update(root);
    gas_print(root);

    GASnum len;
    uint32_t num_out;
    char c[1024];
    char *c_out;
    memset(c, 0, sizeof(c));
    c[64] = 0;
    len = gas_get_attribute_s(root, "class", &c, sizeof(c));
    printf("class was %ld bytes and \"%s\"\n", len, c);
    c_out = gas_get_attribute_ss(root, "class");
    printf("class was again %ld bytes and \"%s\"\n", len, c_out);
    free(c_out);
    gas_get_attribute_s(root, "num", &num_out, sizeof(num_out));
    printf("num (0x%x) was 0x%x\n", num, num_out);

    gas_destroy(root);
}

void test002 (void)
{
    GASchunk* message = NULL;
    gas_new_named(&message, "message");
    gas_set_attribute_ss(message, "reason", "because i can");
    gas_set_attribute_ss(message, "project", "GekkoWare");
    gas_set_attribute_ss(message, "name", "Roger Smith");
    gas_set_attribute_ss(message, "verdict", "CAST IN THE NAME OF GOD YE NOT GUILTY");
    gas_set_payload(message, "hello world", strlen("hello world"));

    gas_print(message);
    printf("\n");

    gas_delete_attribute_at(message, 0);
    gas_print(message);
    printf("\n");

    gas_delete_attribute_at(message, 3);
    gas_print(message);
    printf("\n");

    gas_delete_attribute_at(message, 2);
    gas_print(message);
    printf("\n");

    gas_destroy(message);
}

void test003 (void)
{
    GASchunk* root = NULL;
    gas_new_named(&root, "root");

    GASchunk* tmp = NULL;
    gas_new_named(&tmp, "child0");  gas_add_child(root, tmp);
    gas_new_named(&tmp, "child1");  gas_add_child(root, tmp);
    gas_new_named(&tmp, "child2");  gas_add_child(root, tmp);
    gas_new_named(&tmp, "child3");  gas_add_child(root, tmp);
    gas_new_named(&tmp, "child4");  gas_add_child(root, tmp);
    gas_new_named(&tmp, "child5");  gas_add_child(root, tmp);
    gas_new_named(&tmp, "child6");  gas_add_child(root, tmp);
    gas_new_named(&tmp, "child7");  gas_add_child(root, tmp);

    gas_print(root);
    printf("\n");

    gas_delete_child_at(root, 0);
    gas_print(root);
    printf("\n");

    gas_delete_child_at(root, 2);
    gas_print(root);
    printf("\n");

    gas_delete_child_at(root, 5);
    gas_print(root);
    printf("\n");

    int status;
    do { 
        status = gas_delete_child_at(root, 0);
    } while (status == GAS_OK);
    gas_print(root);
    printf("\n");

    gas_destroy(root);
}


int tree (int argc, char** argv)
{
    test001();
    //test002();
    //test003();

    return 0;
}

// vim: sw=4 fdm=marker
