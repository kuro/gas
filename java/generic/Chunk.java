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

import java.util.HashMap;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.Map;
import java.util.Formatter;
import java.util.Locale;

public class Chunk
{
    protected int size;
    public Chunk parent;
    public String id;
    public HashMap<String, byte[]> attributes;
    public byte[] payload;
    public ArrayList<Chunk> children;

    {
        attributes = new HashMap<String, byte[]>();
        children = new ArrayList<Chunk>();
    }

    public Chunk (String id)
    {
        this.id = id;
    }

    public Chunk (String id, Chunk parent)
    {
        this.id = id;
        parent.children.add(this);
        this.parent = parent;
    }

    public void update ()
    {
        int sum = 0;

        sum += ChunkIO.encodedSize(id.length());
        sum += id.length();

        sum += ChunkIO.encodedSize(attributes.size());
        Iterator<Map.Entry<String, byte[]>> ai
            = attributes.entrySet().iterator();
        while (ai.hasNext()) {
            Map.Entry<String, byte[]> entry = ai.next();

            String key = entry.getKey();
            sum += ChunkIO.encodedSize(key.length());
            sum += key.length();

            byte[] val = entry.getValue();
            sum += ChunkIO.encodedSize(val.length);
            sum += val.length;
        }

        if (payload == null) {
            sum += 1;  // size of 0x80
        } else {
            sum += ChunkIO.encodedSize(payload.length);
            sum += payload.length;
        }

        sum += ChunkIO.encodedSize(children.size());
        Iterator<Chunk> ci = children.iterator();
        while (ci.hasNext()) {
            Chunk child = ci.next();
            child.update();
            sum += ChunkIO.encodedSize(child.size);
            sum += child.size;
        }

        this.size = sum;
    }

    public String toString (String prefix)
    {
        StringBuilder sb = new StringBuilder();
        Formatter f = new Formatter(sb, Locale.US);

        f.format("%s---\n", prefix);

        if (id.length() == 0) {
            f.format("%s\"\"[%d]\n", prefix, size);
        } else {
            f.format("%s%s[%d]\n", prefix, id, size);
        }

        Iterator<Map.Entry<String, byte[]>> ai
            = attributes.entrySet().iterator();
        while (ai.hasNext()) {
            Map.Entry<String, byte[]> entry = ai.next();
            f.format("%s%s: \"%s\"\n", prefix,
                     entry.getKey(), new String(entry.getValue()));
        }

        if (payload != null && payload.length > 0) {
            f.format("%spayload[%d]: \"%s\"\n", prefix,
                     payload.length, new String(payload));
        }

        Iterator<Chunk> ci = children.iterator();
        while (ci.hasNext()) {
            Chunk child = ci.next();
            f.format("%s%s", prefix, child.toString(prefix + "  "));
        }

        return sb.toString();
    }

    public String toString ()
    {
        return toString("");
    }

    public int totalSize ()
    {
        return size + ChunkIO.encodedSize(size);
    }

    public void setPayload (String payload)
    {
        this.payload = payload.getBytes();
    }

}
