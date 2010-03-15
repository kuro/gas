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

#define FAST 1

/**
 * @file xml2gas.cpp
 * @brief xml2gas implementation
 */

#include <gas/qt/chunk.h>

#include <QXmlStreamReader>
#include <QFile>
#include <QTime>
#include <QCoreApplication>
#include <QPointer>
#include <QStringList>
#include <QUuid>
#include <QDebug>

using namespace Gas::Qt;

static int verbose = 0;
static int trim = 0;
static int translate = 0;

QRegExp uuidRegex (
    "\\{?[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}\\}?"
    );

Chunk* parse (const QString& fin)
{
    QTime t;
    t.start();

    QFile input (fin);
    if (fin == "-") {
        input.open(stdin, QIODevice::ReadOnly);
    } else {
        input.open(QIODevice::ReadOnly);
    }

    Chunk* cur = new Chunk("fake_root");

    if (verbose > 0) {
        qDebug() << "parsing";
    }

    QPointer<QIODevice> dev (&input);
    const uchar* map = NULL;
    QByteArray mappedData;
    if (!input.isSequential()) {
        map = input.map(0, input.size());
        mappedData = QByteArray::fromRawData((const char*)map,
                                                        input.size());
    }
    if (map) {
        QBuffer* buf = new QBuffer(&mappedData);
        buf->open(QIODevice::ReadOnly);
        dev = buf;
    }

    QXmlStreamReader xml (dev);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.hasError()) {
            qWarning() << xml.errorString();
        }
        if (xml.isStartElement()) {
            Chunk* n = new Chunk(xml.name().toString(), cur);
            foreach(const QXmlStreamAttribute& attr, xml.attributes()) {
                QString key = attr.name().toString();
                QString value = attr.value().toString();
                if (translate) {
                    if (uuidRegex.exactMatch(value)) {
                        n->dataInsert(key, QUuid(value));
                    } else {
                        n->attributes().insert(key, value.toUtf8());
                    }
                } else {
                    n->attributes().insert(key, value.toUtf8());
                }
            }
            cur = n;
        }
        if ((xml.isCharacters() || xml.isCDATA()) && !xml.isWhitespace()) {
            QByteArray payload = xml.text().toString().toUtf8();
            if (trim > 0) {
                payload = payload.trimmed();
            }
            cur->payload().append(payload);
        }
        if (xml.isEndElement()) {
            cur = cur->parentChunk();
        }
    }

    input.close();

    if (map) {
        dev->close();
        delete dev;
    }

    if (verbose > 0) {
        qDebug() << t.elapsed() << "ms";
    }

    Q_ASSERT(cur->id() == "fake_root");

    return cur;
}

void write (const QString& fout, Chunk* root)
{
    QTime t;
    t.start();

    // write
    QFile output (fout);

    if (fout == "-") {
        output.open(stdout, QIODevice::WriteOnly);
    } else {
        output.open(QIODevice::WriteOnly);
    }

    if (verbose > 0) {
        qDebug() << "writing";
    }

    // i do not need to write the top root element,
    // so when reading the file back, there may be multiple top chunks
    QListIterator<Chunk*> it (root->childChunks());
    while (it.hasNext()) {
        it.next()->write(&output);
    }

    output.close();

    if (verbose > 0) {
        qDebug() << t.restart() << "ms";
    }
}

int xml2gas_main (int argc, char **argv)
{
    QCoreApplication app (argc, argv);
    QStringList args = app.arguments();

    verbose += args.removeAll("-v");
    trim += args.removeAll("--trim");
    trim -= args.removeAll("--no-trim");
    translate += args.removeAll("--translate");
    translate += args.removeAll("-t");

    QString fin  = "-";
    QString fout = "-";

    switch (args.size()) {
    case 3:
        fout = args[2];
        // fall
    case 2:
        fin = args[1];
        break;
    case 1:
        break;
    default:
        qFatal("invalid usage");
        break;
    }

    Chunk* root = parse(fin);
    Q_ASSERT(root);

    write(fout, root);

#if !FAST
    if (verbose > 0) {
        qDebug() << "cleaning";
    }
    delete root;
    if (verbose > 0) {
        qDebug() << t.restart() << "ms";
    }
#endif

    return 0;
}
