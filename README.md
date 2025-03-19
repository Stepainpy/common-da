# Common dynamic array

## Overview

Implementation dynamic array without depend the stored type. Object of type `da_t` save information about stored type (size and destruction function). When creating new `da` need set size of stored type and destructor if necessary.

## Usage

### Creating and using `da`

``` c
DA_CREATE_VAR(da, int, NULL); // place 'da' to stack
/* ... some code ... */
size_t length = da.count; // access to field
da_some_func(&da, ...); 
```

### Access to items

- `da_at_fwd` - get pointer to item by index as return value
- `da_at` - get pointer to item by index
- `da_front` - get pointer to first item
- `da_back` - get pointer to last item

### Adding items

- `da_insert` - insert value from pointer into position by index
- `da_insert_imm` - insert value into position by index
- `da_insert_many` - insert values from array into position by index
- `da_push_back` - insert value from pointer into last position
- `da_push_back_imm` - insert value into last position
- `da_push_back_many` - insert value from array into last position

### Removing items

- `da_remove` - remove item by index
- `da_remove_many` - remove items by indexes in range
- `da_pop_back` - remove last item
- `da_pop_back_many` - remove N-last items

### Deletion items

- `da_clear` - remove all items and save capacity
- `da_destroy` - remove all items and free allocated memory

### Capacity manipulation

- `da_reserve` - reserve memory for N-items
- `da_shrink_to_fit` - deallocate free capacity

## Pre-include macros

- `DA_IMPLEMENTATION` - add definition of functions
- `DA_DEF` - User-provided attributes (as an example: `__declspec(dllexport)`)
- `DA_INIT_CAP` - default start capacity for empty `da`

## Provided macros

- `DA_CREATE_VAR` - initialize new variable of type `da_t`
- `DA_FOREACH` - expand to header (`for` with brackets) for for-loop and iterable by `da`
- `DA_VOID_FOREACH` - as `DA_FOREACH`, but use `void *` iterator variable

## Example

Print command line arguments

``` c
#define DA_IMPLEMENTATION
#include "common_da.h"
#include <stdio.h>

int main(int argc, char** argv) {
    DA_CREATE_VAR(args, const char*, NULL);
    da_error_t err;

    err = da_push_back_many(&args, argv, argc);
    if (err != dae_success) { // error check
        fprintf(stderr, "%s\n", da_error_to_str(err));
        return 1;
    }

    size_t i = 0;
    printf("argc = %zu\n", args.count);
    DA_FOREACH(const char*, arg, &args) {
        printf("argv[%zu] = %s\n", i++, *arg);
    }

    da_destroy(&args);
    return 0;
}
```

Possible output
``` console
$ ./main foo bar baz
argc = 4
argv[0] = main.exe
argv[1] = foo
argv[2] = bar
argv[3] = baz
```