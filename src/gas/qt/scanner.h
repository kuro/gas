
/**
 * @file scanner.h
 * @brief scanner definition
 */

#pragma once

#include <QHash>
#include <QTextStream>

class QIODevice;

namespace Gas
{
namespace Qt
{

struct ScannerPrivate;

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
    T dataValue (const QString& key,
                 QDataStream::ByteOrder bo = QDataStream::BigEndian) const;

private:
    ScannerPrivate* d;
};

#include "scanner.inl"

} // namespace Qt
} // namespace Gas

// vim: sw=4
