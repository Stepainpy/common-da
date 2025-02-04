/* Implement common dynamic array type for C99 and later */
#ifndef COMMON_DA_H
#define COMMON_DA_H

#include <stddef.h>

// Macros for functions declaration
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

typedef enum {
    dae_success,
    dae_no_memory,
    dae_out_of_range,
    dae_invalid_range,
    dae_type_size_overflow
} da_error_t;

/* Convert error code to readable text */
DA_DEF const char* da_error_to_str(da_error_t error);

// Support macros

#define DA_CREATE_TINFO(type, dtor_ptr) \
((da_type_info_t){.size = sizeof(type), .dtor = (dtor_ptr)})

#define DA_CREATE_VAR(name, type, dtor_ptr) \
da_t name = {0}; name.info = DA_CREATE_TINFO(type, dtor_ptr)

#define DA_FOREACH(type, fl_var, da_ptr) \
    for ( type* fl_var = (type*)(da_ptr)->items; \
    fl_var < (type*)(da_ptr)->items + (da_ptr)->count; \
    ++fl_var )

#define DA_VOID_FOREACH(fl_var, da_ptr)  \
    for ( void* fl_var = (da_ptr)->items;  \
    (char*)fl_var < (char*)(da_ptr)->items + \
        (da_ptr)->count * (da_ptr)->info.size; \
    fl_var = (char*)fl_var + (da_ptr)->info.size )

// Function declarations

// !!! Attention !!!
// Use functions with suffix '_imm' only 
// conditional 'info.size <= sizeof(da_imm_t)' is true
typedef const void* da_imm_t;

// Access to items
/* Set pointer from `dest_ptr` as pointer to item by `index` */
DA_DEF da_error_t da_at(da_t* da, void** dest_ptr, size_t index);
/* Set pointer from `dest_ptr` as pointer to first item */
DA_DEF da_error_t da_front(da_t* da, void** dest_ptr);
/* Set pointer from `dest_ptr` as pointer to last item */
DA_DEF da_error_t da_back(da_t* da, void** dest_ptr);

// Adding items
/* Insert value into position by `index` from passed poiter */
DA_DEF da_error_t da_insert(da_t* da, const void* item, size_t index);
/* Insert value of the passed immediately type into position by `index` */
DA_DEF da_error_t da_insert_imm(da_t* da, da_imm_t value, size_t index);
/* Insert values into position by `index` from passed array */
DA_DEF da_error_t da_insert_many(da_t* da, const void* items, size_t items_count, size_t index);
/* Insert value to end from passed poiter */
DA_DEF da_error_t da_push_back(da_t* da, const void* item);
/* Insert value of the passed immediately type to end */
DA_DEF da_error_t da_push_back_imm(da_t* da, da_imm_t value);
/* Insert values to end from passed array */
DA_DEF da_error_t da_push_back_many(da_t* da, const void* items, size_t items_count);

// Removing items
// If provided destructor, call before removing
/* Remove item by `index` */
DA_DEF da_error_t da_remove(da_t* da, size_t index);
/* Remove items by indexs in range [`i`, `j`) */
DA_DEF da_error_t da_remove_many(da_t* da, size_t i, size_t j);
/* Remove the last item */
DA_DEF da_error_t da_pop_back(da_t* da);
/* Remove the `pop_count`-last items */
DA_DEF da_error_t da_pop_back_many(da_t* da, size_t pop_count);

// Deletion items
/* Remove all item and save capacity */
DA_DEF void da_clear(da_t* da);
/* Remove all item and free memory */
DA_DEF void da_destroy(da_t* da);

// Capacity manipulation
/* Reserve place for items greater or equal than `new_cap` */
DA_DEF da_error_t da_reserve(da_t* da, size_t new_cap);
/* Deallocate free capacity */
DA_DEF da_error_t da_shrink_to_fit(da_t* da);

#endif // COMMON_DA_H

#ifdef DA_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

// Implementation detail functions

static da_error_t da_detail_realloc(da_t* da) {
    da->items = realloc(da->items, da->capacity * da->info.size);
    return da->items != NULL ? dae_success : dae_no_memory;
}

static da_error_t da_detail_has_one(da_t* da) {
    if (da->count < da->capacity)
        return dae_success;
    da->capacity += da->capacity == 0
        ? DA_INIT_CAP : (da->capacity + 1) / 2;
    return da_detail_realloc(da);
}

static da_error_t da_detail_has_n(da_t* da, size_t n) {
    if (da->count + n <= da->capacity)
        return dae_success;
    if (da->capacity == 0) da->capacity = DA_INIT_CAP;
    while (da->count + n > da->capacity)
        da->capacity += (da->capacity + 1) / 2;
    return da_detail_realloc(da);
}

