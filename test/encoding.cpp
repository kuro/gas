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
 * @file encoding.c
 */

#include "encoding.moc"

#include <QtTest>

#include <gas/tree.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int success_count;
int failure_count;

void write_size (int fd, GASunum val)
{
    GASubyte byte;
    GASunum i;
    //int full_bytes_needed;

    printf("val: %lx\n", val);

    byte = val;
    write(fd, &byte, 1);


    for (i = (sizeof(GASunum) << 3) - 1; i >= 0; i--) {
        if (val & (1L << i)) {
            break;
        }
    }
    //full_bytes_needed = (i+1) / 8;
    printf("\t\t\ti: %ld\n", i);

    //printf("full_bytes_needed: %d\n", full_bytes_needed);
}

GASunum read_size (int fd)
{
    GASunum retval;
    GASunum i, bytes_read, zero_byte_count, first_bit_set;
    GASubyte byte, mask = 0x00;

    // find first non 0x00 byte
    for (zero_byte_count = 0; 1; zero_byte_count++) {
        bytes_read = read(fd, &byte, 1);
        if (bytes_read != 1) {
            fprintf(stderr, "error: %s\n", strerror(errno));
            abort();
        }
        if (byte != 0x00)
            break;
    }

    // process initial byte
    for (first_bit_set = 7; first_bit_set >= 0; first_bit_set--)
        if (byte & (1L << first_bit_set))
            break;
    assert(first_bit_set > 0);

    for (i = 0; i < first_bit_set; i++)
        mask |= (1L << i);

    GASunum additional_bytes_to_read = (7-first_bit_set) + (7*zero_byte_count);

    // at this point, i have enough information to construct retval
    retval = mask & byte;
    for (i = 0; i < additional_bytes_to_read; i++) {
        bytes_read = read(fd, &byte, 1);
        if (bytes_read != 1) {
            fprintf(stderr, "error: %s\n", strerror(errno));
            abort();
        }
        retval = (retval << 8) | byte;
    }

    return retval;
}

int test_size_read(const char *fname, GASunum expected_value)
{
    int fd;
    GASunum size;

    printf("***\ntesting size read: %s\n", fname);

    fd = open(fname, O_RDONLY);
    size = read_size(fd);

    printf("%s > %lu\n",   fname, size);
    printf("result: 0x%lx\n", size);

    // checks
    int success = 1;
    if (size != expected_value) {
        printf("failed because size was not as expected\n");
        success = 0;
    }
    char buf[1024];
    int extra_bytes = read(fd, buf, sizeof(buf));
    if (extra_bytes != 0) {
        printf("failed because there were at least %d extra bytes\n",
               extra_bytes);
        success = 0;
    }
    if (success) {
        printf("success\n");
        success_count++;
    } else {
        printf("failure\n");
        failure_count++;
    }

    close(fd);

    return 1;
}

void test_size_write(const char* fname, GASunum val)
{
    printf("***\ntesting size write: %lx store to %s\n", val, fname);

    int fd;
    fd = open(fname, O_RDWR|O_CREAT|O_TRUNC, 0600);
    write_size(fd, val);
    close(fd);
    fflush(stdout);
    fflush(stderr);

    char cmd[1024];
    memset(cmd, 0, sizeof(cmd));
    strncat(cmd, "xxd ", sizeof(cmd));
    strncat(cmd, fname, sizeof(cmd));
    system(cmd);
}

int TestEncoding::encoding ()
{
//    test_size_read("2-111.bin", 111);
//    test_size_read("2-1023.bin", 1023);
//    test_size_read("0xfefefefe.bin", 0xfefefefe);
//    test_size_read("0xffffffffffffffff.bin", 0xffffffffffffffff);

//    test_size_write("out-0x1.bin", 0x1);
//    test_size_write("out-0x40.bin", 0x40);
//    test_size_write("out-0x80.bin", 0x80);
//    test_size_write("out-0xffffffff.bin", 0xffffffff);
//    test_size_write("out-0xffffffffffffffff.bin", 0xffffffffffffffff);

    printf("\nfailure rate: %g%%\n",
           ((float)failure_count)/(success_count+failure_count));

    return 0;
}


int encoding (int argc, char** argv)
{
    TestEncoding tc;
    return QTest::qExec(&tc, argc, argv);
}
