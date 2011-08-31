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
 * @file gas2xml.cpp
 * @brief gas2xml implementation
 */

#include <qt/Chunk.h>

#include <QCoreApplication>
#include <QStringList>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDebug>

using namespace Gas;

static int use_cdata = 0;

static inline
void walk (Chunk* c, QXmlStreamWriter& writer)
{
    writer.writeStartElement(c->id());

    QHashIterator<QString, QByteArray> hit (c->attributes());
    while (hit.hasNext()) {
        hit.next();
        writer.writeAttribute(hit.key(), hit.value());
    }

    if (!c->payload().isEmpty()) {
        if (use_cdata) {
            writer.writeCDATA(c->payload());
        } else {
            writer.writeCharacters(c->payload());
        }
    }

    ChunkListIterator it (c->children());
    while (it.hasNext()) {
        walk(it.next(), writer);
    }

    writer.writeEndElement();
}

int gas2xml_main (int argc, char **argv)
{
    QCoreApplication app (argc, argv);
    QStringList args = app.arguments();

    int format = 0;

    format += args.removeAll("-f");
    use_cdata += args.removeAll("-c");
    use_cdata += args.removeAll("--cdata");

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

    QScopedPointer<Chunk> root;
    if (fin == "-") {
        QFile input;
        input.open(stdin, QIODevice::ReadOnly);
        root.reset(Chunk::parse(&input));
    } else {
        root.reset(Chunk::parse(fin));
    }
    Q_ASSERT(root);

    QScopedPointer<QFile> output;

    if (fout == "-") {
        output.reset(new QFile());
        output->open(stdout, QIODevice::WriteOnly);
    } else {
        output.reset(new QFile(fout));
        output->open(QIODevice::WriteOnly);
    }

    QXmlStreamWriter writer (output.data());
    writer.setAutoFormatting(format);

    writer.writeStartDocument();
    walk(root.data(), writer);
    writer.writeEndDocument();

    output->close();

    return 0;
}
