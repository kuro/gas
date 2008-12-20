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
#include "bufio.moc"

void TestBufIO::encode_0xdeadbeef ()
{
    GASubyte expected_data[] = { 0x08, 0xde, 0xad, 0xbe, 0xef };
    QByteArray expected (reinterpret_cast<char*>(expected_data),
                         sizeof(expected_data));
    GASnum result;
    result = gas_write_encoded_num_buf(buf, sizeof(buf), 0xdeadbeef);
    QCOMPARE(result, static_cast<GASnum>(sizeof(expected_data)));
    QCOMPARE(QByteArray(reinterpret_cast<char*>(buf), result), expected);
}

void TestBufIO::encode_0x7e ()
{
    GASubyte expected_data[] = { 0xfe };
    QByteArray expected (reinterpret_cast<char*>(expected_data),
                         sizeof(expected_data));
    GASnum result;
    result = gas_write_encoded_num_buf(buf, sizeof(buf), 0x7e);
    QCOMPARE(result, static_cast<GASnum>(sizeof(expected_data)));
    QCOMPARE(QByteArray(reinterpret_cast<char*>(buf), result), expected);
}

/**
 * @brief 0xff is reserved for future use, so 0x7f should be pushed to the second byte.
 */
void TestBufIO::encode_0x7f ()
{
    GASubyte expected_data[] = { 0x40, 0x7f };
    QByteArray expected (reinterpret_cast<char*>(expected_data),
                         sizeof(expected_data));
    GASnum result;
    result = gas_write_encoded_num_buf(buf, sizeof(buf), 0x7f);
    QCOMPARE(result, static_cast<GASnum>(sizeof(expected_data)));
    QCOMPARE(QByteArray(reinterpret_cast<char*>(buf), result), expected);
}

void TestBufIO::encode_failure_undersized_buffer_0xdeadbeef ()
{
    GASubyte expected_data[] = { 0x08, 0xde, 0xad, 0xbe, 0xef };
    QByteArray expected (reinterpret_cast<char*>(expected_data),
                         sizeof(expected_data));
    GASnum result;
    result = gas_write_encoded_num_buf(buf, sizeof(buf), 0xdeadbeef);
    QCOMPARE(result, static_cast<GASnum>(sizeof(expected_data)));
    QCOMPARE(QByteArray(reinterpret_cast<char*>(buf), result), expected);

    for (unsigned int i = 0; i < sizeof(expected_data); i++) {
        result = gas_write_encoded_num_buf(expected_data, i, 0xdeadbeef);
        QVERIFY(result < 0);
    }
}

void TestBufIO::encode_failure_undersized_buffer_0xdeadbeefdeadbeef ()
{
    GASubyte expected_data[] = { 0x08, 0xde, 0xad, 0xbe, 0xef };
    QByteArray expected (reinterpret_cast<char*>(expected_data),
                         sizeof(expected_data));
    GASnum result;
    result = gas_write_encoded_num_buf(buf, sizeof(buf), 0xdeadbeef);
    QCOMPARE(result, static_cast<GASnum>(sizeof(expected_data)));
    QCOMPARE(QByteArray(reinterpret_cast<char*>(buf), result), expected);

    for (unsigned int i = 0; i < sizeof(expected_data); i++) {
        result = gas_write_encoded_num_buf(expected_data, i, 0xdeadbeef);
        QVERIFY(result < 0);
    }
}

void TestBufIO::decode_0xdeadbeef ()
{
    GASubyte data[] = { 0x08, 0xde, 0xad, 0xbe, 0xef };
    GASunum num;
    GASnum result;
    result = gas_read_encoded_num_buf(data, sizeof(data), &num);
    QCOMPARE(result, static_cast<GASnum>(sizeof(data)));
    QCOMPARE(num, 0xdeadbeeful);
}

void TestBufIO::decode_failure_undersized_buffer_0xdeadbeef ()
{
    GASubyte data[] = { 0x08, 0xde, 0xad, 0xbe, 0xef };
    GASunum num;
    GASnum result;

    // for good measure, make sure it works
    result = gas_read_encoded_num_buf(data, sizeof(data), &num);
    QCOMPARE(result, static_cast<GASnum>(sizeof(data)));
    QCOMPARE(num, 0xdeadbeeful);

    for (unsigned int i = 0; i < sizeof(data); i++) {
        result = gas_read_encoded_num_buf(data, i, &num);
        QVERIFY(result < 0);
    }
}

void TestBufIO::decode_failure_undersized_buffer_0x8f ()
{
    GASubyte data[] = { 0x8f };
    GASunum num;
    GASnum result;

    // for good measure, make sure it works
    result = gas_read_encoded_num_buf(data, sizeof(data), &num);
    QCOMPARE(result, static_cast<GASnum>(sizeof(data)));
    QCOMPARE(num, 0xfUL);

    for (unsigned int i = 0; i < sizeof(data); i++) {
        result = gas_read_encoded_num_buf(data, i, &num);
        QVERIFY(result < 0);
    }
}

void TestBufIO::tree001 ()
{
    GASresult result;
    GASchunk* c = NULL;
    gas_new_named(&c, "blah");
    gas_set_payload_s(c, "hello world");
    result = gas_write_buf(buf, sizeof(buf), c);
    QVERIFY(result > 0);
}

void TestBufIO::tree002 ()
{
    GASresult result, size;
    GASchunk* c = NULL;
    gas_new_named(&c, "blah");
    gas_set_payload_s(c, "hello world");

    result = gas_write_buf(buf, sizeof(buf), c);
    QVERIFY(result > 0);

    size = result;

    // for second try, ensure it uses the same number of bytes again
    result = gas_write_buf(buf, size, c);
    QVERIFY(result > 0);
    QCOMPARE(result, size);

    // finally, ensure that claiming 1 too few bytes causes error
    size = result;
    result = gas_write_buf(buf, size - 1, c);
    QVERIFY(result < 0);

    gas_destroy(c);
}



int bufio (int argc, char **argv)
{
    TestBufIO tc;
    return QTest::qExec(&tc, argc, argv);
}
