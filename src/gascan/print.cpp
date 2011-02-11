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
 * @file print.cpp
 * @brief print implementation
 */

#include "qt/Chunk.h"

#include <QFile>
#include <QBuffer>
#include <QDebug>
#include <QCoreApplication>

using namespace Gas;

/**
 * Print the gas tree.
 *
 * @todo This would be faster if it used a scanner rather than parsing out the
 * tree.
 */
void print_gas_file (QString fname)
{
    QFile file (fname);

    QIODevice* input = &file;  // the device to actually read from

    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly)) {
            qFatal("%s", qPrintable(file.errorString()));
        }
        // try to map
        const uchar* mappedData = file.map(0, file.size());
        if (mappedData) {
            const QByteArray buf = QByteArray::fromRawData(
                reinterpret_cast<const char*>(mappedData), file.size());
            input = new QBuffer(&file);
            qobject_cast<QBuffer*>(input)->setData(buf);
            if (!input->open(QIODevice::ReadOnly)) {
                qFatal("%s", qPrintable(input->errorString()));
            }
        }
    } else {
        if (fname == "-") {
            if (!file.open(stdin, QIODevice::ReadOnly)) {
                qFatal("%s", qPrintable(file.errorString()));
            }
        } else {
            qFatal("%s", qPrintable(file.errorString()));
        }
    }

    Q_ASSERT(input->isOpen());

    QTextStream output (stdout);
    while (!input->atEnd()) {
        QScopedPointer<Chunk> chunk (Chunk::parse(input));
        chunk->dump(QString(), &output);
    }

    file.close();
}
