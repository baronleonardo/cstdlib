/* How To  : To use this module, do this in *ONE* C file:
 *              #define CSTDLIB_DL_LOADER_IMPLEMENTATION
 *              #include "dl_loader.h"
 * Tests   : To use run test, do this in *ONE* C file:
 *              #define CSTDLIB_DL_LOADER_UNIT_TESTS
 *              #include "dl_loader.h"
 * Options :
 *           - C_DL_LOADER_DONT_CHECK_PARAMS: parameters will not get checked
 *                                            (this is off by default)
 * License: MIT (go to the end of this file for details)
 */

/* ------------------------------------------------------------------------ */
/* -------------------------------- header -------------------------------- */
/* ------------------------------------------------------------------------ */

#ifndef CSTDLIB_DL_LOADER
#define CSTDLIB_DL_LOADER

#include <stddef.h>

typedef struct CDLLoader
{
  void* raw;
} CDLLoader;

typedef struct c_dl_error_t
{
  int code;
  char const* desc;
} c_dl_error_t;

#define C_DL_ERROR_none ((c_dl_error_t){ .code = 0, .desc = "" })
#define C_DL_ERROR_out_is_null                                                 \
  ((c_dl_error_t){ .code = 1, .desc = "ld_loader: the out pointer is NULL" })
#define C_DL_ERROR_loading                                                     \
  ((c_dl_error_t){ .code = 2,                                                  \
                   .desc = "ld_loader: failed to load the dynamic library" })
#define C_DL_ERROR_mem_allocation                                              \
  ((c_dl_error_t){ .code = 3, .desc = "memory allocation error" })
#define C_DL_ERROR_finding_symbol                                              \
  ((c_dl_error_t){ .code = 4, .desc = "ld_loader: failed to find this symbol" })
#define C_DL_LOADER_error_invalid_parameters                                   \
  ((c_dl_error_t){ .code = 5, .desc = "ld_loader: invalid parameters" })

/// @brief create a loader
/// @param file_path the dynamic library path
/// @param file_path_len the dynamic library path string length
/// @param out_dl_loader the result loader object
/// @return error
c_dl_error_t
c_dl_loader_create(char const file_path[],
                   size_t file_path_len,
                   CDLLoader* out_dl_loader);

/// @brief load a symbol from the loaded dynamic library
/// @param self the loader object
/// @param symbol_name
/// @param symbol_name_len
/// @param out_result the result symbole (function, variable, ...)
/// @return error
c_dl_error_t
c_dl_loader_get(CDLLoader* self,
                char const symbol_name[],
                size_t symbol_name_len,
                void** out_result);

/// @brief destroy the loader object
/// @param self the loader object
void
c_dl_loader_destroy(CDLLoader* self);

#endif // CSTDLIB_DL_LOADER

/* ------------------------------------------------------------------------ */
/* ---------------------------- implementation ---------------------------- */
/* ------------------------------------------------------------------------ */

#ifdef CSTDLIB_DL_LOADER_IMPLEMENTATION
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#if defined(_WIN32) && (!defined(_MSC_VER) || !(_MSC_VER >= 1900))
#error "You need MSVC must be higher that or equal to 1900"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996) // disable warning about unsafe functions
#endif

#ifndef C_DL_LOADER_DONT_CHECK_PARAMS
#define C_DL_LOADER_CHECK_PARAMS(params)                                       \
  if (!(params))                                                               \
    return C_DL_LOADER_error_invalid_parameters;
#else
#define C_DL_LOADER_CHECK_PARAMS(params) ((void)0)
#endif

