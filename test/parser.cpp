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

#include <gas/parser.h>
#include <gas/ntstring.h>

#include <stdio.h>

static int indent_level = -1;

void indent (void)
{
    int i;
    for (i = 0; i < indent_level; i++) {
        printf("  ");
    }
}

/* default callbacks {{{*/
GASbool my_pre_chunk (GASunum id_size, void *id, void *user_data)
{
    return GAS_TRUE;
}

void my_push_id (GASunum id_size, void *id, void *user_data)
{
    indent_level++;
    indent();  printf("---\n");
    indent();  printf("GASchunk: \"%s\"\n", (char*)id);
}

//void my_push_chunk (GASchunk* c, void *user_data)
//{
//    /*gas_print(c);*/
//}

void my_pop_id (GASunum id_size, void *id, void *user_data)
{
    indent_level--;
}

//void my_pop_chunk (GASchunk* c, void *user_data)
//{
//}

void my_on_attribute (GASunum key_size, void *key,
                   GASunum value_size, void *value,
                   void *user_data)
{
    return;
    indent();  printf("attribute: \"%s\" -> \"%s\"\n", (char*)key, (char*)value);
}

void my_on_payload (GASunum payload_size, void *payload, void *user_data)
{
    indent();  printf("payload: \"%s\"\n", (char*)payload);
}
/*}}}*/


int parser (int argc, char **argv)
{
    GASresult r;
#if 1
    GASchunk *c;
    GAScontext *ctx;
    GASparser *p;

    gas_context_new(&ctx);
    gas_parser_new(&p, ctx);

    p->build_tree = GAS_FALSE;
    //p->get_payloads = GAS_FALSE;

    p->on_pre_chunk = my_pre_chunk;

    p->on_push_id = my_push_id;
    p->on_payload = my_on_payload;
    p->on_attribute = my_on_attribute;
    p->on_pop_id = my_pop_id;

    r = gas_parse(p, "dump.gas", &c);
    //r = gas_parse(p, "xslspec.gas", &c);
    //r = gas_parse(p, "physx-test.gas", &c);
    printf("%d\n", r);

    gas_parser_destroy(p);
    gas_print(c);
    gas_destroy(c);
    gas_context_destroy(ctx);
#endif

#if 0
    GASchunk *c = gas_new(0, NULL);
    GAScontext *s;

    gas_context_new(&s);
    gas_write_cb(c, s);
    gas_destroy(c);
#endif

    return 0;
}

// vim: set fdm=marker sw=4
