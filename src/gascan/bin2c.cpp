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

#include <QStringList>
#include <QDebug>
#include <QCoreApplication>
#include <QFile>

int bin2c (int argc, char** argv)
{
    QCoreApplication app (argc, argv);
    QStringList args = app.arguments();

    QString bufferName;
    QString fileIn = "-";
    QString fileOut = "-";

    args.takeFirst();

    if (!args.isEmpty()) {
        bufferName = args.takeFirst();
    }

    if (!args.isEmpty()) {
        fileIn = args.takeFirst();
    }

    if (!args.isEmpty()) {
        fileOut = args.takeFirst();
    }

    if (bufferName.isEmpty()) {
        qCritical() << "invalid usage: a buffer name must be specified";
        return EXIT_FAILURE;
    }

    QFile input (fileIn);
    if (fileIn == "-") {
        input.open(stdin, QIODevice::ReadOnly);
    } else {
        input.open(QIODevice::ReadOnly);
    }

    QFile output (fileOut);
    if (fileOut == "-") {
        output.open(stdout, QIODevice::WriteOnly);
    } else {
        output.open(QIODevice::WriteOnly);
    }

    QTextStream out (&output);

    out << "unsigned char " << bufferName << "[] = {" << endl;

    unsigned int offset = 0;
    QByteArray data;
    forever {
        if (input.atEnd()) {
            break;
        }
        data = input.read(8);

        out << "    /*";
        out << qSetPadChar('0') << qSetFieldWidth(8) << hex << offset;
        out.reset();
        out << "*/";

        foreach (quint8 c, data) {
            out << " 0x";
            out << qSetPadChar('0') << qSetFieldWidth(2) << hex << c;
            out.reset();
            out << ",";
        }

        if (data.size() < 8) {
            out << QString("      ").repeated(8 - data.size());
        }

        out << " /* ";
        foreach (QChar c, data) {
            out << (c.isPrint() ? c : '.');
        }
        out << " */";

        out << endl;

        offset += data.size();
    }

    out << "};" << endl;

    return EXIT_SUCCESS;
}
