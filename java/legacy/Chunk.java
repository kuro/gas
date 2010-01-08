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

import java.io.*;
import java.util.*;
import java.nio.*;

public class Chunk
{

    public static int encodedSize (int num)//{{{
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
    public static void encodeNumber (OutputStream io, int num)//{{{
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
    public static int decodeNumber (InputStream io)//{{{
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
    public static int decodeNumber (byte[] bytea)//{{{
        throws IOException
    {
        return decodeNumber(new java.io.ByteArrayInputStream(bytea));
    }//}}}

    public static Chunk parse (InputStream io)//{{{
        throws IOException
    {
        Stack children_remaining = new Stack();
        Chunk c;
        int nb_children;

        // get the root
        c = new Chunk();
        nb_children = c.get(io);
        children_remaining.push(new Integer(nb_children));

        while (true) {
            if (((Integer)children_remaining.peek()).intValue() == 0) {
                children_remaining.pop();
                if (c.parent == null) {
                    return c;
                }
                c = c.parent;
            } else {
                nb_children = ((Integer)children_remaining.pop()).intValue();
                children_remaining.push(new Integer(nb_children - 1));

                Chunk child = new Chunk();
                child.parent = c;
                c.children.addElement(child);
                c = child;

                nb_children = c.get(io);
                children_remaining.push(new Integer(nb_children));
            }
        }
    }//}}}
    public void write (OutputStream io)//{{{
        throws IOException
    {
        Chunk c = this;
        Stack children_remaining = new Stack();

        update();

        children_remaining.push(new Integer(c.children.size()));
        c.put(io);

        while (true) {
            if (((Integer)children_remaining.peek()).intValue() == 0) {
                children_remaining.pop();
                if (c.parent == null) {
                    break;
                }
                c = c.parent;
            } else {
                int idx = ((Integer)children_remaining.pop()).intValue();
                children_remaining.push(new Integer(idx - 1));

                c = (Chunk)c.children.elementAt(c.children.size() - idx);
                children_remaining.push(new Integer(c.children.size()));

                c.put(io);
            }
        }
    }//}}}

    protected int size;
    public Chunk parent;
    public String id;
    public Hashtable attributes;
    public byte[] payload;
    public Vector children;

    /**
     * @return number of children
     */
    protected int get (InputStream io)//{{{
        throws IOException
    {
        byte[] tmp;
        int nb;
        size = decodeNumber(io);
        tmp = new byte[decodeNumber(io)];
        io.read(tmp);
        id = new String(tmp);
        nb = decodeNumber(io);
        //System.out.println(id + " has " + nb + " attributes");
        for (int i = 0; i < nb; i++) {
            byte[] key = new byte[decodeNumber(io)];
            io.read(key);
            byte[] val = new byte[decodeNumber(io)];
            io.read(val);
            attributes.put(new String(key), val);
        }
        //System.out.println(attributes);
        payload = new byte[decodeNumber(io)];
        io.read(payload);
        nb = decodeNumber(io);
        return nb;
    }//}}}
/*
    public String toString (int level)//{{{
    {
        StringBuilder sb = new StringBuilder();
        Formatter f = new Formatter(sb, Locale.US);

        for (int i = 0; i < level; i++) { f.format("  "); }
        f.format("---\n");

        //for (int i = 0; i < level; i++) { f.format("  "); }
        //f.format("size: %d\n", size);

        for (int i = 0; i < level; i++) { f.format("  "); }
        f.format("id[%d]: %s\n", id.length, new String(id));

        for (Enumeration e = attributes.keys(); e.hasMoreElements(); ) {
            byte[] key = (byte[])e.nextElement();
            byte[] val = attributes.get(key);
            for (int i = 0; i < level; i++) { f.format("  "); }
            f.format("%s => %s\n", new String(key), new String(val));
        }

        for (int i = 0; i < level; i++) { f.format("  "); }
        f.format("payload[%d]: %s\n", payload.length, new String(payload));

        for (Enumeration e = children.elements(); e.hasMoreElements(); ) {
            Chunk child = (Chunk)e.nextElement();
            f.format("%s", child.toString(level + 1));
        }

        return sb.toString();
    }//}}}

    public String toString ()//{{{
    {
        return toString(0);
    }//}}}
 */