static void* da_detail_at(da_t* da, size_t index) {
    return (char*)da->items + index * da->info.size;
}

// General implementations

const char* da_error_to_str(da_error_t error) {
    switch (error) {
        case dae_success: return "Complete success";
        case dae_no_memory: return "Couldn't allocate memory";
        case dae_out_of_range: return "Out of the range";
        case dae_invalid_range: return "Incorrect range passed";
        case dae_type_size_overflow: return
            "The size of the stored "
            "type is larger than the "
            "size of 'da_imm_t'";
        default: return "Unknown error value";
    }
}

da_error_t da_at(da_t* da, void** dest_ptr, size_t index) {
    if (index >= da->count) return dae_out_of_range;
    *dest_ptr = da_detail_at(da, index);
    return dae_success;
}

da_error_t da_front(da_t* da, void** dest_ptr) {
    return da_at(da, dest_ptr, 0);
}

da_error_t da_back(da_t* da, void** dest_ptr) {
    return da_at(da, dest_ptr, da->count - 1);
}

da_error_t da_push_back(da_t* da, const void* item) {
    return da_insert(da, item, da->count);
}

da_error_t da_push_back_imm(da_t* da, da_imm_t value) {
    return da_insert_imm(da, value, da->count);
}

da_error_t da_push_back_many(da_t* da, const void* items, size_t items_count) {
    return da_insert_many(da, items, items_count, da->count);
}

void da_clear(da_t* da) {
    if (da->info.dtor != NULL)
        for (size_t i = 0; i < da->count; i++)
            da->info.dtor(da_detail_at(da, i));
    da->count = 0;
}

void da_destroy(da_t* da) {
    da_clear(da);
    free(da->items);
    da->items = NULL;
    da->capacity = 0;
}

da_error_t da_insert(da_t* da, const void* item, size_t index) {
    if (index > da->count)
        return dae_out_of_range;
    if (da_detail_has_one(da) == dae_no_memory)
        return dae_no_memory;

    char* inserted = da_detail_at(da, index);
    memmove(inserted + da->info.size, inserted,
        (da->count - index) * da->info.size);
    memcpy(inserted, item, da->info.size);
    da->count += 1;

    return dae_success;
}

da_error_t da_insert_imm(da_t* da, da_imm_t value, size_t index) {
    if (da->info.size > sizeof(da_imm_t))
        return dae_type_size_overflow;
    return da_insert(da, &value, index);
}

da_error_t da_insert_many(da_t* da, const void* items, size_t items_count, size_t index) {
    if (index > da->count)
        return dae_out_of_range;
    if (items_count == 0)
        return dae_success;
    if (da_detail_has_n(da, items_count) == dae_no_memory)
        return dae_no_memory;

    char* first = da_detail_at(da, index);
    char* last  = da_detail_at(da, index + items_count);
    memmove(last, first, (da->count - index) * da->info.size);
    memcpy(first, items, items_count * da->info.size);
    da->count += items_count;

    return dae_success;
}

da_error_t da_pop_back(da_t* da) {
    return da_remove(da, da->count - 1);
}

da_error_t da_pop_back_many(da_t* da, size_t pop_count) {
    return da_remove_many(da, da->count - pop_count, da->count);
}

da_error_t da_remove(da_t* da, size_t index) {
    if (index >= da->count)
        return dae_out_of_range;

    char* removed = da_detail_at(da, index);
    if (da->info.dtor != NULL)
        da->info.dtor(removed);
    memmove(removed, removed + da->info.size,
        da->info.size * (da->count - index - 1));
    da->count -= 1;

    return dae_success;
}

da_error_t da_remove_many(da_t* da, size_t i, size_t j) {
    if (i > da->count || j > da->count)
        return dae_out_of_range;
    if (i > j) return dae_invalid_range;
    if (j - i == 0)
        return dae_success;

    char* first = da_detail_at(da, i);
    char* last  = da_detail_at(da, j);
    if (da->info.dtor != NULL)
        for (size_t k = i; k < j; k++)
            da->info.dtor(da_detail_at(da, k));
    memmove(first, last, da->info.size * (da->count - j));
    da->count -= j - i;

    return dae_success;
}

da_error_t da_reserve(da_t* da, size_t new_cap) {
    return da_detail_has_n(da, new_cap);
}

da_error_t da_shrink_to_fit(da_t* da) {
    da->capacity = da->count;
    return da_detail_realloc(da);
}

#endif // DA_IMPLEMENTATION