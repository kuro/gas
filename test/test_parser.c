
#include <gas/parser.h>
#include <gas/ntstring.h>

GASbool my_pre_chunk (size_t id_size, void *id, void *user_data)
{
    /*printf("pre_chunk: id_size=%ld id=%s\n", id_size, (char*)id);*/

    /*
    return strcmp((char*)id, "header");
    return strcmp((char*)id, "message");
    return strcmp((char*)id, "media");
    return strcmp((char*)id, "movie");
    return GAS_FALSE;
    */
    return GAS_TRUE;
}

int main (void)
{
#if 1
    chunk *c;
    gas_session *s;
    gas_parser *p;

    s = gas_session_new();
    p = gas_parser_new(s, GAS_TRUE);

    p->on_pre_chunk = my_pre_chunk;

    c = gas_parse(p, "dump.gas");

    gas_parser_destroy(p);
    gas_print(c);
    gas_destroy(c);
    gas_session_destroy(s);
#endif

#if 0
    chunk *c = gas_new(0, NULL);
    gas_session *s;

    s = gas_session_new();
    gas_write_cb(c, s);
    gas_destroy(c);
#endif

    return 0;
}