    public Chunk ()//{{{
    {
        size = 0;
        id = new String();
        attributes = new Hashtable();
        children = new Vector();
        payload = new byte[0];
    }//}}}
    public Chunk (String id)//{{{
    {
        this();
        this.id = id;
    }//}}}

    protected void update ()//{{{
    {
        int sum = 0;

        sum += encodedSize(id.length());
        sum += id.length();

        sum += encodedSize(attributes.size());
        for (Enumeration e = attributes.keys(); e.hasMoreElements(); ) {
            String key = (String)e.nextElement();
            byte[] val = (byte[])attributes.get(key);

            sum += encodedSize(key.length());
            sum += key.length();

            sum += encodedSize(val.length);
            sum += val.length;
        }

        sum += encodedSize(payload.length);
        sum += payload.length;

        sum += encodedSize(children.size());
        for (Enumeration e = children.elements(); e.hasMoreElements(); ) {
            Chunk child = (Chunk)e.nextElement();
            child.update();
            sum += encodedSize(child.size);
            sum += child.size;
        }

        this.size = sum;
    }//}}}
    protected void put (OutputStream io)//{{{
        throws IOException
    {
        encodeNumber(io, size);

        encodeNumber(io, id.length());
        io.write(id.getBytes());

        encodeNumber(io, attributes.size());
        for (Enumeration e = attributes.keys(); e.hasMoreElements(); ) {
            String key = (String)e.nextElement();
            byte[] val = (byte[])attributes.get(key);

            encodeNumber(io, key.length());
            io.write(key.getBytes());

            encodeNumber(io, val.length);
            io.write(val);
        }

        encodeNumber(io, payload.length);
        io.write(payload);

        encodeNumber(io, children.size());
    }//}}}

    public byte[] get (String key)
    {
        return (byte[])attributes.get(key);
    }

    public ByteBuffer getDirectByteBuffer (String key)//{{{
    {
        byte[] ba = get(key);
        ByteBuffer bb = ByteBuffer.allocateDirect(ba.length);
        bb.put(ba);
        bb.rewind();
        return bb;
    }//}}}
    public FloatBuffer getDirectFloatBuffer (String key)//{{{
    {
        return getDirectByteBuffer(key).asFloatBuffer();
    }//}}}
    public IntBuffer getDirectIntBuffer (String key)//{{{
    {
        return getDirectByteBuffer(key).asIntBuffer();
    }//}}}
    public ShortBuffer getDirectShortBuffer (String key)//{{{
    {
        return getDirectByteBuffer(key).asShortBuffer();
    }//}}}

    public ByteBuffer payloadDirectByteBuffer ()//{{{
    {
        ByteBuffer bb = ByteBuffer.allocateDirect(payload.length);
        bb.put(payload);
        bb.rewind();
        return bb;
    }//}}}
    public FloatBuffer payloadDirectFloatBuffer ()//{{{
    {
        return payloadDirectByteBuffer().asFloatBuffer();
    }//}}}
    public IntBuffer payloadDirectIntBuffer ()//{{{
    {
        return payloadDirectByteBuffer().asIntBuffer();
    }//}}}
    public ShortBuffer payloadDirectShortBuffer ()//{{{
    {
        return payloadDirectByteBuffer().asShortBuffer();
    }//}}}

    public ByteBuffer getByteBuffer (String key)//{{{
    {
        byte[] ba = get(key);
        return ByteBuffer.wrap(ba);
    }//}}}
    public FloatBuffer getFloatBuffer (String key)//{{{
    {
        return getByteBuffer(key).asFloatBuffer();
    }//}}}
    public IntBuffer getIntBuffer (String key)//{{{
    {
        return getByteBuffer(key).asIntBuffer();
    }//}}}
    public ShortBuffer getShortBuffer (String key)//{{{
    {
        return getByteBuffer(key).asShortBuffer();
    }//}}}

    public ByteBuffer payloadByteBuffer ()//{{{
    {
        return ByteBuffer.wrap(payload);
    }//}}}
    public FloatBuffer payloadFloatBuffer ()//{{{
    {
        return payloadByteBuffer().asFloatBuffer();
    }//}}}
    public IntBuffer payloadIntBuffer ()//{{{
    {
        return payloadByteBuffer().asIntBuffer();
    }//}}}
    public ShortBuffer payloadShortBuffer ()//{{{
    {
        return payloadByteBuffer().asShortBuffer();
    }//}}}

}

// vim: fdm=marker
