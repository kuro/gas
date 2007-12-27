
#include <gas/swap.h>
#include <gas/gas.h>

#include <assert.h>

#ifdef DEBUG
#include <stdio.h>
#endif

int gas_swap (void *buf, size_t stride, size_t bufsize)
{
    int count = bufsize / stride;
    GASunum i;

    if ((bufsize % stride) != 0) {
#ifdef DEBUG
        fprintf(stderr, "invalid stride for buffer size\n");
#endif
        return -1;
    }

    switch (stride) {
    case 2: {
        uint16_t *buf16 = (uint16_t*)buf;
        for (i = 0; i < count; i++) {
            buf16[i] = swap16(buf16[i]);
        }
        break;
    }
    case 4: {
        uint32_t *buf32 = (uint32_t*)buf;
        for (i = 0; i < count; i++) {
            buf32[i] = swap32(buf32[i]);
        }
        break;
    }
#if SIZEOF_VOID_P >= 8
    case 8: {
        uint64_t *buf64 = (uint64_t*)buf;
        for (i = 0; i < count; i++) {
            buf64[i] = swap64(buf64[i]);
        }
        break;
    }
#endif
    default:
#ifdef DEBUG
        fprintf(stderr, "invalid stride\n");
#endif
        return -1;
    }
    return 0;
}
