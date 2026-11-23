#ifndef PARSEARGS_H
#define PARSEARGS_H

#ifdef __cplusplus
extern "C" {
#endif

#define PARSEARGS_ERR_INVALID_ARGUMENT (-1)
#define PARSEARGS_ERR_INVALID_SYNTAX (-2)
#define PARSEARGS_ERR_UNEXPECTED_END_OF_INPUT (-3)
#define PARSEARGS_ERR_OUT_OF_MEMORY (-4)

/* Splits buf into arguments. Understands single and double quoted strings and
 * backslash-escaped quotes.
 *
 * Returns the argument count (>= 0) or a negative PARSEARGS_ERR_* code.
 * argv must not be NULL.
 *
 * buf is never modified.
 *
 * On success with argc > 0, *argv is a NULL-terminated array (argv[argc] is
 * NULL) pointing into one allocation: pointer slots followed by the argument
 * strings. Each argv[i] is a NUL-terminated, unescaped string in that block.
 * Do not free or realloc individual argv[i]; they are invalid after
 * parseargs_free. With argc == 0, or on error, *argv is NULL.
 *
 * The caller must call parseargs_free(*argv) when done. parseargs_free(NULL)
 * is a no-op. */
int parseargs_parse(const char *buf, char ***argv);

void parseargs_free(char **argv);

#ifdef __cplusplus
}
#endif

#endif