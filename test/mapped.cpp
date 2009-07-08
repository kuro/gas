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
#include "mapped.moc"

void TestMapped::non_mapped ()
{
    QFile file ("test.gas");
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    GASchunk* root = NULL;

    QBENCHMARK
    {
        gas_read_buf((GASubyte*)data.data(), data.size(), &root);
        gas_destroy(root);
    }
#if GAS_DEBUG_MEMORY
    QCOMPARE(gas_memory_usage(), 0ul);
#endif
}

void TestMapped::mapped_tree ()
{
    QFile file ("test.gas");
    file.open(QIODevice::ReadOnly);
    uchar* data = file.map(0, file.size());
    GASchunk* root = NULL;

    QBENCHMARK
    {
        gas_read_buf(data, file.size(), &root);
        gas_destroy(root);
    }
#if GAS_DEBUG_MEMORY
    QCOMPARE(gas_memory_usage(), 0ul);
#endif
}

void TestMapped::mapped_treen ()
{
    QFile file ("test.gas");
    file.open(QIODevice::ReadOnly);
    uchar* data = file.map(0, file.size());
    GASchunk* root = NULL;

    QBENCHMARK
    {
        gas_read_bufn(data, file.size(), &root);
        gas_destroyn(root);
    }
#if GAS_DEBUG_MEMORY
    QCOMPARE(gas_memory_usage(), 0ul);
#endif
}





int mapped (int argc, char **argv)
{
    QCoreApplication app (argc, argv);
    TestMapped tc;
    return QTest::qExec(&tc, argc, argv);
}
