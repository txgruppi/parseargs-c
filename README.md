# parseargs-c

A C99 port of [parseargs-go](https://github.com/txgruppi/parseargs-go) that splits a command line into arguments with quotes and backslash escapes. It depends only on `malloc`/`free`, installs as an Arduino library, and compiles for AVR.

## Usage

```c
#include "parseargs.h"
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    char buf[] = "set name \"Put your name here\"";
    char **argv;
    int argc = parseargs(buf, &argv);

    if (argc < 0) {
        fprintf(stderr, "parse error: %d\n", argc);
        return 1;
    }

    for (int i = 0; i < argc; i++) {
        puts(argv[i]);
    }
    free(argv);
    return 0;
}
```

## Errors

| Code | Meaning |
|------|---------|
| `PARSEARGS_ERR_INVALID_ARGUMENT` (-1) | A quote appeared inside an unquoted word (e.g. `my"foo`). |
| `PARSEARGS_ERR_INVALID_SYNTAX` (-2) | A backslash before whitespace in an unquoted word (e.g. `my\ foo`). |
| `PARSEARGS_ERR_UNEXPECTED_END_OF_INPUT` (-3) | An unclosed quoted string. |
| `PARSEARGS_ERR_OUT_OF_MEMORY` (-4) | `malloc` failed when building the pointer array. |

## Memory and ownership

- The caller is responsible for calling `free(argv)`.
- The caller is responsible for keeping `buf` alive and unchanged while `argv` is in use, because each `argv[i]` points into `buf` (no string copies).
- `buf` is modified in place: tokens are NUL-terminated and escapes are removed. Use a writable buffer (not a string literal or `F()` / PROGMEM string). On error, `buf` is left unchanged and `argv` is `NULL`.

## Differences from parseargs-go

- Empty or whitespace-only input returns `0` arguments instead of an error.
- Only ASCII whitespace is trimmed at the edges; Unicode spaces (e.g. U+00A0) are not treated as trim characters.

## Tests

```sh
make test    # host tests (sanitizers)
make header  # header compiles as C and C++
```

## License

MIT