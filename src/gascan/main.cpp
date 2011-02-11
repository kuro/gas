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
 * @file main.cpp
 * @brief main implementation
 */

#include <QCoreApplication>
#include <QDebug>

int test_main (int argc, char **argv);

int xml2gas_main (int argc, char **argv);
int gas2xml_main (int argc, char **argv);

#ifdef QT_GUI_LIB
int qtedit_main (int argc, char **argv);
#endif

int bin2c (int argc, char** argv);

void print_gas_file (QString fname);

int main (int argc, char **argv)
{
    if (argc < 2) {
        qFatal("invalid usage: command not given");
    }

    QString cmd = argv[1];
    if (cmd == "print") {
        QCoreApplication (argc, argv);
        if (argc == 2) {
            print_gas_file("-");
        } else {
            for (int i = 2; i < argc; i++) {
                if (argc > 3) {
                    printf("*** %s\n", argv[i]);
                }
                print_gas_file(argv[i]);
                if (argc > 3) {
                    printf("\n");
                }
            }
        }
    } else if (cmd == "test") {
        test_main(argc-1, &argv[1]);
    } else if (cmd == "xml2gas") {
        xml2gas_main(argc-1, &argv[1]);
    } else if (cmd == "gas2xml") {
        gas2xml_main(argc-1, &argv[1]);
#ifdef QT_GUI_LIB
    } else if (cmd == "edit") {
        qtedit_main(argc-1, &argv[1]);
#endif
    } else if (cmd == "bin2c") {
        bin2c(argc-1, &argv[1]);
    } else {
        qFatal("invalid command");
    }
    return 0;
}

// vim: sw=4 fdm=marker
