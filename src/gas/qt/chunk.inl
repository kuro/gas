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
 * @file chunk.inl
 * @brief chunk inline implementation
 */

template <typename T>
inline
unsigned int Chunk::encode (QIODevice* io, const T& value)
{
    unsigned int i, coded_length;
    quint8 byte, mask;
    unsigned int zero_count, zero_bytes, zero_bits;
    int si;
    unsigned int bytes_produced = 0;

    for (i = 1; 1; i++) {
        if ((unsigned int)value < ((1UL << (7UL*i))-1UL)) {
            break;
        }
        if ((i * 7UL) > (sizeof(unsigned int) << 3)) {
            break;
        }
    }
    coded_length = i;

    zero_count = coded_length - 1;
    zero_bytes = zero_count / 8;
    zero_bits = zero_count % 8;

    byte = 0x0;
    for (i = 0; i < zero_bytes; i++) {
        if (!io->putChar(byte)) {
            return 0;
        }
        bytes_produced += 1;
    }

    mask = 0x80;
    mask >>= zero_bits;

    if (coded_length <= sizeof(unsigned int)) {
        byte = mask | ((value >> ((coded_length-zero_bytes-1)<<3)) & 0xff);
    } else {
        byte = mask;
    }
    if (!io->putChar(byte)) {
        return 0;
    }
    bytes_produced += 1;

    for (si = coded_length - 2 - zero_bytes; si >= 0; si--) {
        byte = ((value >> (si<<3)) & 0xff);
        if (!io->putChar(byte)) {
            return 0;
        }
        bytes_produced += 1;
    }

    return bytes_produced;
}

/// @returns number of bytes consumed
/// @retval 0 error
template <typename T>
inline
unsigned int Chunk::decode (QIODevice* io, T& value)
{
    unsigned int retval = 0x0;
    unsigned int i, zero_byte_count, first_bit_set;
    quint8 byte, mask = 0x00;
    unsigned int additional_bytes_to_read;
    unsigned int bytes_consumed = 0;

    /* find first non 0x00 byte */
    for (zero_byte_count = 0; 1; zero_byte_count++) {
        if (!io->getChar(reinterpret_cast<char*>(&byte))) {
            return 0;
        }
        bytes_consumed++;
        if (byte != 0x00)
            break;
    }

    /* process initial byte */
    for (first_bit_set = 7; first_bit_set >= 0; first_bit_set--)
        if (byte & (1L << first_bit_set))
            break;

    for (i = 0; i < first_bit_set; i++)
        mask |= (1L << i);

    additional_bytes_to_read = (7-first_bit_set) + (7*zero_byte_count);

    retval = mask & byte;
    for (i = 0; i < additional_bytes_to_read; i++) {
        if (!io->getChar(reinterpret_cast<char*>(&byte))) {
            return 0;
        }
        bytes_consumed++;
        retval = (retval << 8) | byte;
    }

    value = retval;
    return bytes_consumed;
}

/**
 * @name QDataStream based access.
 */
//@{
template <typename T>
inline
void Chunk::dataInsert (const QString& key, const T& value)
{
    QByteArray ba;
    QDataStream ds (&ba, QIODevice::WriteOnly);
    ds << value;

    attributes().insert(key, ba);
}

template <typename T>
inline
T Chunk::dataValue (const QString& key) const
{
    QDataStream ds (attributes().value(key));

    T retval;
    ds >> retval;
    return retval;
}

//@}

/**
 * @name QTextStream based access.
 */
//@{
template <typename T>
inline
void Chunk::textInsert (const QString& key, const T& value)
{
    QByteArray ba;
    QTextStream ts (&ba, QIODevice::WriteOnly);
    ts << value << flush;

    attributes().insert(key, ba);
}

template <typename T>
inline
T Chunk::textValue (const QString& key) const
{
    QTextStream ts (attributes().value(key));

    T retval;
    ts >> retval;
    return retval;
}
//@}

/**
 * @name gas encoded number based access.
 */
//@{
inline
quint32 Chunk::encodedValue (const QString& key)
{
    quint32 retval;
    QByteArray data = attributes().value(key);
    QBuffer io (&data);
    io.open(QIODevice::ReadOnly);
    unsigned int r = decode(&io, retval);
    Q_ASSERT(r);
    return retval;
}

inline
void Chunk::encodedInsert (const QString& key, quint32 val)
{
    QByteArray data;
    QBuffer io (&data);
    io.open(QIODevice::WriteOnly);
    unsigned int r = encode(&io, val);
    Q_ASSERT(r);
    setAttribute(key, data);
}
//@}
