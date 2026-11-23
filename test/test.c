#include "parseargs.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void ok(const char *in, int n, const char **want) {
  char buf[256];
  char orig[256];
  char *sentinel;
  char **argv = &sentinel;
  int r;

  strcpy(buf, in);
  strcpy(orig, in);
  sentinel = (char *)1;
  r = parseargs_parse(buf, &argv);
  assert(strcmp(buf, orig) == 0);
  assert(r == n);
  if (n == 0) {
    assert(argv == NULL);
  } else {
    int i;

    assert(argv[n] == NULL);
    for (i = 0; i < n; i++) {
      assert(strcmp(argv[i], want[i]) == 0);
    }
    parseargs_free(argv);
  }
}

static void err(const char *in, int code) {
  char *sentinel;
  char **argv = &sentinel;
  int r;

  sentinel = (char *)1;
  r = parseargs_parse(in, &argv);
  assert(r == code);
  assert(argv == NULL);
}

int main(void) {
  ok("my string of arguments", 4,
     (const char *[]){"my", "string", "of", "arguments"});
  ok("my 'string of arguments'", 2,
     (const char *[]){"my", "string of arguments"});
  ok("\"my string\" of arguments", 3,
     (const char *[]){"my string", "of", "arguments"});
  ok("my 'string \\'of\\'' arguments", 3,
     (const char *[]){"my", "string 'of'", "arguments"});
  ok("my \"string \\\"of\\\"\" arguments", 3,
     (const char *[]){"my", "string \"of\"", "arguments"});
  ok("my 'string \"of\" arguments'", 2,
     (const char *[]){"my", "string \"of\" arguments"});
  ok("my \"string 'of' arguments\"", 2,
     (const char *[]){"my", "string 'of' arguments"});
  ok("my     string of    arguments", 4,
     (const char *[]){"my", "string", "of", "arguments"});
  ok("my\tstring\nof\rarguments", 4,
     (const char *[]){"my", "string", "of", "arguments"});
  ok("my string of arguments 0", 5,
     (const char *[]){"my", "string", "of", "arguments", "0"});

  err("my\"string\" of arguments", PARSEARGS_ERR_INVALID_ARGUMENT);
  err("my \"string of arguments", PARSEARGS_ERR_UNEXPECTED_END_OF_INPUT);
  err("my \\\\\"string\\\\\" of arguments", PARSEARGS_ERR_INVALID_ARGUMENT);
  err("my\\ string of arguments", PARSEARGS_ERR_INVALID_SYNTAX);

  ok("", 0, NULL);
  ok("  \t\n ", 0, NULL);
  ok("\"a\"b", 2, (const char *[]){"a", "b"});
  ok("'a''b'", 2, (const char *[]){"a", "b"});
  ok("a\\b", 1, (const char *[]){"a\\b"});
  ok("a\\\\b", 1, (const char *[]){"a\\b"});
  ok("\"\"", 1, (const char *[]){""});
  ok(" x ", 1, (const char *[]){"x"});
  ok("a", 1, (const char *[]){"a"});

  ok("a\vb", 2, (const char *[]){"a", "b"});
  ok("\va\v", 1, (const char *[]){"a"});
  ok("'a\\\vb'", 1, (const char *[]){"a\vb"});
  err("a\\\vb", PARSEARGS_ERR_INVALID_SYNTAX);
  ok("\xC2\xA0"
     "a",
     1,
     (const char *[]){"\xC2\xA0"
                      "a"});
  ok("\"a\\\"b\" 'c\\'d'", 2, (const char *[]){"a\"b", "c'd"});

  puts("ok");
  return 0;
}