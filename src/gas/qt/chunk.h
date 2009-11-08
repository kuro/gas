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
    unsigned int decode (QIODevice* io, T& value);

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

    QList<Chunk*> childChunks () const;

    QHash<QString, QByteArray>& attributes ();
    const QHash<QString, QByteArray> attributes () const;
    QByteArray& operator[] (const QString& key);
    const QByteArray operator[] (const QString& key) const;

    void setAttribute (const QString& key, const QByteArray& value);
    void setAttribute (const QString& key, const QVariant& value);
    bool hasAttribute (const QString& key);

    template <typename T>
    inline
    void dataInsert (const QString& key, const T& value,
                     QDataStream::ByteOrder bo = QDataStream::BigEndian);

    template <typename T>
    inline
    T textValue (const QString& key) const;

    template <typename T>
    inline
    void textInsert (const QString& key, const T& value);

    template <typename T>
    inline
    T dataValue (const QString& key,
                 QDataStream::ByteOrder bo = QDataStream::BigEndian) const;

    inline quint32 encodedValue (const QString& key);
    inline void encodedInsert (const QString& key, quint32 val);

    void setFloatingPointPrecision (QDataStream::FloatingPointPrecision);
    QDataStream::FloatingPointPrecision floatingPointPrecision () const;

    int size () const;

    void update () const;

    /**
     * @note It is *not* necessary to call update() first.
     */
    bool write (QIODevice* io, bool needsUpdate = true) const;

    bool read (QIODevice* io);

    static Chunk* parse (QIODevice* io);

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
    Private* d;
};

#include "chunk.inl"

}
}

QDataStream& operator<< (QDataStream& stream, const Gas::Qt::Chunk& c);
QDataStream& operator>> (QDataStream& stream, Gas::Qt::Chunk& c);

QDebug operator<< (QDebug stream, const Gas::Qt::Chunk& c);

// vim: sw=4
