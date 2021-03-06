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
 * @file Chunk.h
 * @brief Gas::Chunk definition
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

class Chunk;

typedef QList<Chunk*> ChunkList;
typedef QListIterator<Chunk*> ChunkListIterator;

class Chunk : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId)
    Q_PROPERTY(QByteArray payload READ payload WRITE setPayload)
    Q_PROPERTY(Chunk* parentChunk READ parentChunk WRITE setParentChunk)
    Q_ENUMS(StorageType)

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

    /**
     * @brief QVariant storage methods.
     *
     * SRING appears to be faster.
     * DATA can handle more types.
     *
     * When STRING works, it will use less space than DATA.
     */
    enum StorageType
    {
        DATA,
        STRING
    };

public:
    Chunk (QString id = QString(), Chunk* parent = NULL);
    virtual ~Chunk ();

    QString id () const;
    void setId (const QString& id);

    QByteArray& payload () const;
    void setPayload (const QByteArray& payload);
    void setPayload (const QVariant& payload);

    Chunk* parentChunk () const;
    void setParentChunk (Chunk* p);

    ChunkList& childChunks () const;

    inline
    ChunkList& children () const
    {
        return childChunks();
    }

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

    inline quint32 encodedValue (const QString& key) const;
    inline void encodedInsert (const QString& key, quint32 val);

    template <typename T>
    inline
    void variantInsert (const QString& key, const T& value,
                        StorageType type = DATA);

    template <typename T>
    inline
    T variantValue (const QString& key, StorageType type = DATA) const;

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

    static Chunk* parse (QIODevice* io);
    static Chunk* parse (const QByteArray& data);
    static Chunk* parse (const QString& filename);
    static Chunk* parse (FILE* fs);

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

    static bool parseChunk (QIODevice* dev, Chunk* c, unsigned int& nbChildren);
    static bool writeChunk (QIODevice* dev, const Chunk* c);
    static void dumpChunk (const QString& prefix, QTextStream& s,
                           const Chunk* c);
};

void setDefaultByteOrder (QDataStream::ByteOrder);
void setDefaultFloatingPointPrecision (QDataStream::FloatingPointPrecision);
QDataStream::ByteOrder defaultByteOrder ();
QDataStream::FloatingPointPrecision defaultFloatingPointPrecision ();

#if 0
void hexdump (const QByteArray& buf);
#endif

#include "Chunk.inl"

} // namespace Gas

QDataStream& operator<< (QDataStream& stream, const Gas::Chunk& c);
//QDataStream& operator>> (QDataStream& stream, Gas::Chunk& c);

QDebug operator<< (QDebug stream, const Gas::Chunk& c);

// vim: sw=4
