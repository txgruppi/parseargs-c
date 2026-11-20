#include "parseargs.h"
#include <stdlib.h>
#include <string.h>

/* ASCII whitespace only; bytes >= 0x80 are never whitespace. */
static int is_ws(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

static int is_special(char c)
{
    return is_ws(c) || c == '"' || c == '\'';
}

static int is_escapable(char c)
{
    return is_special(c) || c == '\\';
}

struct parser {
    const char *s;
    size_t len;
    int reading;
    char start_char;
    size_t start_index;
    char **argv;
    int argc;
};

static int has_valid_backslash(const struct parser *p, size_t i)
{
    size_t n = 0;

    while (n < i && p->s[i - 1 - n] == '\\') {
        n++;
    }
    return (int)(n % 2 == 0);
}

static void unescape(char *t)
{
    char *r = t;
    char *w = t;

    while (*r != '\0') {
        if (r[0] == '\\' && is_escapable(r[1])) {
            r++;
        }
        *w++ = *r++;
    }
    *w = '\0';
}

static int emit(struct parser *p, size_t start, size_t end)
{
    size_t n;
    char *copy;

    p->reading = 0;
    p->start_char = 0;
    if (p->argv != NULL) {
        n = end - start;
        copy = (char *)malloc(n + 1);
        if (copy == NULL) {
            return -1;
        }
        if (n > 0) {
            memcpy(copy, p->s + start, n);
        }
        copy[n] = '\0';
        unescape(copy);
        p->argv[p->argc] = copy;
    }
    p->argc++;
    return 0;
}

static int run(const char *buf, char **argv)
{
    struct parser p;
    size_t i;
    size_t trim_end;

    p.argv = argv;
    p.argc = 0;
    p.reading = 0;
    p.start_char = 0;
    p.start_index = 0;
    p.s = buf;

    while (p.s[0] != '\0' && is_ws(p.s[0])) {
        p.s++;
    }

    trim_end = 0;
    while (p.s[trim_end] != '\0') {
        trim_end++;
    }
    while (trim_end > 0 && is_ws(p.s[trim_end - 1])) {
        trim_end--;
    }
    p.len = trim_end;

    if (p.len == 0) {
        return 0;
    }

    for (i = 0; i < p.len; i++) {
        char c = p.s[i];

        if (p.reading && p.start_char == ' ' && is_special(c) && !is_ws(c)) {
            return PARSEARGS_ERR_INVALID_ARGUMENT;
        }

        if (!p.reading && !is_special(c)) {
            p.reading = 1;
            p.start_char = ' ';
            p.start_index = i;
            if (i == p.len - 1) {
                if (emit(&p, p.start_index, p.len) != 0) {
                    return PARSEARGS_ERR_OUT_OF_MEMORY;
                }
            }
            continue;
        }

        if (!p.reading && is_special(c) && !is_ws(c)) {
            p.reading = 1;
            p.start_char = c;
            p.start_index = i;
            continue;
        }

        if (!p.reading) {
            continue;
        }

        if (p.start_char == ' ' && is_ws(c)) {
            if (!has_valid_backslash(&p, i)) {
                return PARSEARGS_ERR_INVALID_SYNTAX;
            }
            if (emit(&p, p.start_index, i) != 0) {
                return PARSEARGS_ERR_OUT_OF_MEMORY;
            }
            continue;
        }

        if (c == p.start_char && is_special(p.start_char) && has_valid_backslash(&p, i)) {
            if (emit(&p, p.start_index + 1, i) != 0) {
                return PARSEARGS_ERR_OUT_OF_MEMORY;
            }
            continue;
        }

        if (i == p.len - 1 && p.start_char == ' ') {
            if (emit(&p, p.start_index, p.len) != 0) {
                return PARSEARGS_ERR_OUT_OF_MEMORY;
            }
            continue;
        }
    }

    if (p.reading) {
        return PARSEARGS_ERR_UNEXPECTED_END_OF_INPUT;
    }

    return p.argc;
}

void parseargs_free(char **argv)
{
    char **p;

    if (argv == NULL) {
        return;
    }
    for (p = argv; *p != NULL; p++) {
        free(*p);
    }
    free(argv);
}

int parseargs_parse(const char *buf, char ***argv)
{
    char **arr;
    int n;
    int r;

    *argv = NULL;
    n = run(buf, NULL);
    if (n <= 0) {
        return n;
    }

    arr = (char **)calloc((size_t)n + 1, sizeof *arr);
    if (arr == NULL) {
        return PARSEARGS_ERR_OUT_OF_MEMORY;
    }

    r = run(buf, arr);
    if (r < 0) {
        parseargs_free(arr);
        return r;
    }

    *argv = arr;
    return n;
}