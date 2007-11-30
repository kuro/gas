
/**
 * @file numbers.c
 * @brief numbers implementation
 */

#include <gas/gas.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define _GNU_SOURCE
#include <stdio.h>

int successes = 0, failures = 0;

int try (int fd, GASunum num)
{
    GASunum out;

    lseek(fd, 0, SEEK_SET);
    gas_write_encoded_num_fd(fd, num);

    lseek(fd, 0, SEEK_SET);
    out = gas_read_encoded_num_fd(fd);

    if (num == out) {
        successes++;
    } else {
        printf("input = 0x%lx\n", num);
        printf("output= 0x%lx\n", out);
        failures++;
    }

    return num == out;
}

void test_range (int start, int end)
{
    int fd;
    GASunum i, j;

    char *fname = "/dev/shm/dump";
    fd = open(fname, O_RDWR|O_CREAT|O_TRUNC, 0600);

    for (i = start; i <= end; i++) {
        lseek(fd, 0, SEEK_SET);
        gas_write_encoded_num_fd(fd, i);
        //printf("i=%lx\n", i);

        lseek(fd, 0, SEEK_SET);
        j = gas_read_encoded_num_fd(fd);
        //printf("j=%lx\n", j);

        if (i != j) {
            char cmd[1024];
            cmd[0] = '\0';
            strcat(cmd, "xxd ");
            strcat(cmd, fname);
            printf("error: 0x%lx != 0x%lx\n", i, j);
            system(cmd);
            //exit(1);
        }
    }

    close(fd);
    unlink(fname);
}

void test_random (int count, GASunum mask)
{
    int i, fd, fd_rand;
    GASunum num;
    char *fname;

    fname = "/dev/shm/dump";
    fd = open(fname, O_RDWR|O_CREAT|O_TRUNC, 0600);


    fd_rand = open("/dev/urandom", O_RDONLY);

    for (i = 0; i < count; i++) {
        read(fd_rand, &num, sizeof(num));
        num &= mask;

        if ( ! try(fd, num)) {
            char cmd[1024];
            cmd[0] = '\0';
            strcat(cmd, "xxd ");
            strcat(cmd, fname);
            printf("error: 0x%lx != 0x%lx\n", num, -1L);
            fflush(stdout);
            system(cmd);
            //exit(1);
        }
    }

    close(fd);
    unlink(fname);
}

void test_number (GASunum num)
{
    int fd;
    char *fname;

    fname = "/dev/shm/dump";
    fd = open(fname, O_RDWR|O_CREAT|O_TRUNC, 0600);

    if ( ! try(fd, num)) {
        char cmd[1024];
        cmd[0] = '\0';
        strcat(cmd, "xxd ");
        strcat(cmd, fname);
        printf("error: 0x%lx != 0x%lx\n", num, -1L);
        fflush(stdout);
        system(cmd);
        //exit(1);
    }

    close(fd);
    unlink(fname);
}

int main (void)
{
    //int i;

    puts("testing specific cases");
    test_number(0xe9a2952240160c);

//    puts("testing given ranges");
//    test_range(0, 0xffffffff);
//    test_range(0x6ffffff, 0xfffffffff / 4);
//    test_range(0xfffffffffffffff0, 0xffffffffffffffff);

    puts("testing random masked numbers");
    test_random(10000, 0xffffffffffffffff);
    test_random(100, 0xff00000000000000);
    test_random(100, 0x00ff000000000000);
    test_random(100, 0x0000ff0000000000);
    test_random(100, 0x000000ff00000000);
    test_random(100, 0x00000000ff000000);
    test_random(100, 0x0000000000ff0000);
    test_random(100, 0x000000000000ff00);
    test_random(100, 0x00000000000000ff);
    test_random(100, 0xffff000000000000);
    test_random(100, 0x0000ffff00000000);
    test_random(100, 0x00000000ffff0000);
    test_random(100, 0x000000000000ffff);
    test_random(100, 0xffffffff00000000);
    test_random(100, 0x0000ffffffff0000);
    test_random(100, 0x00000000ffffffff);

//    for (i = 0; i < 10; i++) {
//        int x = rand() % sizeof(GASunum);
//        printf("%lx\n", 0xffL << (x*8));
//    }

#if 0
    puts("this range will not finish any time soon");
    //test_range(0, 0xffffffffffffffff);
    {
        GASunum num = 0;
        for (i = 1; i < sizeof(GASunum); i++) {
            memset(&num, 0xff, i);
            //printf("%lx\n", num);
            printf("%ld %ld\n", 0x1L << ((i-1)*8L), num);
            test_range(0x1L << ((i-1)*8L), num);
        }
    }
#endif

    return failures == 0 ? 0 : 1;
}

// vim: sw=4 fdm=marker
