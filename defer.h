/* How To  : `#include "defer.h"`, check Tests for examples
 * Tests   : To use run test, do this in *ONE* C file:
 *              #define CSTDLIB_DEFER_UNIT_TESTS
 *              #include "defer.h"
 * Options :
 *           - C_DEFER_MAX_DEFER_NODES: maximum defer nodes count
 *           - C_DEFER_MAX_ERROR_DEFER_NODES: maximum defer error nodes count
 * Note    : Never return from within c_guard or it will leaks (this is also
 *           true for `exit()`)
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
#ifndef C_DEFER_MAX_ERROR_DEFER_NODES
#define C_DEFER_MAX_ERROR_DEFER_NODES C_DEFER_DEFAULT_MAX_NODES
#endif

typedef struct CDeferNode
{
  void (*destructor)(void*);
  void* param;
} CDeferNode;

typedef struct CDeferStack
{
  size_t capacity;
  size_t len;
  CDeferNode* nodes;
} CDeferStack;

typedef struct CDefer
{
  bool has_error;
  CDeferStack defer_stack;
  CDeferStack defer_error_stack;
} CDefer;

#define c_guard                                                                \
  for (__c_defer_init(C_DEFER_MAX_DEFER_NODES, C_DEFER_MAX_ERROR_DEFER_NODES); \
       c_defer_var.defer_stack.capacity &&                                     \
       c_defer_var.defer_error_stack.capacity;                                 \
       __c_defer_deinit(&c_defer_var))

#define c_defer(fn, param)                                                     \
  if (c_defer_var.defer_stack.len < c_defer_var.defer_stack.capacity) {        \
    c_defer_var.defer_stack.nodes[c_defer_var.defer_stack.len++] =             \
      (CDeferNode){ fn, param };                                               \
  }

#define c_defer_err(cond, destructor, destructor_param, on_error)              \
  if (c_defer_var.defer_error_stack.len <                                      \
      c_defer_var.defer_error_stack.capacity) {                                \
    c_defer_var.defer_error_stack.nodes[c_defer_var.defer_error_stack.len++] = \
      (CDeferNode){ destructor, destructor_param };                            \
    if (!(cond)) {                                                             \
      c_defer_var.has_error = true;                                            \
      (on_error);                                                              \
      continue;                                                                \
    }                                                                          \
  }

/// internal - don't use them directly

#define __c_defer_init(defer_stack_capacity, defer_error_stack_capacity)       \
  CDefer c_defer_var = {                                                       \
    .defer_stack = { .nodes = (CDeferNode[defer_stack_capacity]){ { 0 } },     \
                     .capacity = defer_stack_capacity },                       \
    .defer_error_stack = { .nodes =                                            \
                             (CDeferNode[defer_error_stack_capacity]){         \
                               { 0 } },                                        \
                           .capacity = defer_error_stack_capacity }            \
  }

static inline void
__c_defer_deinit(CDefer* c_defer_var)
{
  for (size_t i = c_defer_var->defer_stack.len - 1;
       i < c_defer_var->defer_stack.len;
       --i) {
    c_defer_var->defer_stack.nodes[i].destructor(
      c_defer_var->defer_stack.nodes[i].param);
  }
  if (c_defer_var->has_error) {
    for (size_t i = c_defer_var->defer_error_stack.len - 1;
         i < c_defer_var->defer_error_stack.len;
         --i) {
      c_defer_var->defer_error_stack.nodes[i].destructor(
        c_defer_var->defer_error_stack.nodes[i].param);
    }
  }

  *c_defer_var = (CDefer){ 0 };
}

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

int
fn(void)
{
  int error_code = 0;
  c_guard
  {
    int* arr1 = calloc(10, sizeof(int));
    c_defer(free, arr1);

    int err = 10;
    int* arr2 = calloc(10, sizeof(int));
    // this will fail and the defer_err sequence will start
    c_defer_err(err != 10, free, arr2, error_code = -1);

    int* arr3 = calloc(10, sizeof(int));
    c_defer(free, arr3);
  }

  return error_code;
}

int
main(void)
{
  int err = fn();
  DEFER_TEST(err != 0); // this will fail on defer
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
