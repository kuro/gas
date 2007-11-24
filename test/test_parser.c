
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
GASbool my_pre_chunk (size_t id_size, void *id, void *user_data)
{
    return GAS_TRUE;
}

void my_push_id (size_t id_size, void *id, void *user_data)
{
    indent_level++;
    indent();  printf("---\n");
    indent();  printf("chunk: \"%s\"\n", (char*)id);
}

//void my_push_chunk (chunk* c, void *user_data)
//{
//    /*gas_print(c);*/
//}

void my_pop_id (size_t id_size, void *id, void *user_data)
{
    indent_level--;
}

//void my_pop_chunk (chunk* c, void *user_data)
//{
//}

void my_on_attribute (size_t key_size, void *key,
                   size_t value_size, void *value,
                   void *user_data)
{
    return;
    indent();  printf("attribute: \"%s\" -> \"%s\"\n", (char*)key, (char*)value);
}

void my_on_payload (size_t payload_size, void *payload, void *user_data)
{
    indent();  printf("payload: \"%s\"\n", (char*)payload);
}
/*}}}*/


int main (void)
{
#if 1
    chunk *c;
    gas_context *ctx;
    gas_parser *p;

    ctx = gas_context_new();
    p = gas_parser_new(ctx, GAS_TRUE);

    p->build_tree = GAS_FALSE;
    //p->get_payloads = GAS_FALSE;

    p->on_pre_chunk = my_pre_chunk;

    p->on_push_id = my_push_id;
    p->on_payload = my_on_payload;
    p->on_attribute = my_on_attribute;
    p->on_pop_id = my_pop_id;

    //c = gas_parse(p, "dump.gas");
    //c = gas_parse(p, "xslspec.gas");
    c = gas_parse(p, "physx-test.gas");

    gas_parser_destroy(p);
    gas_print(c);
    gas_destroy(c);
    gas_context_destroy(ctx);
#endif

#if 0
    chunk *c = gas_new(0, NULL);
    gas_context *s;

    s = gas_context_new();
    gas_write_cb(c, s);
    gas_destroy(c);
#endif

    return 0;
}

// vim: set fdm=marker sw=4
