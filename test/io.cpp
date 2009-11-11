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
 * @file io.c
 * @brief io implementation
 */

#include "io.moc"

#include <QtTest>

#include <gas/fdio.h>
#include <gas/ntstring.h>
#include <gas/fsio.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
//#include <linux/types.h>

void TestIo::test0001 (void)
{
    GASchunk* root = NULL;

    gas_new_named(&root, "header");

    gas_set_attribute_ss(root, "class", "samurai");
    gas_set_attribute_ss(root, "checksum", "11235");

    uint32_t num = 0x00636261;
    gas_set_payload(root, &num, sizeof(num));


    GASchunk* message = NULL;
    gas_new_named(&message, "message");

    gas_set_attribute_ss(message, "reason", "because i can");
    gas_set_attribute_ss(message, "project", "GekkoWare");
    gas_set_payload(message, "hello world", strlen("hello world"));
    gas_add_child(root, message);

    GASchunk* media = NULL;
    gas_new_named(&media, "media");
    gas_add_child(root, media);


    GASchunk* movie = NULL;
    gas_new_named(&movie, "movie");
    gas_set_attribute_ss(movie, "title", "TMNT");
    gas_add_child(media, movie);


    FILE* fs = fopen("dump.gas", "w");
    gas_update(root);
    gas_write_fs(fs, root);
    fclose(fs);
    system("xxd dump.gas");

    fs = fopen("dump.gas", "r");
    GASchunk* root2 = NULL;
    gas_read_fs(fs, &root2);
    gas_print(root2);
    gas_destroy(root2);
    fclose(fs);

    gas_destroy(root);
}

void TestIo::test0002 (void)
{
    GASchunk* root = NULL;
    gas_new_named(&root, "head");
    gas_set_attribute_ss(root, "foo", "bar");

    FILE *fs = fopen("1attr.gas", "w");
    gas_update(root);
    gas_write_fs(fs, root);
    fclose(fs);
    system("xxd 1attr.gas");

    fs = fopen("1attr.gas", "r");
    GASchunk* root2 = NULL;
    gas_read_fs(fs, &root2);
    gas_print(root2);
    gas_destroy(root2);
    fclose(fs);

    gas_destroy(root);
}

void TestIo::test0003 (void)
{
    GASchunk* root = NULL;
    gas_new(&root, NULL, 0);

    FILE* fs = fopen("empty.gas", "w");
    gas_update(root);
    gas_write_fs(fs, root);
    fclose(fs);
    system("xxd empty.gas");

    fs = fopen("empty.gas", "r");
    GASchunk* root2 = NULL;
    gas_read_fs(fs, &root2);
    gas_print(root2);
    gas_destroy(root2);
    fclose(fs);

    gas_destroy(root);
}
void TestIo::test0004 (void)
{
    GASchunk* root = NULL;
    gas_new(&root, NULL, 0);

    FILE* fs = fopen("double.gas", "w");
    gas_update(root);
    gas_write_fs(fs, root);
    gas_write_fs(fs, root);
    fclose(fs);
    system("xxd double.gas");

    fs = fopen("double.gas", "r");
    GASchunk* root3 = NULL;
    gas_read_fs(fs, &root3);
    gas_print(root3);
    gas_destroy(root3);
    GASchunk* root2 = NULL;
    gas_read_fs(fs, &root2);
    gas_print(root2);
    gas_destroy(root2);
    fclose(fs);

    gas_destroy(root);
}


int io (int argc, char** argv)
{
    TestIo tc;
    return QTest::qExec(&tc, argc, argv);
}
