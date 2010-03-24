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
#include  <qt/chunk.h>
#include  <qt/scanner.h>
#include  <gas/types.h>
#include "qt.moc"

#define TEST_FILE "test.gas"

#define showit(v) qDebug().nospace() << #v << ": " << v

using namespace Gas;

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
    qDebug() << "root" << c->parentChunk()->objectName();

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
    //qDebug() << "child's attributes" << n->findChild<Chunk*>("asdf")->attributes();
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
            case Scanner::Push: {
                //const QString& id = scanner.id();
                //qDebug() << id;
                //scanner.skip();
                break;
            }
            default:
                break;
            }
            QCOMPARE(scanner.error(), Scanner::NoError);
        }
    }
}

void TestGasQt::scanner_mapped_bm ()
{
    QFile file (TEST_FILE);
    file.open(QIODevice::ReadOnly);
    uchar* map = file.map(0, file.size());
    QByteArray ba = QByteArray::fromRawData((char*)map, file.size());
    QBuffer dev (&ba);
    dev.open(QIODevice::ReadOnly);

    Scanner scanner (&dev);

    QBENCHMARK {
        file.reset();
        while (!scanner.atEnd()) {
            switch (scanner.readNext()) {
            case Scanner::Push: {
                //const QString& id = scanner.id();
                //qDebug() << id;
                //scanner.skip();
                break;
            }
            default:
                break;
            }
            QCOMPARE(scanner.error(), Scanner::NoError);
        }
    }
}

void TestGasQt::data ()
{
    Chunk* c = new Chunk("root");
    quint32 val = 0x30313233u;
    c->dataInsert("quint32", val);
    c->dataInsert("x", 0x30313233u);
    val = 0;
    val = c->dataValue<quint32>("quint32");
    qDebug().nospace() << hex << "0x" << val;
    QCOMPARE(val, 0x30313233u);

    c->dataInsert("today", QDate::currentDate());

    qDebug() << c->attributes();
    qDebug() << "should not be found>" << c->dataValue<int>("not found");
}

void TestGasQt::text ()
{
    Chunk* c = new Chunk("root");
    quint32 val = 0x30313233u;
    c->textInsert("quint32", val);
    c->textInsert("string", "hello world");
    qDebug() << c->textValue<quint32>("quint32");

    c->textInsert("x", 0x30313233u);
    val = 0;
    val = c->textValue<quint32>("quint32");
    qDebug().nospace() << hex << "0x" << val;
    QCOMPARE(val, 0x30313233u);

    qDebug() << c->attributes();
    qDebug() << "should not be found>" << c->textValue<quint32>("not found");
}

void TestGasQt::raw ()
{
    Chunk* c = new Chunk("root");
    (*c)["blah"] = QByteArray::fromRawData("abc", 3);
    int iVal = 0x30313233u;
    (*c)["blah"] = QByteArray::fromRawData((char*)&iVal, sizeof(iVal));
    qDebug() << c->attributes();
    qDebug().nospace() << hex << "0x" << *(int*)(*c)["blah"].data();
    QCOMPARE(*(int*)(*c)["blah"].data(), iVal);
    c->update();
    qDebug() << c->size();
}

void TestGasQt::at ()
{
    QScopedPointer<Chunk> root (new Chunk("root"));
    Chunk* a = new Chunk("a", root.data());
    Chunk* b = new Chunk("b", root.data());
    Chunk* a1 = new Chunk("1", a);
    Chunk* a2 = new Chunk("2", a);
    Chunk* b1 = new Chunk("1", b);
    Chunk* b2 = new Chunk("2", b);

    QCOMPARE(root->at("a"), a);
    QCOMPARE(root->at("b"), b);
    QCOMPARE(root->at("c"), (Chunk*)NULL);
    QCOMPARE(root->at("c/1"), (Chunk*)NULL);
    QCOMPARE(root->at("a/1"), a1);
    QCOMPARE(root->at("b/2"), b2);
    QCOMPARE(root->at("b/3"), (Chunk*)NULL);

    QBENCHMARK {
        root->at("a/1");
    }
}

void TestGasQt::benchmark_update ()
{
    QScopedPointer<Chunk> root (Chunk::parse(QString(TEST_FILE)));
    QBENCHMARK {
        root->update();
    }
}

void TestGasQt::variants ()
{
    QScopedPointer<Chunk> c (new Chunk("test"));
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QDate currentDate = QDate::currentDate();
    QTime currentTime = QTime::currentTime();
    QVariantList list123;
    list123 << 1 << 2 << 3;

    Chunk::StorageType type;
#if 0
    type = Chunk::STRING;
#else
    type = Chunk::DATA;
#endif

    QBitArray flags (2);
    flags.setBit(1);
    int iflags;
    QBENCHMARK {
        flags.testBit(1);
        //(iflags & (1 << 0)) == 1;
    }
    return;
        c->variantInsert("true", true, type);
        c->variantInsert("false", false, type);
        c->variantInsert("true1", 1, type);
        c->variantInsert("false0", 0, type);
        c->variantInsert("0", 0, type);
        c->variantInsert("1", 1, type);
        c->variantInsert("123.4f", 123.4f, type);
        c->variantInsert("123.4", 123.4, type);
        c->variantInsert("currentDateTime", currentDateTime, type);
        c->variantInsert("currentTime", currentTime, type);
        c->variantInsert("currentDate", currentDate, type);
        c->variantInsert("list123", list123, type);

    c->dump("variants: ");

    QFile file ("test-variants.gas");
    file.open(QIODevice::WriteOnly);
    c->write(&file);
    file.close();

//    showit(c->variantValue<bool>("true"));
//    showit(c->variantValue<bool>("false"));
//    showit(c->variantValue<bool>("0"));
//    showit(c->variantValue<bool>("1"));
//    showit(c->variantValue<QString>("true"));
//    showit(c->variantValue<int>("1"));
//    showit(c->variantValue<int>("123.4"));
//    showit(c->variantValue<qreal>("1"));
//    showit(c->variantValue<qreal>("123.4"));
//    showit(c->variantValue<QTime>("now"));
//    showit(c->variantValue<QVariantList>("list123"));

    QCOMPARE(c->variantValue<bool>("true"), true);
    QCOMPARE(c->variantValue<bool>("false"), false);
    QCOMPARE(c->variantValue<bool>("true1"), true);
    QCOMPARE(c->variantValue<bool>("false0"), false);
    QCOMPARE(c->variantValue<bool>("1"), true);
    QCOMPARE(c->variantValue<bool>("0"), false);
    QCOMPARE(c->variantValue<int>("1"), 1);
    QCOMPARE(c->variantValue<int>("0"), 0);
    QCOMPARE(c->variantValue<float>("1"), 1.f);
    QCOMPARE(c->variantValue<float>("0"), 0.f);
    QCOMPARE(c->variantValue<QDateTime>("currentDateTime"), currentDateTime);
    QCOMPARE(c->variantValue<QTime>("currentTime"), currentTime);
    QCOMPARE(c->variantValue<QDate>("currentDate"), currentDate);
    QCOMPARE(c->variantValue<QVariantList>("list123"), list123);
    QCOMPARE(c->variantValue<float>("123.4f"), 123.4f);

    /// @todo the following two tests fail
    //QCOMPARE(c->variantValue<double>("123.4"), 123.4);
    //QVERIFY(qFuzzyCompare(c->variantValue<double>("123.4"), 123.4));
}

int qt (int argc, char **argv)
{
    TestGasQt tc;
    return QTest::qExec(&tc, argc, argv);
}
