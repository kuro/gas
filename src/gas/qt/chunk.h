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

namespace Gas
{
namespace Qt
{
struct ChunkPrivate;
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

    QByteArray payload () const;
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

    int size () const;
    void update ();
    bool write (QIODevice* io) const;
    bool read (QIODevice* io);

    static Chunk* parse (QIODevice* io);

protected:
    ChunkPrivate* d;
};

#include "chunk.inl"

}
}

QDataStream& operator<< (QDataStream& stream, const Gas::Qt::Chunk& c);
QDataStream& operator>> (QDataStream& stream, Gas::Qt::Chunk& c);

// vim: sw=4
