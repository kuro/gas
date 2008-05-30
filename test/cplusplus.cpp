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

#include  <QtTest>
#include  <gas/ntstring.h>
#include "cplusplus.moc"

void TestCPlusPlusIO::append_child_op_001 ()
{
    Gas::Chunk *c = new Gas::Chunk("root");
    *c << new Gas::Chunk("child 0");
    gas_print(c);
    delete c;
}


void TestCPlusPlusIO::test_001 ()
{
    char str0[] = "str0";
    char *str1 = strdup("str1");

    Gas::Chunk *c = new Gas::Chunk("root");

    c->set_attribute("ten", 10);
    c->set_attribute("ten long", 10L);
    c->set_attribute("value is string", "hello world");
    c->set_attribute("str0", str0);
    c->set_attribute("str1", str1);

    gas_print(c);

    int my_int = 123;
    c->get_attribute("ten", my_int);
    QCOMPARE(my_int, 10);

    try {
        my_int = 123;
        fprintf(stderr, "the following should produce a debug warning\n");
        c->get_attribute("ten long", my_int);
        QFAIL("the previous get_attribute() should have failed");
    }
    catch (Gas::Exception& e)
    {
        qDebug() << "error successfully caught: " << e.what();
    }


    long my_long = 123;
    c->get_attribute("ten long", my_long);
    QCOMPARE(my_long, 10l);

    free(str1);
    delete c;
}


void TestCPlusPlusIO::test_has_attribute ()
{
    Gas::Chunk *c = new Gas::Chunk("root");
    c->set_attribute("fun", "FUN");
    QVERIFY(c->has_attribute("fun"));
    QVERIFY( ! c->has_attribute("sun"));
    delete c;
}


int cplusplus (int argc, char **argv)
{
    TestCPlusPlusIO tc;
    return QTest::qExec(&tc, argc, argv);
}
