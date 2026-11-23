# parseargs-c

Splits a command line into arguments with quotes and backslash escapes. C99,
depends only on `malloc`/`free`, installs as an Arduino library, and compiles
for AVR.

## Usage

```c
#include "parseargs.h"
#include <stdio.h>

int main(void)
{
    const char *line = "set name \"Put your name here\"";
    char **argv;
    int argc = parseargs_parse(line, &argv);

    if (argc < 0) {
        fprintf(stderr, "parse error: %d\n", argc);
        return 1;
    }

    for (int i = 0; i < argc; i++) {
        puts(argv[i]);
    }
    parseargs_free(argv);
    return 0;
}
```

## Errors

| Code                                         | Meaning                                                             |
| -------------------------------------------- | ------------------------------------------------------------------- |
| `PARSEARGS_ERR_INVALID_ARGUMENT` (-1)        | A quote appeared inside an unquoted word (e.g. `my"foo`).           |
| `PARSEARGS_ERR_INVALID_SYNTAX` (-2)          | A backslash before whitespace in an unquoted word (e.g. `my\ foo`). |
| `PARSEARGS_ERR_UNEXPECTED_END_OF_INPUT` (-3) | An unclosed quoted string.                                          |
| `PARSEARGS_ERR_OUT_OF_MEMORY` (-4)           | The single allocation for argv and strings failed.                  |

## Memory and ownership

- `parseargs_parse` makes one `malloc`: the `argv` pointer array and all
  argument strings live in that block. Do not free individual `argv[i]` entries.
- The caller calls `parseargs_free(argv)` once; that frees the whole block.
  `argv` ends with a `NULL` entry, as `main`'s does.
- `buf` is never modified and can be freed or reused right after the call.
- On AVR, `F()` and `PROGMEM` strings still have to be copied to RAM first,
  because the parser reads with normal pointers.

## Behavior

- Empty or whitespace-only input returns `0` arguments.
- Whitespace is ASCII only (``, `\t`, `\n`, `\v`, `\f`, `\r`); bytes `0x80` and
  above are ordinary token bytes.

## Tests

```sh
make test    # host tests (sanitizers)
make header  # header compiles as C and C++
```

## License

MIT
