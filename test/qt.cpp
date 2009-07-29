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

#include  <QtTest>
#include  <gas/qt/chunk.h>
#include  <gas/qt/scanner.h>
#include  <gas/types.h>
#include "qt.moc"

#define TEST_FILE "test.gas"

using namespace Gas::Qt;

extern "C"
{
GASresult gas_hexdump (const GASvoid *input, GASunum size);
}

void walk (Chunk* c, int level = 0)
{
    QByteArray indentation;

    QTextStream stream (stdout);

    for (int i = 0; i < level; i++) {
        indentation += "  ";
    }

    stream << indentation << "---" << endl;

    stream << indentation << "id: \"" << c->id() << "\"" << endl;

    QHashIterator<QString, QByteArray> it (c->attributes());
    int i = 0;
    while (it.hasNext()) {
        it.next();
        stream << indentation << "attr[" << i << "]: \""
               << it.key() << "\" -> \"" << it.value() << "\"" << endl;
        i++;
    }

    stream << indentation << "payload: \"" << c->payload() << "\"" << endl;

    foreach (Chunk* child, c->childChunks()) {
        walk(child, level+1);
    }
}

void TestGasQt::test_001 ()
{
    Chunk* c = NULL;
    Chunk* root = NULL;
    root = new Chunk("root");
    c = new Chunk("asdf", root);
    qDebug() << c->id();
    qDebug() << "root" << c->parentChunk()->id();
    qDebug() << "root" << c->parent()->objectName();

    c->setProperty("payload", "this is my payload");
    qDebug() << "c->payload" << c->payload();

    (*c)["type"] = "unknown";
    qDebug() << (*c)["type"];

    c->attributes().insert("type", "known");
    qDebug() << (*c)["type"];

    quint32 tmp = 0x61626364u;
    (*c)["type"] = QByteArray((char*)&tmp, sizeof(tmp));
    qDebug() << (*c)["type"];

    c->setAttribute("something", 456);
    qDebug() << "something" << (*c)["something"];

    c->setAttribute("something more", QByteArray((char*)&tmp, sizeof(tmp)));
    qDebug() << "something more" << (*c)["something more"];

    c->setPayload(QDate::currentDate());
    qDebug() << "payload" << c->payload();

    c->setPayload(123);
    qDebug() << "payload" << c->payload();

    root->dumpObjectInfo();
    root->dumpObjectTree();

    root->update();
    qDebug() << root->size();

    QByteArray ba;
    QBuffer buf (&ba);
    buf.open(QIODevice::ReadWrite);
    root->write(&buf);
    gas_hexdump(ba.data(), ba.size());
    //qDebug() << hex << ba;

    buf.reset();
    Chunk* n = Chunk::parse(&buf);
    qDebug() << "child" << n->findChild<Chunk*>("asdf");
    qDebug() << "child's attributes" << n->findChild<Chunk*>("asdf")->attributes();
    delete n;

    buf.close();

    qDebug() << ba.size();

    delete root;
}

void TestGasQt::non_mapped ()
{
    QFile file (TEST_FILE);
    file.open(QIODevice::ReadOnly);
    Chunk* c = NULL;
    QBENCHMARK {
        file.reset();
        c = Chunk::parse(&file);
        //walk(c);
        delete c;
    }
    file.close();
}

void TestGasQt::mapped ()
{
    QFile file (TEST_FILE);
    file.open(QIODevice::ReadOnly);
    uchar* p = file.map(0, file.size());
    QByteArray ba = QByteArray::fromRawData((char*)p, file.size());
    QBuffer buf (&ba);
    buf.open(QIODevice::ReadOnly);
    Chunk* c = NULL;
    QBENCHMARK {
        buf.reset();
        c = Chunk::parse(&buf);
        //walk(c);
        delete c;
    }
    file.close();
}

void TestGasQt::streams ()
{
    QFile file (TEST_FILE);
    file.open(QIODevice::ReadOnly);
    QDataStream stream (&file);
    Chunk c;
    stream >> c;
    walk(&c);

    QByteArray ba;
    QBuffer buf (&ba);
    buf.open(QIODevice::WriteOnly);
    stream.setDevice(&buf);
    stream << c;

    QCOMPARE((int)ba.size(), (int)file.size());
}

void TestGasQt::scanner ()
{
    QFile file (TEST_FILE);
    file.open(QIODevice::ReadOnly);

    int depth = 0;

    Scanner scanner (&file);
    while (!scanner.atEnd())
    {
        QCOMPARE(scanner.error(), Scanner::NoError);
        switch (scanner.readNext())
        {
        case Scanner::Push:
            //for (int i = 0; i < depth; i++) { printf("  "); }
            qDebug() << "id:" << scanner.id();
            if (!scanner.attributes().isEmpty()) {
                //for (int i = 0; i < depth; i++) { printf("  "); }
                qDebug() << "attributes:" << scanner.attributes();
            }
            if (!scanner.payload().isEmpty()) {
                //for (int i = 0; i < depth; i++) { printf("  "); }
                qDebug() << "payload:" << scanner.payload();
            }
            depth++;
            break;
        case Scanner::Pop:
            depth--;
            break;
        default:
            QFAIL("oops");
            break;
        }
    }
}

void TestGasQt::scanner_bm ()
{
    QFile file (TEST_FILE);
    file.open(QIODevice::ReadOnly);
    Scanner scanner (&file);
    QBENCHMARK {
        file.reset();
        while (!scanner.atEnd()) {
            switch (scanner.readNext()) {
            case Scanner::Push:
                //scanner.skip();
                break;
            default:
                break;
            }
            QCOMPARE(scanner.error(), Scanner::NoError);
        }
    }
}

int qt (int argc, char **argv)
{
    TestGasQt tc;
    return QTest::qExec(&tc, argc, argv);
}
