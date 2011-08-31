/*
 * Copyright 2011 Blanton Black
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
 * @file writer.h
 * @brief writer definition
 */

#pragma once

#include  <QObject>
#include <gas/writer.h>
#include <gas/ntstring.h>

class TestWriter : public QObject
{
    Q_OBJECT

private:
    GASubyte buf[1024];

private slots:
    void test001 (void);
    void test002 (void);
};

// vim: sw=4 fdm=marker
