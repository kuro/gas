
/**
 * @file io.c
 * @brief io implementation
 */

#include <gas/gas.h>
#include <gas/ntstring.h>

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
    gas_set_payload(root, sizeof(num), &num);


    chunk* message = gas_new_named("message");
    gas_set_attribute_ss(message, "reason", "because i can");
    gas_set_attribute_ss(message, "project", "GekkoWare");
    gas_set_payload(message, strlen("hello world"), "hello world");
    gas_add_child(root, message);

    chunk* media = gas_new_named("media");
    gas_add_child(root, media);


    chunk* movie = gas_new_named("movie");
    gas_set_attribute_ss(movie, "title", "TMNT");
    gas_add_child(media, movie);


    int fd = open("dump.gas", O_WRONLY|O_TRUNC|O_CREAT, 0600);
    gas_update(root);
    gas_write_fd(root, fd);
    close(fd);
    system("xxd dump.gas");

    fd = open("dump.gas", O_RDONLY);
    chunk* root2 = gas_read_fd(fd);
    gas_print(root2);
    gas_destroy(root2);
    close(fd);

    gas_destroy(root);
}

void test0002 (void)
{
    chunk* root = gas_new_named("head");
    gas_set_attribute_ss(root, "foo", "bar");

    int fd = open("1attr.gas", O_WRONLY|O_TRUNC|O_CREAT, 0600);
    gas_update(root);
    gas_write_fd(root, fd);
    close(fd);
    system("xxd 1attr.gas");

    fd = open("1attr.gas", O_RDONLY);
    chunk* root2 = gas_read_fd(fd);
    gas_print(root2);
    gas_destroy(root2);
    close(fd);

    gas_destroy(root);
}

void test0003 (void)
{
    chunk* root = gas_new(0, NULL);

    int fd = open("empty.gas", O_WRONLY|O_TRUNC|O_CREAT, 0600);
    gas_update(root);
    gas_write_fd(root, fd);
    close(fd);
    system("xxd empty.gas");

    fd = open("empty.gas", O_RDONLY);
    chunk* root2 = gas_read_fd(fd);
    gas_print(root2);
    gas_destroy(root2);
    close(fd);

    gas_destroy(root);
}
void test0004 (void)
{
    chunk* root = gas_new(0, NULL);

    int fd = open("double.gas", O_WRONLY|O_TRUNC|O_CREAT, 0600);
    gas_update(root);
    gas_write_fd(root, fd);
    gas_write_fd(root, fd);
    close(fd);
    system("xxd double.gas");

    fd = open("double.gas", O_RDONLY);
    chunk* root3 = gas_read_fd(fd);
    gas_print(root3);
    gas_destroy(root3);
    chunk* root2 = gas_read_fd(fd);
    gas_print(root2);
    gas_destroy(root2);
    close(fd);

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
