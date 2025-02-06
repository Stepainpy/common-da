#ifndef DA_STRING_H
#define DA_STRING_H

#include "common_da.h"

#define DS_FMT "%.*s"
#define DS_ARG(ds) (int)(ds).count, (const char*)(ds).items

typedef da_t dstring_t;

DA_DEF dstring_t ds_create_empty(void);
DA_DEF dstring_t ds_create_cstr(const char* source);
DA_DEF dstring_t ds_create_char_seq(char ch, size_t count);

DA_DEF dstring_t read_entry_file(const char* path);

#endif // DA_STRING_H

#ifdef DA_STRING_IMPLEMENTATION

#include <string.h>
#include <stdio.h>

dstring_t ds_create_empty(void) {
    return (dstring_t){
        .items = NULL,
        .count = 0,
        .capacity = 0,
        .info = {
            .size = sizeof(char),
            .dtor = NULL
        }
    };
}

dstring_t ds_create_cstr(const char* source) {
    dstring_t out = ds_create_empty();
    if (source && *source != '\0')
        da_push_back_many(&out, source, strlen(source));
    return out;
}

dstring_t ds_create_char_seq(char ch, size_t count) {
    dstring_t out = ds_create_empty();
    da_reserve(&out, count);
    memset(out.items, ch, count);
    out.count = count;
    return out;
}

dstring_t read_entry_file(const char* path) {
    dstring_t out = ds_create_empty();

    FILE* file = fopen(path, "rb");
    if (!file) return out;

    if (fseek(file, 0, SEEK_END))
        return out;
    const long l = ftell(file);
    if (l == -1L) return out;
    if (fseek(file, 0, SEEK_SET))
        return out;

    const size_t len = l;
    da_reserve(&out, len);
    out.count = fread(out.items, 1, len, file);
    fclose(file);

    return out;
}

#endif // DA_STRING_IMPLEMENTATION