c_dl_error_t
c_dl_loader_create(char const file_path[],
                   size_t file_path_len,
                   CDLLoader* out_dl_loader)
{
  C_DL_LOADER_CHECK_PARAMS(file_path && file_path_len > 0);
  C_DL_LOADER_CHECK_PARAMS(file_path[file_path_len] == '\0');
  (void)file_path_len;

  if (!out_dl_loader) {
    return C_DL_ERROR_none;
  }

  *out_dl_loader = (CDLLoader){ 0 };

#ifdef _WIN32
  out_dl_loader->raw = (void*)LoadLibraryA(file_path);
  return out_dl_loader->raw ? C_DL_ERROR_none : C_DL_ERROR_loading;
#else
  out_dl_loader->raw = dlopen(file_path, RTLD_LAZY);
  return out_dl_loader->raw ? C_DL_ERROR_none : C_DL_ERROR_loading;
#endif
}

c_dl_error_t
c_dl_loader_get(CDLLoader* self,
                char const symbol_name[],
                size_t symbol_name_len,
                void** out_result)
{
  C_DL_LOADER_CHECK_PARAMS(self && self->raw);
  C_DL_LOADER_CHECK_PARAMS(symbol_name && symbol_name_len > 0);
  (void)symbol_name_len;

  if (!out_result) {
    return C_DL_ERROR_none;
  }
#ifdef _WIN32
  *out_result = GetProcAddress(self->raw, symbol_name);
  return *out_result ? C_DL_ERROR_none : C_DL_ERROR_finding_symbol;
#else
  *out_result = dlsym(self->raw, symbol_name);
  return *out_result ? C_DL_ERROR_none : C_DL_ERROR_finding_symbol;
#endif
}

void
c_dl_loader_destroy(CDLLoader* self)
{
  if (self && self->raw) {
#ifdef _WIN32
    BOOL free_status = FreeLibrary(self->raw);
    C_DL_LOADER_CHECK_PARAMS(free_status);
#else
    int close_status = dlclose(self->raw);
    /// FIXME:
    (void)close_status;
    // C_DL_LOADER_CHECK_PARAMS (close_status == 0);
#endif

    *self = (CDLLoader){ 0 };
  }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#undef C_DL_LOADER_DONT_CHECK_PARAMS
#undef CSTDLIB_DL_LOADER_IMPLEMENTATION
#endif // CSTDLIB_DL_LOADER_IMPLEMENTATION

/* ------------------------------------------------------------------------ */
/* -------------------------------- tests --------------------------------- */
/* ------------------------------------------------------------------------ */

#ifdef CSTDLIB_DL_LOADER_UNIT_TESTS
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

#define DL_STR(str) str, (sizeof(str) - 1)
#define DL_TEST_PRINT_ABORT(msg) (fprintf(stderr, "%s\n", msg), abort())
#define DL_TEST(err)                                                           \
  (err.code != C_DL_ERROR_none.code) ? DL_TEST_PRINT_ABORT(err.desc) : (void)0
#define DL_ASSERT(cond) (!(cond)) ? DL_TEST_PRINT_ABORT(#cond) : (void)0

int
main(void)
{
  c_dl_error_t err = C_DL_ERROR_none;
  (void)err;

  {
#ifdef _WIN32
    char const lib_path[] = "test_assets/mylib.dll";
#else
    char const lib_path[] = "test_assets/libmylib.so";
#endif
    CDLLoader loader;
    err = c_dl_loader_create(DL_STR(lib_path), &loader);
    DL_TEST(err);

    int (*add)(int, int) = NULL;
    err = c_dl_loader_get(&loader, DL_STR("add"), (void**)&add);
    DL_ASSERT(add(1, 2) == 3);

    c_dl_loader_destroy(&loader);
  }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef NDEBUG_
#define NDEBUG
#undef NDEBUG_
#endif

#undef DL_STR
#undef DL_TEST_PRINT_ABORT
#undef DL_TEST
#undef DL_TEST
#undef CSTDLIB_DL_LOADER_UNIT_TESTS
#endif // CSTDLIB_DL_LOADER_UNIT_TESTS

/*
 * MIT License
 *
 * Copyright (c) 2024 Mohamed A. Elmeligy
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without ion, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions: The above copyright
 * notice and this permission notice shall be included in all copies or
 * substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS",
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
