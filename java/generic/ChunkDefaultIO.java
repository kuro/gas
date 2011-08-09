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

package com.mentaldistortion.gas;

import java.util.Stack;
import java.util.Iterator;
import java.util.Map;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;

public class ChunkDefaultIO
{

    private static void encodeNumber (OutputStream io, int num)
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
    }

    private static int decodeNumber (InputStream io)
        throws IOException
    {
        int retval = 0x0;
        int i, bytes_read, zero_byte_count, first_bit_set;
        int b, mask = 0x00;
        int additional_bytes_to_read;

        /* find first non 0x00 b */
        for (zero_byte_count = 0; true; zero_byte_count++) {
            b = io.read() & 0xff;
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
            b = io.read() & 0xff;
            retval = (retval << 8) | b;
        }

        return retval;
    }

//    private static int decodeNumber (byte[] bytea)
//        throws IOException
//    {
//        return decodeNumber(new java.io.ByteArrayInputStream(bytea));
//    }

    /**
     * @param c A chunk to be modified
     * @return number of children
     */
    private int parseChunk (InputStream io, Chunk c)
        throws IOException
    {
        c.size = decodeNumber(io);
        byte[] id = new byte[decodeNumber(io)];
        io.read(id);
        c.id = new String(id);
        int nbAttributes = decodeNumber(io);
        for (int i = 0; i < nbAttributes; i++) {
            byte[] key = new byte[decodeNumber(io)];
            io.read(key);
            byte[] val = new byte[decodeNumber(io)];
            io.read(val);
            c.attributes.put(new String(key), val);
        }
        c.payload = new byte[decodeNumber(io)];
        io.read(c.payload);
        int nbChildren = decodeNumber(io);
        return nbChildren;
    }

    public Chunk parse (InputStream io)
        throws IOException
    {
        Stack<Integer> children_remaining = new Stack<Integer>();
        Chunk c;
        int nb_children;

        // get the root
        c = new Chunk(null);
        nb_children = parseChunk(io, c);
        children_remaining.push(new Integer(nb_children));

        while (true) {
            if (children_remaining.peek().intValue() == 0) {
                children_remaining.pop();
                if (children_remaining.isEmpty()) {
                    return c;
                }
                c = c.parent;
            } else {
                nb_children = children_remaining.pop().intValue();
                children_remaining.push(new Integer(nb_children - 1));

                Chunk child = new Chunk(null);
                child.parent = c;
                c.children.add(child);
                c = child;

                nb_children = parseChunk(io, c);
                children_remaining.push(new Integer(nb_children));
            }
        }
    }

    private void writeChunk (OutputStream io, Chunk c)
        throws IOException
    {
        encodeNumber(io, c.size);

        encodeNumber(io, c.id.length());
        io.write(c.id.getBytes());

        encodeNumber(io, c.attributes.size());
        Iterator<Map.Entry<String, byte[]>> ai
            = c.attributes.entrySet().iterator();
        while (ai.hasNext()) {
            Map.Entry<String, byte[]> entry = ai.next();

            byte[] key = entry.getKey().getBytes();
            encodeNumber(io, key.length);
            io.write(key);

            byte[] val = entry.getValue();
            encodeNumber(io, val.length);
            io.write(val);
        }

        if (c.payload == null) {
            io.write(0x80);
        } else {
            encodeNumber(io, c.payload.length);
            io.write(c.payload);
        }

        encodeNumber(io, c.children.size());
    }

    /**
     * @warning Does not auto update.
     */
    public void write (OutputStream io, Chunk root)
        throws IOException
    {
        Chunk c = root;
        Stack<Integer> children_remaining = new Stack<Integer>();

        children_remaining.push(new Integer(c.children.size()));
        writeChunk(io, c);

        while (true) {
            if (children_remaining.peek().intValue() == 0) {
                children_remaining.pop();
                if (c.parent == null) {
                    break;
                }
                c = c.parent;
            } else {
                int idx = children_remaining.pop().intValue();
                children_remaining.push(new Integer(idx - 1));

                c = c.children.get(c.children.size() - idx);
                children_remaining.push(new Integer(c.children.size()));

                writeChunk(io, c);
            }
        }
    }
}
