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

import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;


public class ChunkIO
{

    public static int encodedSize (int num)
    {
        int i, coded_length;
        int zero_count, zero_bytes;

        for (i = 1; true; i++) {
            if (num < ((1 << (7*i))-1)) {
                break;
            }
        }
        coded_length = i;  /* not including header */

        zero_count = coded_length - 1;
        zero_bytes = zero_count / 8;

        return coded_length + zero_bytes;
    }

    public static Chunk parse (InputStream io)
        throws IOException
    {
        ChunkDefaultIO parser = new ChunkDefaultIO();
        return parser.parse(io);
    }

    public static void write (OutputStream io, Chunk root)
        throws IOException
    {
        ChunkDefaultIO writer = new ChunkDefaultIO();
        writer.write(io, root);
    }

}
