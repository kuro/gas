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

    void tree001 ();
    void tree002 ();
};

// vim: sw=4 fdm=marker
