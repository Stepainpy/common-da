#ifndef DA_ALGORITHMS_H
#define DA_ALGORITHMS_H

#include <stdbool.h>
#include "common_da.h"

#define DA_ALG_NPOS ((size_t)-1)

// Common operations
/* Swap value `*lhs` and `*rhs` with pass size */
DA_DEF void da_alg_swap(void* lhs, void* rhs, size_t size);

// Search operations
/* Find first item for `pred` is true and return his index or `DA_ALG_NPOS` */
DA_DEF size_t da_alg_find(const da_t* da, bool (*pred)(const void*));
/* Find first item for `pred` is false and return his index or `DA_ALG_NPOS` */
DA_DEF size_t da_alg_find_not(const da_t* da, bool (*pred)(const void*));

// Order-changing
/* Reorder items in `da` as first part return true from `pred` and
   second part return false. Return index of first item for `pred` equal false */
DA_DEF size_t da_alg_partition(da_t* da, bool (*pred)(const void*));
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

size_t da_alg_find(const da_t* da, bool (*pred)(const void*)) {
    size_t i = 0;
    DA_VOID_FOREACH(item, da) {
        if (pred(item))
            return i;
        ++i;
    }
    return DA_ALG_NPOS;
}

size_t da_alg_find_not(const da_t* da, bool (*pred)(const void*)) {
    size_t i = 0;
    DA_VOID_FOREACH(item, da) {
        if (!pred(item))
            return i;
        ++i;
    }
    return DA_ALG_NPOS;
}

size_t da_alg_partition(da_t* da, bool (*pred)(const void*)) {
    size_t first_i = da_alg_find_not(da, pred);
    if (first_i == DA_ALG_NPOS)
        return da->count;
    
    for (size_t i = first_i + 1; i != da->count; i++)
        if (pred(da_at_fwd(da, i))) {
            da_alg_swap(
                da_at_fwd(da, i),
                da_at_fwd(da, first_i),
                da->info.size
            );
            ++first_i;
        }

    return first_i;
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