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
 * @file Scanner.h
 * @brief Scanner definition
 */

#pragma once

#include <QHash>
#include <QTextStream>

class QIODevice;

namespace Gas
{
class Scanner
{
public:
    Scanner (QIODevice* dev);
    ~Scanner ();

    enum Error
    {
        NoError,
        PrematureEndOfDocumentError
    };

    enum TokenType
    {
        NoToken,
        Invalid,
        Push,
        Pop
    };

    bool atEnd () const;
    TokenType readNext ();

    Error error () const;

    QByteArray id () const;
    QHash<QByteArray, QByteArray> attributes () const;
    QByteArray payload () const;

    void skip ();

    template <typename T>
    inline
    T textValue (const QString& key) const;

    template <typename T>
    inline
    T dataValue (const QString& key) const;

    void setByteOrder (QDataStream::ByteOrder);
    QDataStream::ByteOrder byteOrder () const;

    void setFloatingPointPrecision (QDataStream::FloatingPointPrecision);
    QDataStream::FloatingPointPrecision floatingPointPrecision () const;

private:
    struct Private;
    QScopedPointer<Private> d;

    Q_DISABLE_COPY(Scanner);
};

#include "Scanner.inl"

} // namespace Gas

// vim: sw=4
