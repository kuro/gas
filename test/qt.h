/*
 * Copyright 2009 Blanton Black
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

#pragma once

#include  <QObject>

class TestGasQt : public QObject
{
    Q_OBJECT

private slots:
    void test_001 ();
    void non_mapped ();
    void mapped ();
    void streams ();
    void scanner ();
    void scanner_bm ();
    void scanner_mapped_bm ();
    void data ();
    void text ();
    void raw ();
    void at ();
    void benchmark_update ();
    void variants ();
};

// vim: sw=4 fdm=marker
