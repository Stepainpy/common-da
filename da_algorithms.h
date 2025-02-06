#ifndef DA_ALGORITHMS_H
#define DA_ALGORITHMS_H

#include "common_da.h"

// Common operations
/* Swap value `*lhs` and `*rhs` with pass size */
DA_DEF void da_alg_swap(void* lhs, void* rhs, size_t size);

// Order-changing
/* Reverse items order in `da` */
DA_DEF void da_alg_reverse(da_t* da);
/* Shuffle items in `da`, `gen` - random generator with result [a, b] */
DA_DEF void da_alg_shuffle(da_t* da, size_t (*gen)(size_t /* a */, size_t /* b */));

#endif // DA_ALGORITHMS_H

#ifdef DA_ALG_IMPLEMENTATION
#undef DA_ALG_IMPLEMENTATION

#include <string.h>

#define DA_ALG_SWAP_BUF_CAP 64

void da_alg_swap(void* lhs, void* rhs, size_t size) {
    const size_t n = size / DA_ALG_SWAP_BUF_CAP;
    const size_t m = size % DA_ALG_SWAP_BUF_CAP;
    char buf[DA_ALG_SWAP_BUF_CAP] = {0};

    for (size_t i = 0; i < n; i++) {
        memcpy(buf, lhs, DA_ALG_SWAP_BUF_CAP);
        memcpy(lhs, rhs, DA_ALG_SWAP_BUF_CAP);
        memcpy(rhs, buf, DA_ALG_SWAP_BUF_CAP);
        lhs = (char*)lhs + DA_ALG_SWAP_BUF_CAP;
        rhs = (char*)rhs + DA_ALG_SWAP_BUF_CAP;
    }

    memcpy(buf, lhs, m);
    memcpy(lhs, rhs, m);
    memcpy(rhs, buf, m);
}

void da_alg_reverse(da_t* da) {
    if (da->count == 0) return;

    char* first = da_at_fwd(da, 0);
    char* last  = da_at_fwd(da, da->count);
    for (last -= da->info.size; first < last;
        first += da->info.size, last -= da->info.size)
        da_alg_swap(first, last, da->info.size);
}

void da_alg_shuffle(da_t* da, size_t (*gen)(size_t, size_t)) {
    for (ptrdiff_t i = da->count - 1; i > 0; i--)
        da_alg_swap(
            da_at_fwd(da, i),
            da_at_fwd(da, gen(0, i)),
            da->info.size
        );
}

#endif // DA_ALG_IMPLEMENTATION