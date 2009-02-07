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

/**
 * @file xml2gas.cpp
 * @brief xml2gas implementation
 */

#include <gas/fsio.h>
#include <gas/ntstring.h>

#include "gascan.h"

#include <fcntl.h>

#include <QXmlStreamReader>
#include <QFile>
#include <QtCore>

int xml2gas_main (int argc, char **argv)
{
    QString fin, fout;

    switch (argc) {
    case 1:
        fin = "-";
        fout = "-";
        break;
    case 2:
        fin = argv[1];
        fout = "-";
        break;
    case 3:
        fin = argv[1];
        fout = argv[2];
        break;
    default:
        die("invalid usage");
        break;
    }

    QFile *input;

    if (fin == "-") {
        input = new QFile();
        input->open(stdin, QIODevice::ReadOnly);
    } else {
        input = new QFile(fin);
        input->open(QIODevice::ReadOnly);
    }

    GASchunk* cur = NULL;
    gas_new_named(&cur, "fake_root");

    QXmlStreamReader xml (input);
    while ( ! xml.atEnd()) {
        xml.readNext();
        if (xml.hasError()) {
            qDebug() << xml.errorString();
        }
        if (xml.isStartElement()) {
            //QXmlStreamAttributes& attr = xml.attributes();

            GASchunk* n = NULL;
            gas_new(&n, (const char*)xml.name().toString().toAscii(),
                    xml.name().size());

            foreach(QXmlStreamAttribute attr, xml.attributes()) {
                gas_set_attribute_ss(n,
                        (const char*)attr.name().toString().toAscii(), 
                        (const char*)attr.value().toString().toAscii()
                        );
            }


            gas_add_child(cur, n);
            cur = n;
        }
        if ((xml.isCharacters() || xml.isCDATA()) && !xml.isWhitespace()) {
            const char *str = (const char*)xml.text().toString().toAscii();
            int length = xml.text().size();
            int total_new_len = length + cur->payload_size;
            cur->payload = (GASubyte*)realloc(cur->payload, total_new_len + 1);
            GAS_CHECK_MEM(cur->payload);
            memcpy(((GASubyte*)cur->payload) + cur->payload_size, str, length);
            ((char*)cur->payload)[total_new_len] = 0;
            cur->payload_size = total_new_len;
        }
        if (xml.isEndElement()) {
            cur = cur->parent;
        }
    }

    gas_update(cur);
    int verbose = 0;
    if (verbose) {
        //puts("printing gas");
        gas_print(cur);
    }
    //puts("saving gas");
    FILE* fs = NULL;
    if (fout == "-") {
        fs = stdin;
    } else {
        fs = fopen(fout.toAscii(), "w");
    }
    // i do not need to write the top root element,
    // so when reading the file back, there may be multiple top chunks
    unsigned int i;
    for (i = 0; i < cur->nb_children; i++) {
        printf("total size: %ld\n", gas_total_size(cur->children[i]));
        gas_write_fs(fs, cur->children[i]);
    }
    fclose(fs);
    gas_destroy(cur);

    return 0;
}

// vim: sw=4 fdm=marker
