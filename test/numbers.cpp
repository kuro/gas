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
 * @file numbers.c
 * @brief numbers implementation
 */

#include <gas/fsio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>

int successes = 0;
int failures = 0;

int tryit (FILE* fs, GASunum num)
{
    GASunum out;
    //printf("%lx\n", num);

    rewind(fs);
    gas_write_encoded_num_fs(fs, num);

    rewind(fs);
    gas_read_encoded_num_fs(fs, &out);

    if (num == out) {
        successes++;
    } else {
        printf("input = 0x%lx\n", num);
        printf("output= 0x%lx\n", out);
        failures++;
    }

    return num == out;
}

void test_range (GASunum start, GASunum end)
{
    FILE* fs;
    GASunum i, j;

    char *fname = "/dev/shm/dump";
    fs = fopen(fname, "w+");

    for (i = start; i <= end; i++) {
        rewind(fs);
        gas_write_encoded_num_fs(fs, i);
        //printf("i=%lx\n", i);

        rewind(fs);
        gas_read_encoded_num_fs(fs, &j);
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

    fclose(fs);
    unlink(fname);
}

void test_random (int count, GASunum mask)
{
    int i;
    FILE *fs, *fs_rand;
    GASunum num;
    char *fname;

    fname = "/dev/shm/dump";
    fs = fopen(fname, "w+");


    fs_rand = fopen("/dev/urandom", "r");

    for (i = 0; i < count; i++) {
        fread(&num, sizeof(num), 1, fs_rand);
        num &= mask;

        if ( ! tryit(fs, num)) {
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

    fclose(fs);
    unlink(fname);
}

void test_number (GASunum num)
{
    FILE* fs;
    char *fname;

    fname = "/dev/shm/dump";
    fs = fopen(fname, "w+");

    if ( ! tryit(fs, num)) {
        char cmd[1024];
        cmd[0] = '\0';
        strcat(cmd, "xxd ");
        strcat(cmd, fname);
        printf("error: 0x%lx != 0x%lx\n", num, -1L);
        fflush(stdout);
        system(cmd);
        //exit(1);
    }

    fclose(fs);
    unlink(fname);
}

int numbers (int argc, char** argv)
{
    //int i;

    puts("testing specific cases");
    //test_number(0x32);
    //test_number(0x4632);
    //test_number(0xf24632);
    test_number(0x35f24632);
#if SIZEOF_VOID_P >= 8
    test_number(0xe9a2952240160c);
#endif

//    puts("testing given ranges");
//    test_range(0, 0xffffffff);
//    test_range(0x6ffffff, 0xfffffffff / 4);
//    test_range(0xfffffffffffffff0, 0xffffffffffffffff);

    puts("testing random masked numbers");
    test_random(100, 0xffffffff);
    puts("testing random masked numbers");
    test_random(100, 0x00000000);
    test_random(100, 0x00000000);
    test_random(100, 0x00000000);
    test_random(100, 0x00000000);
    test_random(100, 0xff000000);
    test_random(100, 0x00ff0000);
    test_random(100, 0x0000ff00);
    test_random(100, 0x000000ff);
    test_random(100, 0xffff0000);
    test_random(100, 0x0000ffff);
    test_random(100, 0x00000000);
    test_random(100, 0xffff0000);
    test_random(100, 0xffffffff);
#if SIZEOF_VOID_P >= 8
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
#endif

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
