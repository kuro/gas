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

/**
 * @file chunk.cpp
 * @brief Gas::Qt::Chunk implementation
 */

#include "chunk.moc"


#include "../types.h"
#include "../tree.h"

#include <QHash>
#include <QtDebug>
#include <QTextStream>
#include <QReadWriteLock>
#include <QStringList>
#include <QFile>

static inline
unsigned int encoded_size (unsigned int value)
{
    unsigned int i;
    for (i = 1; 1; i++) {
        if (value < ((1UL << (7UL*i))-1UL)) {
            break;
        }
    }
    return i + ((i - 1) >> 3);
}

using namespace Gas::Qt;

static QDataStream::ByteOrder g_defaultByteOrder = QDataStream::BigEndian;
static QDataStream::FloatingPointPrecision g_defaultFloatingPointPrecision
    = QDataStream::SinglePrecision;
static QReadWriteLock g_byteOrderLock;
static QReadWriteLock g_floatingPointPrecisionLock;

void Gas::Qt::setDefaultByteOrder (QDataStream::ByteOrder v)
{
    QWriteLocker ml (&g_byteOrderLock);
    g_defaultByteOrder = v;
}
void Gas::Qt::setDefaultFloatingPointPrecision (
    QDataStream::FloatingPointPrecision v)
{
    QWriteLocker ml (&g_floatingPointPrecisionLock);
    g_defaultFloatingPointPrecision = v;
}
QDataStream::ByteOrder Gas::Qt::defaultByteOrder ()
{
    QReadLocker ml (&g_byteOrderLock);
    return g_defaultByteOrder;
}
QDataStream::FloatingPointPrecision Gas::Qt::defaultFloatingPointPrecision ()
{
    QReadLocker ml (&g_floatingPointPrecisionLock);
    return g_defaultFloatingPointPrecision;
}

void Gas::Qt::hexdump (const QByteArray& buf)
{
    gas_hexdump(buf.data(), buf.size());
}

struct Chunk::Private
{
    mutable unsigned int size;
    QHash<QString, QByteArray> attributes;
    QByteArray payload;

    QDataStream::FloatingPointPrecision floatingPointPrecision;
    QDataStream::ByteOrder byteOrder;

    Chunk* parent;
    ChunkList children;

    Private () :
        size(0),
        parent(NULL)
    {
    }
};

Chunk::Chunk (QString id, Chunk* parent) :
    QObject(),
    d(new Private)
{
    setId(id);

    setParentChunk(parent);

    if (parent) {
        d->floatingPointPrecision = parent->floatingPointPrecision();
        d->byteOrder = parent->byteOrder();
    } else {
        d->floatingPointPrecision = defaultFloatingPointPrecision();
        d->byteOrder = defaultByteOrder();
    }
}

Chunk::~Chunk ()
{
    qDeleteAll(d->children);
}

QString Chunk::id () const
{
    return objectName();
}

void Chunk::setId (const QString& id)
{
    setObjectName(id);
}

QByteArray& Chunk::payload () const
{
    return d->payload;
}

void Chunk::setPayload (const QByteArray& payload)
{
    d->payload = payload;
}

void Chunk::setPayload (const QVariant& payload)
{
    setPayload(payload.toString().toUtf8());
}

Chunk* Chunk::parentChunk ()
{
    return d->parent;
}

void Chunk::setParentChunk (Chunk* parent)
{
    d->parent = parent;
    if (parent) {
        parent->d->children.append(this);
    }
}

ChunkList Chunk::childChunks () const
{
    return d->children;
}

Chunk* Chunk::at (const QString& path) const
{
    const Chunk* top = this;
    QStringList segments = path.split("/");
    int remainingLevels = segments.size();
    QStringListIterator segmentIter (segments);
    while (segmentIter.hasNext()) {
        const QString& segment = segmentIter.next();
        QListIterator<Chunk*> children (top->childChunks());
        while (children.hasNext()) {
            if (children.next()->id() == segment) {
                top = children.peekPrevious();
                remainingLevels--;
                break;
            }
        }
    }
    return remainingLevels ? NULL : const_cast<Chunk*>(top);
}

