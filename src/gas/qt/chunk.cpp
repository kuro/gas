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

#include <QHash>
#include <QtDebug>

extern "C"
{
GASunum gas_encoded_size (GASunum value);
}

using namespace Gas::Qt;

struct Chunk::Private
{
    unsigned int size;
    QHash<QString, QByteArray> attributes;
    QByteArray payload;

    Private() :
        size(0)
    {
    }
};

Chunk::Chunk (QString id, Chunk* parent) :
    QObject(parent),
    d(new Private)
{
    setId(id);
}

Chunk::~Chunk ()
{
    delete d;
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
    return qobject_cast<Chunk*>(parent());
}

void Chunk::setParentChunk (Chunk* p)
{
    setParent(p);
}

QList<Chunk*> Chunk::childChunks () const
{
    QList<Chunk*> list;
    Chunk* child;

    foreach (QObject* o, children()) {
        child = qobject_cast<Chunk*>(o);
        if (child) {
            list << child;
        }
    }

    return list;
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

/**
 * @brief equivalent to gas_total_size().
 */
int Chunk::size () const
{
    return d->size + gas_encoded_size(d->size);
}

void Chunk::update ()
{
    unsigned int& sum = d->size;
    Chunk* child;
    QHashIterator<QString, QByteArray> it (d->attributes);

    sum = 0;
    sum += gas_encoded_size(id().toUtf8().size());
    sum += id().toUtf8().size();
    sum += gas_encoded_size(attributes().size());
    while (it.hasNext()) {
        it.next();
        sum += gas_encoded_size(it.key().toUtf8().size());
        sum += it.key().toUtf8().size();
        sum += gas_encoded_size(it.value().size());
        sum += it.value().size();
    }
    sum += gas_encoded_size(payload().size());
    sum += payload().size();
    sum += gas_encoded_size(children().size());
    foreach (QObject* o, children()) {
        child = qobject_cast<Chunk*>(o);
        if (child) {
            child->update();
            sum += gas_encoded_size(child->d->size);
            sum += child->d->size;
        }
    }
}

bool Chunk::write (QIODevice* io) const
{
    QHashIterator<QString, QByteArray> it (d->attributes);
    Chunk* child;

    encode(io, d->size);
    encode(io, id().toUtf8().size());
    if (!io->write(id().toUtf8())) {
        return false;
    }
    encode(io, d->attributes.size());
    while (it.hasNext()) {
        it.next();
        encode(io, it.key().toUtf8().size());
        if (!io->write(it.key().toUtf8())) {
            return false;
        }
        encode(io, it.value().size());
        if (!io->write(it.value())) {
            return false;
        }
    }
    encode(io, payload().size());
    io->write(payload());
    encode(io, children().size());
    foreach (QObject* o, children()) {
        child = qobject_cast<Chunk*>(o);
        if (child && !child->write(io)) {
            return false;
        }
    }
    return true;
}

bool Chunk::read (QIODevice* io)
{
    unsigned int tmp, attr_count, child_count;
    QString key;
    QByteArray val;
    QByteArray buf;
    Chunk* child;

    decode(io, d->size);
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
        delete c;
        return NULL;
    }
    return c;
}

QDataStream& operator<< (QDataStream& stream, const Gas::Qt::Chunk& c)
{
    c.write(stream.device());
    return stream;
}

QDataStream& operator>> (QDataStream& stream, Gas::Qt::Chunk& c)
{
    c.read(stream.device());
    return stream;
}

// vim: sw=4
