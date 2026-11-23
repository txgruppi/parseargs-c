#include "parseargs.h"
#include <stdlib.h>
#include <string.h>

/* ASCII whitespace only; bytes >= 0x80 are never whitespace. */
static int is_ws(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' ||
         c == '\r';
}

static int is_special(char c) { return is_ws(c) || c == '"' || c == '\''; }

static int is_escapable(char c) { return is_special(c) || c == '\\'; }

struct parser {
  const char *s;
  size_t len;
  int reading;
  char start_char;
  size_t start_index;
  char **argv;
  int argc;
  size_t bytes;
  char *out;
};

static int has_valid_backslash(const struct parser *p, size_t i) {
  size_t n = 0;

  while (n < i && p->s[i - 1 - n] == '\\') {
    n++;
  }
  return (int)(n % 2 == 0);
}

static void unescape(char *t) {
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

static void emit(struct parser *p, size_t start, size_t end) {
  size_t n;

  p->reading = 0;
  p->start_char = 0;
  n = end - start;
  if (p->argv != NULL) {
    if (n > 0) {
      memcpy(p->out, p->s + start, n);
    }
    p->out[n] = '\0';
    unescape(p->out);
    p->argv[p->argc] = p->out;
    p->out += strlen(p->out) + 1;
  } else {
    /* ponytail: raw span+NUL; one extra byte per removing escape (upgrade:
     * count escapes). */
    p->bytes += n + 1;
  }
  p->argc++;
}

static int run(const char *buf, char **argv, char *out, size_t *bytes_out) {
  struct parser p;
  size_t i;
  size_t trim_end;

  p.argv = argv;
  p.argc = 0;
  p.reading = 0;
  p.start_char = 0;
  p.start_index = 0;
  p.bytes = 0;
  p.out = out;
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
    if (bytes_out != NULL) {
      *bytes_out = 0;
    }
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
        emit(&p, p.start_index, p.len);
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
      emit(&p, p.start_index, i);
      continue;
    }

    if (c == p.start_char && is_special(p.start_char) &&
        has_valid_backslash(&p, i)) {
      emit(&p, p.start_index + 1, i);
      continue;
    }

    if (i == p.len - 1 && p.start_char == ' ') {
      emit(&p, p.start_index, p.len);
      continue;
    }
  }

  if (p.reading) {
    return PARSEARGS_ERR_UNEXPECTED_END_OF_INPUT;
  }

  if (bytes_out != NULL) {
    *bytes_out = p.bytes;
  }
  return p.argc;
}

void parseargs_free(char **argv) { free(argv); }

int parseargs_parse(const char *buf, char ***argv) {
  char **arr;
  size_t bytes;
  size_t slots;
  size_t total;
  char *out;
  int n;
  int r;

  *argv = NULL;
  n = run(buf, NULL, NULL, &bytes);
  if (n <= 0) {
    return n;
  }

  slots = ((size_t)n + 1) * sizeof(char *);
  total = slots + bytes;
  if (total < slots) {
    return PARSEARGS_ERR_OUT_OF_MEMORY;
  }

  arr = (char **)malloc(total);
  if (arr == NULL) {
    return PARSEARGS_ERR_OUT_OF_MEMORY;
  }

  out = (char *)arr + slots;
  r = run(buf, arr, out, NULL);
  if (r < 0) {
    free(arr);
    return r;
  }

  arr[n] = NULL;
  *argv = arr;
  return n;
}