QHash<QString, QByteArray>& Chunk::attributes ()
{
    return d->attributes;
}

const QHash<QString, QByteArray> Chunk::attributes () const
{
    return d->attributes;
}

QByteArray& Chunk::operator[] (const QString& key)
{
    return d->attributes[key];
}

const QByteArray Chunk::operator[] (const QString& key) const
{
    return d->attributes[key];
}

void Chunk::setAttribute (const QString& key, const QByteArray& value)
{
    d->attributes.insert(key, value);
}

void Chunk::setAttribute (const QString& key, const QVariant& value)
{
    d->attributes.insert(key, value.toString().toUtf8());
}

bool Chunk::hasAttribute (const QString& key) const
{
    return d->attributes.contains(key);
}

int Chunk::size () const
{
    return d->size + encoded_size(d->size);
}

unsigned int Chunk::update () const
{
    const Chunk* child;
    unsigned int& sum = d->size;
    QHashIterator<QString, QByteArray> it (d->attributes);

    int tmp;

    sum = 0;
    // id
    tmp = id().size();
    sum += encoded_size(tmp);
    sum += tmp;
    // attributes
    sum += encoded_size(d->attributes.size());
    while (it.hasNext()) {
        it.next();
        // key
        tmp = it.key().size();
        sum += encoded_size(tmp);
        sum += tmp;
        // value
        tmp = it.value().size();
        sum += encoded_size(tmp);
        sum += tmp;
    }
    // payload
    tmp = d->payload.size();
    sum += encoded_size(tmp);
    sum += tmp;
    // children
    sum += encoded_size(d->children.size());

    for (ChunkList::ConstIterator it=d->children.begin(), end=d->children.end();
         it != end; ++it)
    {
        child = *it;
        tmp = child->update();
        sum += encoded_size(tmp);
        sum += tmp;
    }

    return sum;
}

/**
 * @remarks Qt's write returns false when the byte array is empty.
 *
 * @todo check the encode calls
 */
bool Chunk::write (QIODevice* io, bool needsUpdate) const
{
    if (needsUpdate) {
        update();
    }

    QHashIterator<QString, QByteArray> it (d->attributes);

    encode(io, d->size);
    encode(io, id().toUtf8().size());
    if (!io->write(id().toUtf8())) {
        goto abort;
    }
    encode(io, d->attributes.size());
    while (it.hasNext()) {
        it.next();
        encode(io, it.key().toUtf8().size());
        if (!io->write(it.key().toUtf8())) {
            goto abort;
        }
        encode(io, it.value().size());
        if ((it.value().size() > 0) && !io->write(it.value())) {
            goto abort;
        }
    }
    encode(io, payload().size());
    if ((payload().size() > 0) && !io->write(payload())) {
        goto abort;
    }
    encode(io, d->children.size());
    foreach (Chunk* child, d->children) {
        if (!child->write(io, false)) {
            goto abort;
        }
    }

    return true;

abort:
    qWarning() << io->errorString();
    return false;
}

QByteArray Chunk::serialize () const
{
    QByteArray byteArray;
    QBuffer buffer (&byteArray);
    buffer.open(QIODevice::WriteOnly);
    write(&buffer);
    return byteArray;
}

