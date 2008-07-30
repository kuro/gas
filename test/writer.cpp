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
 * @file writer.cpp
 * @brief writer implementation
 */


#include <stdio.h>
#include  <QtTest>
#include "writer.moc"

GASresult my_write_payload (GASwriter* writer, GASchunk* c,
                            unsigned int *bytes_written)
{
    GASresult r;
    GASubyte buf[] = { 0xde, 0xad, 0xbe, 0xef };

    Q_CHECK_PTR(writer);
    Q_CHECK_PTR(c);
    Q_CHECK_PTR(bytes_written);
    Q_ASSERT(c->payload_size == 4);

    r = writer->context->write(writer->handle,
                               buf, c->payload_size, bytes_written,
                               writer->context->user_data);

    Q_ASSERT(*bytes_written == 4);
    Q_ASSERT(r == GAS_OK);

    return GAS_OK;
}

void TestWriter::test001 ()
{
    GASresult r;
    GASchunk *c;
    GAScontext *ctx;
    GASwriter *w;
    ctx = gas_context_new();
    w = gas_writer_new(ctx);
    c = gas_new_named("test");
    w->on_write_payload = my_write_payload;

    gas_set_payload(c, NULL, 4);

    gas_update(c);
    gas_print(c);
    gas_write(w, "writer-test-payload-callback.gas", c);
    gas_destroy(c);
    gas_writer_destroy(w);
}

void TestWriter::test002 ()
{
    GASresult r;
    GASchunk *c;
    GAScontext *ctx;
    GASwriter *w;
    ctx = gas_context_new();
    w = gas_writer_new(ctx);
    c = gas_new_named("test");
    w->on_write_payload = my_write_payload;

    gas_set_payload(c, "abcd", 4);

    gas_update(c);
    gas_print(c);
    gas_write(w, "writer-test-abcd.gas", c);
    gas_destroy(c);
    gas_writer_destroy(w);
}

int writer (int argc, char **argv)
{
    TestWriter test;
    return QTest::qExec(&test, argc, argv);
}

// vim: set fdm=marker
