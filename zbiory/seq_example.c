#include "seq.h"
#include "memory_tests.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

/** MAKRA SKRACAJĄCE IMPLEMENTACJĘ TESTÓW **/

// To są możliwe wyniki testu.
#define PASS 0
#define FAIL 1
#define WRONG_TEST 2

// Oblicza liczbę elementów tablicy x.
#define SIZE(x) (sizeof x / sizeof x[0])

#define TEST_EINVAL(f)                  \
  do {                                  \
    if ((f) != -1 || errno != EINVAL)   \
      return FAIL;                      \
  } while (0)

#define TEST_NULL_EINVAL(f)             \
  do {                                  \
    if ((f) != NULL || errno != EINVAL) \
      return FAIL;                      \
  } while (0)

#define TEST_PASS(f)                    \
  do {                                  \
    if ((f) != 1)                       \
      return FAIL;                      \
  } while (0)

#define TEST_FAIL(f)                    \
  do {                                  \
    if ((f) != 0)                       \
      return FAIL;                      \
  } while (0)

#define TEST_COMP(f, s)                 \
  do {                                  \
    if (strcmp((f), (s)) != 0)          \
      return FAIL;                      \
  } while (0)

#define TEST_NULL_FAIL(f)               \
  do {                                  \
    if ((f) != NULL || errno != 0)      \
      return FAIL;                      \
  } while (0)

#define V(code, where) (((unsigned long)code) << (3 * where))

/** WŁAŚCIWE TESTY **/

// Testuje poprawność weryfikacji parametrów wywołań funkcji.
static int params(void) {
  static const char bad_seq[4] = {0, 1, 2, 3};

  seq_t *seq = seq_new();

  TEST_EINVAL(seq_add(NULL, "1"));
  TEST_EINVAL(seq_add(seq, NULL));
  TEST_EINVAL(seq_add(seq, ""));
  TEST_EINVAL(seq_add(seq, "/"));
  TEST_EINVAL(seq_add(seq, "3"));
  TEST_EINVAL(seq_add(seq, "10/"));
  TEST_EINVAL(seq_add(seq, "103"));
  TEST_EINVAL(seq_add(seq, bad_seq));

  TEST_EINVAL(seq_remove(NULL, "2"));
  TEST_EINVAL(seq_remove(seq, NULL));
  TEST_EINVAL(seq_remove(seq, ""));
  TEST_EINVAL(seq_remove(seq, "/"));
  TEST_EINVAL(seq_remove(seq, "3"));
  TEST_EINVAL(seq_remove(seq, "21/"));
  TEST_EINVAL(seq_remove(seq, "213"));
  TEST_EINVAL(seq_remove(seq, bad_seq));

  TEST_EINVAL(seq_valid(NULL, "0"));
  TEST_EINVAL(seq_valid(seq, NULL));
  TEST_EINVAL(seq_valid(seq, ""));
  TEST_EINVAL(seq_valid(seq, "/"));
  TEST_EINVAL(seq_valid(seq, "3"));
  TEST_EINVAL(seq_valid(seq, "0/0"));
  TEST_EINVAL(seq_valid(seq, "030"));
  TEST_EINVAL(seq_valid(seq, bad_seq));

  TEST_EINVAL(seq_set_name(NULL, "0", "a"));
  TEST_EINVAL(seq_set_name(seq, NULL, "b"));
  TEST_EINVAL(seq_set_name(seq, "", "c"));
  TEST_EINVAL(seq_set_name(seq, "1", NULL));
  TEST_EINVAL(seq_set_name(seq, "2", ""));
  TEST_EINVAL(seq_set_name(seq, "/", "d"));
  TEST_EINVAL(seq_set_name(seq, "3", "e"));
  TEST_EINVAL(seq_set_name(seq, bad_seq, "f"));

  TEST_NULL_EINVAL(seq_get_name(NULL, "0"));
  TEST_NULL_EINVAL(seq_get_name(seq, NULL));
  TEST_NULL_EINVAL(seq_get_name(seq, ""));
  TEST_NULL_EINVAL(seq_get_name(seq, "/"));
  TEST_NULL_EINVAL(seq_get_name(seq, "3"));
  TEST_NULL_EINVAL(seq_get_name(seq, bad_seq));

  TEST_EINVAL(seq_equiv(NULL, "0", "1"));
  TEST_EINVAL(seq_equiv(seq, NULL, "1"));
  TEST_EINVAL(seq_equiv(seq, "", "1"));
  TEST_EINVAL(seq_equiv(seq, "0", NULL));
  TEST_EINVAL(seq_equiv(seq, "0", ""));
  TEST_EINVAL(seq_equiv(seq, "/", "1"));
  TEST_EINVAL(seq_equiv(seq, "3", "1"));
  TEST_EINVAL(seq_equiv(seq, "0", "/"));
  TEST_EINVAL(seq_equiv(seq, "0", "3"));
  TEST_EINVAL(seq_equiv(seq, "0", bad_seq));
  TEST_EINVAL(seq_equiv(seq, bad_seq, "1"));

  seq_delete(seq);
  return PASS;
}

// Testuje podstawową funkcjonalność biblioteki.
static int simple(void) {
  seq_t *seq = seq_new();

  TEST_PASS(seq_add(seq, "012"));
  TEST_FAIL(seq_add(seq, "01"));
  TEST_FAIL(seq_remove(seq, "0120"));

  TEST_PASS(seq_valid(seq, "0"));
  TEST_PASS(seq_valid(seq, "01"));
  TEST_PASS(seq_valid(seq, "012"));
  TEST_FAIL(seq_valid(seq, "0120"));

  TEST_PASS(seq_remove(seq, "01"));

  TEST_PASS(seq_valid(seq, "0"));
  TEST_FAIL(seq_valid(seq, "01"));
  TEST_FAIL(seq_valid(seq, "012"));

  seq_delete(seq);
  return PASS;
}

