
#include  <QtTest>
#include "test_bufio.moc"

void xdump (GASubyte* data, GASunum len)
{
    printf("0x");
    for (GASunum i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

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


QTEST_APPLESS_MAIN(TestBufIO);
