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

public class Chunk
{
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
}

// vim: set fdm=marker
