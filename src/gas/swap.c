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

#include <gas/swap.h>

#if HAVE_ASSERT_H
#include <assert.h>
#else
#define assert(expr) do {} while (0)
#endif

#ifdef GAS_DEBUG
#include <stdio.h>
#endif

int gas_swap (void *buf, size_t stride, size_t bufsize)
{
    GASunum count = bufsize / stride;
    GASunum i;

    if ((bufsize % stride) != 0) {
#ifdef GAS_DEBUG
/*        fprintf(stderr, "invalid stride for buffer size\n");*/
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
#if GAS_SIZEOF_VOID_P >= 8
    case 8: {
        uint64_t *buf64 = (uint64_t*)buf;
        for (i = 0; i < count; i++) {
            buf64[i] = swap64(buf64[i]);
        }
        break;
    }
#endif
    default:
#ifdef GAS_DEBUG
/*        fprintf(stderr, "invalid stride\n");*/
#endif
        return -1;
    }
    return 0;
}
