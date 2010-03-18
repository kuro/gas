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
 * @file chunk.h
 * @brief Gas::Qt::Chunk definition
 */

#pragma once

#include <QObject>
#include <QByteArray>
#include <QHash>
#include <QVariant>
#include <QTextStream>
#include <QBuffer>

namespace Gas
{
namespace Qt
{

class Chunk;

typedef QList<Chunk*> ChunkList;
typedef QListIterator<Chunk*> ChunkListIterator;

class Chunk : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId)
    Q_PROPERTY(QByteArray payload READ payload WRITE setPayload)
    Q_PROPERTY(Chunk* parentChunk READ parentChunk WRITE setParentChunk)

public:
    /// @returns number of bytes produced
    /// @retval 0 error
    template <typename T>
    static inline
    unsigned int encode (QIODevice* io, const T& value);

    /// @returns number of bytes consumed
    /// @retval 0 error
    template <typename T>
    static inline
    unsigned int decode (QIODevice* io, T& value, bool block = false);

public:
    Chunk (QString id = QString(), Chunk* parent = NULL);
    virtual ~Chunk ();

    QString id () const;
    void setId (const QString& id);

    QByteArray& payload () const;
    void setPayload (const QByteArray& payload);
    void setPayload (const QVariant& payload);

    Chunk* parentChunk ();
    void setParentChunk (Chunk* p);

    ChunkList& childChunks () const;

    Chunk* at (const QString& path) const;

    QHash<QString, QByteArray>& attributes ();
    const QHash<QString, QByteArray> attributes () const;
    QByteArray& operator[] (const QString& key);
    const QByteArray operator[] (const QString& key) const;

    void setAttribute (const QString& key, const QByteArray& value);
    void setAttribute (const QString& key, const QVariant& value);
    bool hasAttribute (const QString& key) const;

    template <typename T>
    inline
    void dataInsert (const QString& key, const T& value);

    template <typename T>
    inline
    T textValue (const QString& key) const;

    template <typename T>
    inline
    void textInsert (const QString& key, const T& value);

    template <typename T>
    inline
    T dataValue (const QString& key) const;

    inline quint32 encodedValue (const QString& key);
    inline void encodedInsert (const QString& key, quint32 val);

    void setByteOrder (QDataStream::ByteOrder);
    QDataStream::ByteOrder byteOrder () const;

    void setFloatingPointPrecision (QDataStream::FloatingPointPrecision);
    QDataStream::FloatingPointPrecision floatingPointPrecision () const;

    /**
     * @brief The total size.
     * @note Equivalent to gas_total_size().
     */
    int size () const;

    /**
     * @note The returned value is *not* the total size.
     */
    unsigned int update () const;

    /**
     * @note It is *not* necessary to call update() first.
     */
    bool write (QIODevice* io, bool needsUpdate = true) const;

    QByteArray serialize () const;

    bool read (QIODevice* io);

    static Chunk* parse (QIODevice* io);
    static Chunk* parse (const QByteArray& data);
    static Chunk* parse (const QString& filename);

    /// @brief dump to stderr (without prefix)
    void dump () const;

    /// @brief dump to stderr with @a prefix
    void dump (const QString& prefix) const;

    /// @brief dump to @a stream with @a prefix
    void dump (const QString& prefix, QTextStream* stream) const;

    /// @brief dump to @a dev with @a prefix
    void dump (const QString& prefix, QIODevice* dev) const;

    /// @brief dump to @a stream (without prefix)
    void dump (QTextStream* stream) const;

    /// @brief dump to @a dev (without prefix)
    void dump (QIODevice* dev) const;

private:
    struct Private;
    QScopedPointer<Private> d;

    Q_DISABLE_COPY(Chunk);
};

void setDefaultByteOrder (QDataStream::ByteOrder);
void setDefaultFloatingPointPrecision (QDataStream::FloatingPointPrecision);
QDataStream::ByteOrder defaultByteOrder ();
QDataStream::FloatingPointPrecision defaultFloatingPointPrecision ();

void hexdump (const QByteArray& buf);

#include "chunk.inl"

}
}

QDataStream& operator<< (QDataStream& stream, const Gas::Qt::Chunk& c);
QDataStream& operator>> (QDataStream& stream, Gas::Qt::Chunk& c);

QDebug operator<< (QDebug stream, const Gas::Qt::Chunk& c);

// vim: sw=4
