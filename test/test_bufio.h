
/**
 * @file test_bufio.h
 * @brief test_bufio definition
 */

#pragma once

#include  <QObject>

#include <gas/bufio.h>

class TestBufIO : public QObject
{
    Q_OBJECT

private:
    GASubyte buf[1024];

private slots:
    void encode_0xdeadbeef ();
    void encode_0x7e ();
    void encode_0x7f ();
    void encode_failure_undersized_buffer_0xdeadbeef ();
    void encode_failure_undersized_buffer_0xdeadbeefdeadbeef ();

    void decode_0xdeadbeef ();
    void decode_failure_undersized_buffer_0xdeadbeef ();
    void decode_failure_undersized_buffer_0x8f ();
};

// vim: sw=4 fdm=marker
