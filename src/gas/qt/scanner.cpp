
/**
 * @file scanner.cpp
 * @brief scanner implementation
 */

#include "scanner.h"
#include "chunk.h"

#include <QStack>

using namespace Gas::Qt;

struct Gas::Qt::Scanner::Private
{
    Scanner::Error error;
    QIODevice* dev;
    QByteArray id;
    QHash<QByteArray, QByteArray> attributes;
    QByteArray payload;
    QStack<unsigned int> children_remaining;
    bool skip;
    unsigned int size;
    unsigned int data_size;

    QDataStream::FloatingPointPrecision floatingPointPrecision;
    QDataStream::ByteOrder byteOrder;

    Private() :
        error(NoError),
        skip(false),
        floatingPointPrecision(defaultFloatingPointPrecision()),
        byteOrder(defaultByteOrder())
    {
    }
};

Scanner::Scanner (QIODevice* dev) :
    d(new Private)
{
    d->dev = dev;
}

Scanner::~Scanner ()
{
}

Scanner::Error Scanner::error () const
{
    return d->error;
}

bool Scanner::atEnd () const
{
    return d->dev->atEnd();
}

#include "../types.h"
extern "C"
{
GASunum gas_encoded_size (GASunum value);
}

Scanner::TokenType Scanner::readNext ()
{
    unsigned int size;
    unsigned int attributes_left;
    QByteArray key;
    QByteArray val;

    if (!d->children_remaining.isEmpty() && d->children_remaining.top() == 0) {
        d->children_remaining.pop();
        if (!d->children_remaining.isEmpty()) {
            d->children_remaining.top()--;
        }
        return Pop;
    }

    if (d->skip) {
        d->skip = false;
        unsigned int delta = d->size - d->data_size;
        if (d->dev->isSequential()) {
            d->dev->read(delta);
        } else {
            d->dev->seek(d->dev->pos() + delta);
        }
        return Pop;
    }

    Chunk::decode(d->dev, d->size);
    d->data_size = 0;

    d->data_size += Chunk::decode(d->dev, size);
    d->data_size += size;
    d->id = d->dev->read(size);

    d->data_size += Chunk::decode(d->dev, attributes_left);
    d->attributes.clear();
    for (; attributes_left; attributes_left--) {
        d->data_size += Chunk::decode(d->dev, size);
        d->data_size += size;
        key = d->dev->read(size);

        d->data_size += Chunk::decode(d->dev, size);
        d->data_size += size;
        val = d->dev->read(size);

        d->attributes.insert(key, val);
    }

    d->data_size += Chunk::decode(d->dev, size);
    d->data_size += size;
    d->payload = d->dev->read(size);

    d->data_size += Chunk::decode(d->dev, size);
    d->children_remaining.push(size);

    return Push;
}

QByteArray Scanner::id () const
{
    return d->id;
}

QHash<QByteArray, QByteArray> Scanner::attributes () const
{
    return d->attributes;
}

QByteArray Scanner::payload () const
{
    return d->payload;
}

void Scanner::skip ()
{
    d->skip = true;
}

void Scanner::setFloatingPointPrecision (QDataStream::FloatingPointPrecision v)
{
    d->floatingPointPrecision = v;
}

QDataStream::FloatingPointPrecision Scanner::floatingPointPrecision () const
{
    return d->floatingPointPrecision;
}

// vim: sw=4
