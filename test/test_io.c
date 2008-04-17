
/**
 * @file io.c
 * @brief io implementation
 */

#include <gas/fdio.h>
#include <gas/ntstring.h>
#include <gas/fsio.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
//#include <linux/types.h>

void gas_print (chunk* c);

void test0001 (void)
{
    chunk* root = gas_new_named("header");
    gas_set_attribute_ss(root, "class", "samurai");
    gas_set_attribute_ss(root, "checksum", "11235");

    uint32_t num = 0x00636261;
    gas_set_payload(root, &num, sizeof(num));


    chunk* message = gas_new_named("message");
    gas_set_attribute_ss(message, "reason", "because i can");
    gas_set_attribute_ss(message, "project", "GekkoWare");
    gas_set_payload(message, "hello world", strlen("hello world"));
    gas_add_child(root, message);

    chunk* media = gas_new_named("media");
    gas_add_child(root, media);


    chunk* movie = gas_new_named("movie");
    gas_set_attribute_ss(movie, "title", "TMNT");
    gas_add_child(media, movie);


    FILE* fs = fopen("dump.gas", "w");
    gas_update(root);
    gas_write_fs(fs, root);
    fclose(fs);
    system("xxd dump.gas");

    fs = fopen("dump.gas", "r");
    chunk* root2 = NULL;
    gas_read_fs(fs, &root2);
    gas_print(root2);
    gas_destroy(root2);
    fclose(fs);

    gas_destroy(root);
}

void test0002 (void)
{
    chunk* root = gas_new_named("head");
    gas_set_attribute_ss(root, "foo", "bar");

    FILE *fs = fopen("1attr.gas", "w");
    gas_update(root);
    gas_write_fs(fs, root);
    fclose(fs);
    system("xxd 1attr.gas");

    fs = fopen("1attr.gas", "r");
    chunk* root2 = NULL;
    gas_read_fs(fs, &root2);
    gas_print(root2);
    gas_destroy(root2);
    fclose(fs);

    gas_destroy(root);
}

void test0003 (void)
{
    chunk* root = gas_new(NULL, 0);

    FILE* fs = fopen("empty.gas", "w");
    gas_update(root);
    gas_write_fs(fs, root);
    fclose(fs);
    system("xxd empty.gas");

    fs = fopen("empty.gas", "r");
    chunk* root2 = NULL;
    gas_read_fs(fs, &root2);
    gas_print(root2);
    gas_destroy(root2);
    fclose(fs);

    gas_destroy(root);
}
void test0004 (void)
{
    chunk* root = gas_new(NULL, 0);

    FILE* fs = fopen("double.gas", "w");
    gas_update(root);
    gas_write_fs(fs, root);
    gas_write_fs(fs, root);
    fclose(fs);
    system("xxd double.gas");

    fs = fopen("double.gas", "r");
    chunk* root3 = NULL;
    gas_read_fs(fs, &root3);
    gas_print(root3);
    gas_destroy(root3);
    chunk* root2 = NULL;
    gas_read_fs(fs, &root2);
    gas_print(root2);
    gas_destroy(root2);
    fclose(fs);

    gas_destroy(root);
}


int main (void)
{
    //test0004();
    //test0003();
    //test0002();
    test0001();
    return 0;
}

// vim: sw=4 fdm=marker
