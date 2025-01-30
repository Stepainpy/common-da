/* Implement common dynamic array type for C99 and later */
#ifndef COMMON_DA_H
#define COMMON_DA_H

#include <stddef.h>

// Support macros for functions declaration
#ifndef DA_DEF
#define DA_DEF
#endif

#ifndef DA_INIT_CAP
#define DA_INIT_CAP 64
#elif DA_INIT_CAP == 0
#undef DA_INIT_CAP
#define DA_INIT_CAP 64
#endif

typedef struct {
    size_t size;
    void (*dtor)(void*);
} da_type_info_t;

typedef struct {
    void* items;
    size_t count;
    size_t capacity;
    da_type_info_t info;
} da_t;

#define DA_CREATE_TINFO(type, dtor_ptr) \
((da_type_info_t){.size = sizeof(type), .dtor = (dtor_ptr)})

typedef const void* da_imm_t;

// Function declarations

DA_DEF void da_push_back(da_t* da, const void* item);
DA_DEF void da_push_back_many(da_t* da, const void* items, size_t items_count);
DA_DEF void da_clear(da_t* da);
DA_DEF void da_destroy(da_t* da);
DA_DEF void da_insert(da_t* da, const void* item, size_t index);
DA_DEF void da_insert_many(da_t* da, const void* items, size_t items_count, size_t index);
DA_DEF void da_remove(da_t* da, size_t index);
DA_DEF void da_remove_many(da_t* da, size_t i, size_t j);
DA_DEF void da_reserve(da_t* da, size_t new_cap);
DA_DEF void da_shrink_to_fit(da_t* da);

// Use only conditional 'info.size <= sizeof(da_imm_t)' is true

DA_DEF void da_push_back_imm(da_t* da, da_imm_t value);
DA_DEF void da_insert_imm(da_t* da, da_imm_t value, size_t index);

#endif // COMMON_DA_H

#ifdef DA_IMPLEMENTATION

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// Implementation detail functions

static void da_detail_realloc(da_t* da) {
    da->items = realloc(da->items, da->capacity * da->info.size);
    assert(da->items != NULL && "Not memory");
}

static void da_detail_has_one(da_t* da) {
    if (da->count < da->capacity) return;
    da->capacity += da->capacity == 0
        ? DA_INIT_CAP : (da->capacity + 1) / 2;
    da_detail_realloc(da);
}

static void da_detail_has_n(da_t* da, size_t n) {
    if (da->count + n <= da->capacity) return;
    if (da->capacity == 0) da->capacity = DA_INIT_CAP;
    while (da->count + n > da->capacity)
        da->capacity += (da->capacity + 1) / 2;
    da_detail_realloc(da);
}

// General implementations

void da_push_back(da_t* da, const void* item) {
    da_insert(da, item, da->count);
}

void da_push_back_imm(da_t* da, da_imm_t value) {
    da_insert_imm(da, value, da->count);
}

void da_push_back_many(da_t* da, const void* items, size_t items_count) {
    da_insert_many(da, items, items_count, da->count);
}

void da_clear(da_t* da) {
    if (da->info.dtor != NULL)
        for (size_t i = 0; i < da->count; i++)
            da->info.dtor((char*)da->items + i * da->info.size);
    da->count = 0;
}

void da_destroy(da_t* da) {
    da_clear(da);
    free(da->items);
    da->items = NULL;
    da->capacity = 0;
}

void da_insert(da_t* da, const void* item, size_t index) {
    assert(index <= da->count && "Out of range");
    da_detail_has_one(da);
    char* inserted = (char*)da->items + index * da->info.size;
    memmove(inserted + da->info.size, inserted,
        (da->count - index) * da->info.size);
    memcpy(inserted, item, da->info.size);
    da->count += 1;
}

void da_insert_imm(da_t* da, da_imm_t value, size_t index) {
    assert(da->info.size <= sizeof(da_imm_t)
        && "Invalid use immediately insert");
    da_insert(da, &value, index);
}

void da_insert_many(da_t* da, const void* items, size_t items_count, size_t index) {
    assert(index <= da->count && "Out of range");
    da_detail_has_n(da, items_count);
    char* first = (char*)da->items + index * da->info.size;
    char* last = first + items_count * da->info.size;
    memmove(last, first, (da->count - index) * da->info.size);
    memcpy(first, items, items_count * da->info.size);
    da->count += items_count;
}

void da_remove(da_t* da, size_t index) {
    assert(index < da->count && "Out of range");
    char* removed = (char*)da->items
        + index * da->info.size;
    if (da->info.dtor != NULL)
        da->info.dtor(removed);
    memmove(removed, removed + da->info.size,
        da->info.size * (da->count - index - 1));
    da->count -= 1;
}

void da_remove_many(da_t* da, size_t i, size_t j) {
    assert(i <= da->count && j <= da->count && "Out of range");
    assert(i <= j && "Invalid range");
    char* first = (char*)da->items + i * da->info.size;
    char* last  = (char*)da->items + j * da->info.size;
    if (da->info.dtor != NULL)
        for (char* cur = first; cur < last; cur += da->info.size)
            da->info.dtor(cur);
    memmove(first, last, da->info.size * (da->count - j));
    da->count -= j - i;
}

void da_reserve(da_t* da, size_t new_cap) {
    if (da->capacity >= new_cap) return;
    da->capacity = new_cap;
    da_detail_realloc(da);
}

void da_shrink_to_fit(da_t* da) {
    da->capacity = da->count;
    da_detail_realloc(da);
}

#endif // DA_IMPLEMENTATION