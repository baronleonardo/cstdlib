/* How To  : `#include "defer.h"`, check Tests for examples
 * Tests   : To use run test, do this in *ONE* C file:
 *              #define CSTDLIB_DEFER_UNIT_TESTS
 *              #include "defer.h"
 * Options :
 *           - C_DEFER_MAX_DEFER_NODES: maximum defer nodes count
 * Note    : Never return from between c_defer_init() and c_defer_deinit() or it
 *           will leaks (this is also true for `exit()`)
 * License : MIT (go to the end of this file for details)
 */

/* ------------------------------------------------------------------------ */
/* -------------------------------- header -------------------------------- */
/* ------------------------------------------------------------------------ */

#ifndef CSTDLIB_DEFER_H
#define CSTDLIB_DEFER_H

#include <stdbool.h>
#include <stddef.h>

#define C_DEFER_DEFAULT_MAX_NODES 15
#ifndef C_DEFER_MAX_DEFER_NODES
#define C_DEFER_MAX_DEFER_NODES C_DEFER_DEFAULT_MAX_NODES
#endif

typedef struct CDeferNode {
  void (*destructor)(void*);
  void* param;
} CDeferNode;

typedef struct CDefer {
  size_t      capacity;
  size_t      len;
  CDeferNode* nodes;
} CDefer;

/// @brief between this and `c_defer_deinit` you can call any `c_defer` method
/// @param defer_stack_capacity the capacity of the stack that will hold the
///                             destructors
#define c_defer_init(defer_stack_capacity)                                     \
  CDefer c_defer_var = {.nodes    = (CDeferNode[defer_stack_capacity]){{0}},   \
                        .capacity = defer_stack_capacity}

/// @brief this pushes a destructor and it parameter to stack
///        and call each one in LIFO style at when c_defer_deinit get called
/// @param destructor a function with void(*)(void*) signature (could be NULL)
/// @param destructor_param the destructor parameter
#define c_defer(destructor, destructor_param)                                  \
  if (c_defer_var.len < c_defer_var.capacity) {                                \
    c_defer_var.nodes[c_defer_var.len++]                                       \
        = (CDeferNode){(void (*)(void*))destructor, destructor_param};         \
  }

/// @brief same as c_defer_err but in `cond` failure, it fails immediately
/// @param cond a test condition
/// @param destructor a function with void(*)(void*) signature (could be NULL)
/// @param destructor_param the destructor parameter
/// @param on_error a code that will be called on failed `cond`, this will be
///                 called before the destructor in failure of `cond`
#define c_defer_err(cond, destructor, destructor_param, on_error)              \
  do {                                                                         \
    if (c_defer_var.len < c_defer_var.capacity) {                              \
      c_defer_var.nodes[c_defer_var.len++]                                     \
          = (CDeferNode){(void (*)(void*))destructor, destructor_param};       \
      if (!(cond)) {                                                           \
        do {                                                                   \
          on_error;                                                            \
        } while (0);                                                           \
        goto c_defer_deinit_label;                                             \
      }                                                                        \
    }                                                                          \
  } while (0)

/// @brief same as c_defer_err but if `cond` is false, it fails immediately
/// @param cond a test condition
/// @param destructor a function with void(*)(void*) signature (could be NULL)
/// @param destructor_param the destructor parameter
/// @param on_error a code that will be called on failed `cond`, this will be
///                 called before the destructor in failure of `cond`
#define c_defer_check(cond, destructor, destructor_param, on_error)            \
  do {                                                                         \
    if (!(cond)) {                                                             \
      do {                                                                     \
        on_error;                                                              \
      } while (0);                                                             \
      goto c_defer_deinit_label;                                               \
    }                                                                          \
  } while (0)

/// @brief this will deinitiate the c_defer and will call all destructors
#define c_defer_deinit()                                                       \
  do {                                                                         \
  c_defer_deinit_label:                                                        \
    for (size_t i = c_defer_var.len - 1; i < c_defer_var.len; --i) {           \
      if (c_defer_var.nodes[i].destructor) {                                   \
        c_defer_var.nodes[i].destructor(c_defer_var.nodes[i].param);           \
      }                                                                        \
    }                                                                          \
    c_defer_var = (CDefer){0};                                                 \
  } while (0)

#endif // CSTDLIB_DEFER_H

/* ------------------------------------------------------------------------ */
/* -------------------------------- tests --------------------------------- */
/* ------------------------------------------------------------------------ */

#ifdef CSTDLIB_DEFER_UNIT_TESTS
#ifdef NDEBUG
#define NDEBUG_
#undef NDEBUG
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996) // disable warning about unsafe functions
#endif

#include <stdio.h>
#include <stdlib.h>

#define DEFER_STR(str) str, (sizeof(str) - 1)
#define DEFER_TEST_PRINT_ABORT(msg) (fprintf(stderr, "%s\n", msg), abort())
#define DEFER_TEST(cond) (!(cond)) ? DEFER_TEST_PRINT_ABORT(#cond) : (void)0

void
destructor(void* called)
{
  *(bool*)called = true;
}

typedef struct S {
  size_t len;
  int*   data;
} S;

void
s_free(S* self)
{

  self->len = 0;
  free(self->data);
  *self = (S){0};
}

int
fn(void)
{
  int error_code = 0;

  c_defer_init(10);

  int* arr1 = calloc(10, sizeof(int));
  c_defer(free, arr1);

  S s    = {0};
  s.data = calloc(10, sizeof(*s.data));
  c_defer_err(true, s_free, &s, NULL);

  int  err       = 10;
  bool is_called = false;
  c_defer_check(err != 10, destructor, &is_called, error_code = -1);
  DEFER_TEST(!is_called);

  int* arr3 = calloc(10, sizeof(int)); // this will never get called
  c_defer(free, arr3);

  c_defer_deinit();

  return error_code;
}

int
main(void)
{
  int err = fn();
  DEFER_TEST(err != 0);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef NDEBUG_
#define NDEBUG
#undef NDEBUG_
#endif

#undef DEFER_STR
#undef DEFER_TEST_PRINT_ABORT
#undef DEFER_TEST_ECODE
#undef DEFER_TEST
#undef CSTDLIB_DEFER_UNIT_TESTS
#endif // CSTDLIB_DEFER_UNIT_TESTS

/*
 * MIT License
 *
 * Copyright (c) 2024 Mohamed A. Elmeligy
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without ion, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions: The above
 * copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS",
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