bool Chunk::read (QIODevice* io)
{
    unsigned int tmp, attr_count, child_count;
    QString key;
    QByteArray val;
    QByteArray buf;
    Chunk* child;

    decode(io, d->size, true);

    while (io->bytesAvailable() < d->size) {
        if (!io->waitForReadyRead(30000)) {
            qWarning() << Q_FUNC_INFO << io->errorString();
            return false;
        }
    }

    decode(io, tmp);
    buf = io->read(tmp);
    if (tmp != (unsigned int)buf.size()) {
        return false;
    }
    setId(buf);
    decode(io, attr_count);
    for (unsigned int i = 0; i < attr_count; i++) {
        decode(io, tmp);
        key = io->read(tmp);
        if (tmp != (unsigned int)key.size()) {
            return false;
        }
        decode(io, tmp);
        val = io->read(tmp);
        if (tmp != (unsigned int)val.size()) {
            return false;
        }
        d->attributes.insert(key, val);
    }
    decode(io, tmp);
    buf = io->read(tmp);
    if (tmp != (unsigned int)buf.size()) {
        return false;
    }
    setPayload(buf);
    decode(io, child_count);
    for (unsigned int i = 0; i < child_count; i++) {
        child = new Chunk(QString(), this);
        if (!child->read(io)) {
            delete child;
            return false;
        }
    }
    return true;
}

Chunk* Chunk::parse (QIODevice* io)
{
    Chunk* c = NULL;
    c = new Chunk;
    if (!c->read(io)) {
        qWarning() << "Chunk::read() failed";
        delete c;
        return NULL;
    }
    return c;
}

Chunk* Chunk::parse (const QByteArray& data)
{
    QBuffer buf;
    buf.setData(data);
    buf.open(QIODevice::ReadOnly);
    return parse(&buf);
}

Chunk* Chunk::parse (const QString& filename)
{
    QFile file (filename);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << file.errorString();
        return NULL;
    }

    uchar* buf = file.map(0, file.size());
    if (buf) {
        return parse(QByteArray::fromRawData((char*)buf, file.size()));
    } else {
        return parse(&file);
    }
}

QDataStream& operator<< (QDataStream& stream, const Gas::Qt::Chunk& c)
{
    c.write(stream.device(), true);
    return stream;
}

QDataStream& operator>> (QDataStream& stream, Gas::Qt::Chunk& c)
{
    c.read(stream.device());
    return stream;
}

void Chunk::dump (const QString& prefix, QTextStream* s) const
{
    Q_ASSERT(s);
    QHashIterator<QString, QByteArray> it (d->attributes);
    *s << prefix << "---" << endl;
    *s << prefix << "id: " << id() << endl;
    while (it.hasNext()) {
        it.next();
        *s << prefix << it.key() << ": " << it.value() << endl;
    }
    if (!d->payload.isEmpty()) {
        *s << prefix << "payload: " << d->payload << endl;
    }
    foreach (Chunk* child, d->children) {
        child->dump(prefix + "  ", s);
    }
}

void Chunk::dump (const QString& prefix) const
{
    QTextStream s (stderr);
    dump(prefix, &s);
}

void Chunk::dump () const
{
    QTextStream s (stderr);
    dump(QString(), &s);
}

void Chunk::dump (QTextStream* stream) const
{
    dump(QString(), stream);
}

void Chunk::dump (const QString& prefix, QIODevice* dev) const
{
    QTextStream s (dev);
    dump(prefix, &s);
}

void Chunk::dump (QIODevice* dev) const
{
    dump(QString(), dev);
}

QDebug operator<< (QDebug debug, const Gas::Qt::Chunk& c)
{
    QByteArray data;
    QBuffer buf (&data);
    buf.open(QIODevice::WriteOnly);
    c.dump(&buf);
    debug << data.data();
    return debug;
}

void Chunk::setByteOrder (QDataStream::ByteOrder v)
{
    d->byteOrder = v;
}

QDataStream::ByteOrder Chunk::byteOrder () const
{
    return d->byteOrder;
}

void Chunk::setFloatingPointPrecision (QDataStream::FloatingPointPrecision v)
{
    d->floatingPointPrecision = v;
}

QDataStream::FloatingPointPrecision Chunk::floatingPointPrecision () const
{
    return d->floatingPointPrecision;
}

// vim: sw=4
