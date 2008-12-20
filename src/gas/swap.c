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

/**
 * @brief Unconditional array swapping routine.
 *
 * @param buf array buffer to swap bytes
 * @param stride byte stride, 2 or 4 (short and long, respectively)
 * @param bufsize the total data length of the buffer
 *
 * @retval 0 success, otherwise failure
 */
GASresult gas_swap (GASvoid *buf, GASunum stride, GASunum bufsize)
{
    GASunum count = bufsize / stride;
    GASunum i;

    if ((bufsize % stride) != 0) {
        return GAS_ERR_INVALID_PARAM;
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
        return GAS_ERR_INVALID_PARAM;
    }
    return GAS_OK;
}

/* vim: set sw=4 fdm=marker :*/