// Testuje tworzenie klas abstrakcji i przypisywanie im nazw.
static int equivalence(void) {
  seq_t *seq = seq_new();

  TEST_FAIL(seq_equiv(seq, "0", "1"));

  TEST_PASS(seq_add(seq, "00"));
  TEST_FAIL(seq_equiv(seq, "00", "00"));
  TEST_FAIL(seq_equiv(seq, "00", "11"));

  TEST_PASS(seq_set_name(seq, "0", "zero"));
  TEST_COMP(seq_get_name(seq, "0"), "zero");
  TEST_FAIL(seq_set_name(seq, "0", "zero"));
  TEST_PASS(seq_set_name(seq, "0", "ZERO"));
  TEST_COMP(seq_get_name(seq, "0"), "ZERO");
  TEST_FAIL(seq_set_name(seq, "000", "trzy zera"));
  TEST_NULL_FAIL(seq_get_name(seq, "00"));
  TEST_NULL_FAIL(seq_get_name(seq, "1"));

  TEST_PASS(seq_add(seq, "11"));

  TEST_NULL_FAIL(seq_get_name(seq, "1"));
  TEST_NULL_FAIL(seq_get_name(seq, "11"));

  TEST_PASS(seq_equiv(seq, "0", "1"));
  TEST_FAIL(seq_equiv(seq, "0", "1"));

  TEST_COMP(seq_get_name(seq, "0"), "ZERO");
  TEST_COMP(seq_get_name(seq, "1"), "ZERO");

  TEST_PASS(seq_equiv(seq, "00", "11"));

  TEST_PASS(seq_set_name(seq, "1", "JEDEN"));
  TEST_COMP(seq_get_name(seq, "0"), "JEDEN");
  TEST_COMP(seq_get_name(seq, "1"), "JEDEN");
  TEST_PASS(seq_set_name(seq, "11", "DWA"));
  TEST_COMP(seq_get_name(seq, "00"), "DWA");
  TEST_COMP(seq_get_name(seq, "11"), "DWA");

  TEST_PASS(seq_equiv(seq, "11", "0"));
  TEST_COMP(seq_get_name(seq, "0"), "DWAJEDEN");
  TEST_COMP(seq_get_name(seq, "1"), "DWAJEDEN");
  TEST_COMP(seq_get_name(seq, "00"), "DWAJEDEN");
  TEST_COMP(seq_get_name(seq, "11"), "DWAJEDEN");

  TEST_FAIL(seq_equiv(seq, "11", "11"));

  seq_delete(seq);
  return PASS;
}

// Testuje reakcję implementacji na niepowodzenie alokacji pamięci.
static unsigned long alloc_fail_seq_new_seq_add(void) {
  unsigned long visited = 0;
  seq_t *seq;
  int result;

  if ((seq = seq_new()) != NULL)
    visited |= V(1, 0);
  else if (errno == ENOMEM && (seq = seq_new()) != NULL)
    visited |= V(2, 0);
  else
    return visited |= V(4, 0);

  if ((result = seq_add(seq, "012")) == 1)
    visited |= V(1, 1);
  else if (result == -1 && errno == ENOMEM && seq_valid(seq, "0") == 0 &&
           seq_valid(seq, "01") == 0 && seq_valid(seq, "012") == 0 &&
           seq_add(seq, "012") == 1)
    visited |= V(2, 1);
  else
    return visited |= V(4, 1);

  seq_delete(seq);

  return visited;
}

// Sprawdza reakcję implementacji na niepowodzenie alokacji pamięci.
static int memory_test(unsigned long (* test_function)(void)) {
  memory_test_data_t *mtd = get_memory_test_data();

  unsigned fail = 0, pass = 0;
  mtd->call_total = 0;
  mtd->fail_counter = 1;
  while (fail < 3 && pass < 3) {
    mtd->call_counter = 0;
    mtd->alloc_counter = 0;
    mtd->free_counter = 0;
    mtd->function_name = NULL;
    unsigned long visited_points = test_function();
    if (mtd->alloc_counter != mtd->free_counter ||
        (visited_points & 0444444444444444444444UL) != 0) {
      fprintf(stderr,
              "fail_counter %u, alloc_counter %u, free_counter %u, "
              "function_name %s, visited_point %lo\n",
              mtd->fail_counter, mtd->alloc_counter, mtd->free_counter,
              mtd->function_name, visited_points);
      ++fail;
    }
    if (mtd->function_name == NULL)
      ++pass;
    else
      pass = 0;
    mtd->fail_counter++;
  }

  return mtd->call_total > 0 && fail == 0 ? PASS : FAIL;
}

static int memory(void) {
  return memory_test(alloc_fail_seq_new_seq_add);
}

/** URUCHAMIANIE TESTÓW **/

typedef struct {
  char const *name;
  int (*function)(void);
} test_list_t;

#define TEST(t) {#t, t}

static const test_list_t test_list[] = {
  TEST(params),
  TEST(simple),
  TEST(equivalence),
  TEST(memory),
};

static int do_test(int (*function)(void)) {
  int result = function();
  puts(get_magic_string());
  return result;
}

int main(int argc, char *argv[]) {
  if (argc == 2)
    for (size_t i = 0; i < SIZE(test_list); ++i)
      if (strcmp(argv[1], test_list[i].name) == 0)
        return do_test(test_list[i].function);

  fprintf(stderr, "Użycie:\n%s nazwa_testu\n", argv[0]);
  return WRONG_TEST;
}
