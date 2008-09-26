/*
 * Copyright 2008 Blanton Black
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

import java.io.*;
import java.util.*;

public class Chunk
{

// static methods {{{
    public static int encoded_size (int num)//{{{
    {
        int i, coded_length;
        int zero_count, zero_bytes;

        for (i = 1; true; i++) {
            /*if (num < (unsigned int)pow(2, 7*i)-2) {*/
            if (num < ((1 << (7*i))-1)) {
                break;
            }
        }
        coded_length = i;  /* not including header */

        zero_count = coded_length - 1;
        zero_bytes = zero_count / 8;

        return coded_length + zero_bytes;
    }//}}}
    public static void encode_number (OutputStream io, int num)//{{{
        throws IOException
    {
        int i;
        int coded_length = 1;
        for (i = 1; true; i++) {
            if (num < ((1 << (7 * i)) - 1)) {
                break;
            }
            if ((i * 7L) > (4 * 8L)) {
                /* warning, close to overflow */
                System.out.println("warning: close to overflow");
                /* i--; */
                break;
            }
        }
        coded_length = i;

        int zero_count = coded_length - 1;
        int zero_bytes = zero_count >> 3;
        int zero_bits = zero_count % 8;
        for (i = 0; i < zero_bytes; i++) {
            io.write(0);
        }

        int mask = (0x80 >> zero_bits);
        int b = (mask|((num>>((coded_length-zero_bytes-1)<<3))&0xff));
        io.write(b);


        int si = coded_length - 2 - zero_bytes;
        while (si >= 0) {
            b = ((num >> (si << 3)) & 0xff);
            io.write(b);
            si -= 1;
        }
    }//}}}
    public static int decode_number (InputStream io)//{{{
        throws IOException
    {
        int retval = 0x0;
        int i, bytes_read, zero_byte_count, first_bit_set;
        int b, mask = 0x00;
        int additional_bytes_to_read;

        /* find first non 0x00 b */
        for (zero_byte_count = 0; true; zero_byte_count++) {
            b = io.read();
            if (b != 0x00) {
                break;
            }
        }

        /* process initial byte */
        for (first_bit_set = 7; first_bit_set >= 0; first_bit_set--) {
            if ((b & (1 << first_bit_set)) != 0) {
                break;
            }
        }

        for (i = 0; i < first_bit_set; i++) {
            mask |= (1 << i);
        }

        additional_bytes_to_read = (7-first_bit_set) + (7*zero_byte_count);

        /* at this point, i have enough information to construct retval */
        retval = mask & b;
        for (i = 0; i < additional_bytes_to_read; i++) {
            b = io.read();
            retval = (retval << 8) | b;
        }

        return retval;
    }//}}}
    public static int decode_number (byte[] bytea)//{{{
        throws IOException
    {
        return decode_number(new java.io.ByteArrayInputStream(bytea));
    }//}}}
//}}}

    protected int size;
    public byte[] id;
    public HashMap<byte[], byte[]> attributes;
    public byte[] payload;
    public ArrayList<Chunk> children;

    public Chunk ()//{{{
    {
        size = 0;
        id = new byte[0];
        attributes = new HashMap<byte[], byte[]>();
        children = new ArrayList<Chunk>();
        payload = new byte[0];
    }//}}}
    public Chunk (byte[] id)//{{{
    {
        this();
        this.id = id;
    }//}}}
    public Chunk (String id)//{{{
    {
        this(id.getBytes());
    }//}}}

    public void update ()//{{{
    {
        int sum = 0;

        sum += encoded_size(id.length);
        sum += id.length;

        sum += encoded_size(attributes.size());
        Iterator<Map.Entry<byte[], byte[]>> ai
            = attributes.entrySet().iterator();
        while (ai.hasNext()) {
            Map.Entry<byte[], byte[]> entry = ai.next();

            byte[] key = entry.getKey();
            sum += encoded_size(key.length);
            sum += key.length;

            byte[] val = entry.getValue();
            sum += encoded_size(val.length);
            sum += val.length;
        }

        sum += encoded_size(payload.length);
        sum += payload.length;

        sum += encoded_size(children.size());
        Iterator<Chunk> ci = children.iterator();
        while (ci.hasNext()) {
            Chunk child = ci.next();
            child.update();
            sum += encoded_size(child.size);
            sum += child.size;
        }

        this.size = sum;
    }//}}}
    public void write (OutputStream io)//{{{
        throws IOException
    {
        encode_number(io, size);

        encode_number(io, id.length);
        io.write(id);

        encode_number(io, attributes.size());
        Iterator<Map.Entry<byte[],byte[]>> ai =attributes.entrySet().iterator();
        while (ai.hasNext()) {
            Map.Entry<byte[], byte[]> entry = ai.next();

            byte[] key = entry.getKey();
            encode_number(io, key.length);
            io.write(key);

            byte[] val = entry.getValue();
            encode_number(io, val.length);
            io.write(val);
        }

        encode_number(io, payload.length);
        io.write(payload);

        encode_number(io, children.size());
        Iterator<Chunk> ci = children.iterator();
        while (ci.hasNext()) {
            Chunk child = ci.next();
            child.write(io);
        }

    }//}}}

}

// vim: fdm=marker
