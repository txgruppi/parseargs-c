#ifndef PARSEARGS_H
#define PARSEARGS_H

#ifdef __cplusplus
extern "C" {
#endif

#define PARSEARGS_ERR_INVALID_ARGUMENT         (-1)
#define PARSEARGS_ERR_INVALID_SYNTAX           (-2)
#define PARSEARGS_ERR_UNEXPECTED_END_OF_INPUT  (-3)
#define PARSEARGS_ERR_OUT_OF_MEMORY            (-4)

/* Splits buf into arguments, in place (like strtok). Understands single and
 * double quoted strings and backslash-escaped quotes.
 *
 * Returns the argument count (>= 0) or a negative PARSEARGS_ERR_* code.
 * argv must not be NULL.
 *
 * On success *argv points to a malloc'd array of argc pointers (NULL when argc == 0).
 *   - The CALLER is responsible for calling free(*argv) when done with it.
 *   - The pointers in *argv point INTO buf; no strings are copied. The CALLER is
 *     responsible for keeping buf alive and unchanged for as long as *argv is in use.
 *   - buf is modified: tokens are NUL-terminated and escapes are removed in place,
 *     so buf must be writable (not a string literal or flash string).
 *
 * On error *argv is NULL and buf is left unchanged. */
int parseargs(char *buf, char ***argv);

#ifdef __cplusplus
}
#endif

#endif