/* How To : To use this module, do this in *ONE* C file:
 *              #define CSTDLIB_STR_IMPLEMENTATION
 *              #include "str.h"
 * Tests  : To use run tests, do this in *ONE* C file:
 *              #define CSTDLIB_STR_UNIT_TESTS
 *              #include "str.h"
 * License: MIT (go to the end of this file for details)
 */

/* ------------------------------------------------------------------------ */
/* -------------------------------- header -------------------------------- */
/* ------------------------------------------------------------------------ */

#ifndef CSTDLIB_STR_H
#define CSTDLIB_STR_H
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct CStr
{
  char* data;
  size_t capacity;
  size_t len;
} CStr;

typedef struct CStrUnmanaged
{
  char* data;
  size_t capacity;
  size_t len;
} CStrUnmanaged;

// typedef struct CGrapheme
// {
//   size_t index;
//   size_t size;
// } CGrapheme;

// typedef struct CCodePoint
// {
//   size_t index;
//   size_t size;
// } CCodePoint;

typedef struct c_str_error_t
{
  int code;
  char const* msg;
} c_str_error_t;

#define C_STR_ERROR_none ((c_str_error_t){ 0, "" })
#define C_STR_ERROR_out_is_null                                                \
  ((c_str_error_t){ 1, "str: the out pointer is NULL" })
#define C_STR_ERROR_mem_allocation                                             \
  ((c_str_error_t){ 2, "str: memory allocation error" })
#define C_STR_ERROR_wrong_len ((c_str_error_t){ 3, "str: wrong length" })
#define C_STR_ERROR_wrong_capacity ((c_str_error_t){ 4, "str: wrong capactiy" })
#define C_STR_ERROR_wrong_index ((c_str_error_t){ 5, "str: wrong index" })
#define C_STR_ERROR_capacity_full                                              \
  ((c_str_error_t){ 6, "str: capacity is full" })
#define C_STR_ERROR_invalid_utf8 ((c_str_error_t){ 7, "str: invalid utf-8" })
#define C_STR_ERROR_invalid_format ((c_str_error_t){ 8, "str: invalid format" })
#define C_STR_ERROR_needle_not_found                                           \
  ((c_str_error_t){ 9, "str: needle not found" })
#define C_STR_ERROR_invalid_parameters                                         \
  ((c_str_error_t){ 10, "str: invalid parameters" })

c_str_error_t c_str_create (char const cstr[], size_t cstr_len, CStr* out_cstr);
c_str_error_t c_str_create_unmanaged (
    char const* cstr,
    size_t cstr_len,
    void* malloc_fn (size_t),
    CStrUnmanaged* out_cstr
);

c_str_error_t c_str_create_empty (size_t capacity, CStr* out_cstr);
c_str_error_t c_str_create_empty_unmanaged (
    size_t capacity, void* malloc_fn (size_t), CStrUnmanaged* out_cstr
);

c_str_error_t c_str_clone (CStr* self, CStr* out_cstr);
c_str_error_t c_str_clone_unmanaged (
    CStrUnmanaged* self, void* malloc_fn (size_t), CStrUnmanaged* out_cstr
);

c_str_error_t c_str_find (
    CStr* self, char const cstr[], size_t cstr_len, char* out_result[]
);
c_str_error_t c_str_find_unmanaged (
    CStrUnmanaged* self, char const cstr[], size_t cstr_len, char* out_result[]
);

c_str_error_t c_str_insert (
    CStr* self, char const cstr[], size_t cstr_len, size_t index
);
c_str_error_t c_str_insert_unmanaged (
    CStrUnmanaged* self,
    char const cstr[],
    size_t cstr_len,
    size_t index,
    void* realloc_fn (void*, size_t)
);

c_str_error_t c_str_remove (CStr* self, char const cstr[], size_t cstr_len);
c_str_error_t c_str_remove_unmanaged (
    CStrUnmanaged* self,
    char const cstr[],
    size_t cstr_len,
    void* realloc_fn (void*, size_t)
);

c_str_error_t c_str_remove_at (
    CStr* self, size_t index, size_t range, size_t* out_range_size
);
c_str_error_t c_str_remove_at_unmanaged (
    CStrUnmanaged* self,
    size_t index,
    size_t range,
    void* realloc_fn (void*, size_t),
    size_t* out_range_size
);

c_str_error_t c_str_replace (
    CStr* self,
    char const needle[],
    size_t needle_len,
    char const with[],
    size_t with_len
);
c_str_error_t c_str_replace_unmanaged (
    CStrUnmanaged* self,
    char const needle[],
    size_t needle_len,
    char const with[],
    size_t with_len,
    void* realloc_fn (void*, size_t)
);

c_str_error_t c_str_replace_at (
    CStr* self, size_t index, size_t range, char const with[], size_t with_len
);
c_str_error_t c_str_replace_at_unmanaged (
    CStrUnmanaged* self,
    size_t index,
    size_t range,
    char const with[],
    size_t with_len,
    void* realloc_fn (void*, size_t)
);

c_str_error_t c_str_append (CStr* str1, CStr const* str2);
c_str_error_t c_str_append_unmanaged (
    CStrUnmanaged* str1,
    CStrUnmanaged const* str2,
    void* realloc_fn (void*, size_t)
);

c_str_error_t c_str_append_with_cstr (
    CStr* str1, char const cstr[], size_t cstr_len
);
c_str_error_t c_str_append_with_cstr_unmanaged (
    CStrUnmanaged* str1,
    char const cstr[],
    size_t cstr_len,
    void* realloc_fn (void*, size_t)
);

c_str_error_t c_str_format (
    CStr* self, size_t index, size_t format_len, char const* format, ...
);
c_str_error_t c_str_format_va (
    CStr* self, size_t index, size_t format_len, char const* format, va_list va
);
c_str_error_t c_str_format_unmanaged (
    CStrUnmanaged* self,
    void* realloc_fn (void*, size_t),
    size_t index,
    size_t format_len,
    char const* format,
    ...
);
c_str_error_t c_str_format_va_unmanaged (
    CStrUnmanaged* self,
    void* realloc_fn (void*, size_t),
    size_t index,
    size_t format_len,
    char const* format,
    va_list va
);

c_str_error_t c_str_utf8_valid (CStr* self, bool* out_is_valid);
c_str_error_t c_str_utf8_valid_unmanaged (
    CStrUnmanaged* self, bool* out_is_valid
);

c_str_error_t c_str_utf8_next_codepoint (
    CStr* self, size_t index, size_t* out_next_cp_index
);
c_str_error_t c_str_utf8_next_codepoint_unmanaged (
    CStrUnmanaged* self, size_t index, size_t* out_next_cp_index
);

// size_t c_str_utf8_next_grapheme (CStr *self,
//                                  size_t index,
//                                  int *err);
// size_t c_str_utf8_next_grapheme_unmanaged (CStrUnmanaged *self,
//                                  size_t index,
//                                  int *err);

c_str_error_t c_str_len (CStr const* self, size_t* out_len);
c_str_error_t c_str_len_unmanaged (CStrUnmanaged const* self, size_t* out_len);

c_str_error_t c_str_set_len (CStr* self, size_t len);
c_str_error_t c_str_set_len_unmanaged (
    CStrUnmanaged* self, size_t len, void* realloc_fn (void*, size_t)
);

c_str_error_t c_str_capacity (CStr const* self, size_t* out_capacity);
c_str_error_t c_str_capacity_unmanaged (
    CStrUnmanaged const* self, size_t* out_capacity
);

c_str_error_t c_str_set_capacity (CStr* self, size_t capacity);
c_str_error_t c_str_set_capacity_unmanaged (
    CStrUnmanaged* self, size_t capacity, void* realloc_fn (void*, size_t)
);

char const* c_str_get_whitespaces (void);

void c_str_destroy (CStr* self);
void c_str_destroy_unmanaged (CStrUnmanaged* self);
#endif /* CSTDLIB_STR_H */

/* ------------------------------------------------------------------------ */
/* ---------------------------- implementation ---------------------------- */
/* ------------------------------------------------------------------------ */

#ifdef CSTDLIB_STR_IMPLEMENTATION
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#ifndef C_STR_DONT_CHECK_PARAMS
#define C_STR_CHECK_PARAMS(params)                                             \
  if (!(params))                                                               \
    return C_STR_ERROR_invalid_parameters;
#else
#define C_STR_CHECK_PARAMS(params) ((void) 0)
#endif

#if _WIN32 && (!_MSC_VER || !(_MSC_VER >= 1900))
#error "You need MSVC must be higher that or equal to 1900"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996) // disable warning about unsafe functions
#endif

#define C_STR_WHITESPACES " \t\n\v\f\r"

static char* internal_c_str_find (
    CStr* self, char const cstr[], size_t cstr_len
);

c_str_error_t
c_str_create (char const cstr[], size_t cstr_len, CStr* out_cstr)
{
  return c_str_create_unmanaged (
      cstr, cstr_len, malloc, (CStrUnmanaged*) out_cstr
  );
}

c_str_error_t
c_str_create_unmanaged (
    char const cstr[],
    size_t cstr_len,
    void* malloc_fn (size_t),
    CStrUnmanaged* out_cstr
)
{
  C_STR_CHECK_PARAMS (cstr);

  if (!out_cstr)
    {
      return C_STR_ERROR_out_is_null;
    }

  c_str_error_t err =
      c_str_create_empty_unmanaged (cstr_len + 1, malloc_fn, out_cstr);
  if (err.code != C_STR_ERROR_none.code)
    {
      return C_STR_ERROR_mem_allocation;
    }

  memcpy (out_cstr->data, cstr, cstr_len);

  out_cstr->data[cstr_len] = '\0';
  out_cstr->len = cstr_len;

  return C_STR_ERROR_none;
}

c_str_error_t
c_str_create_empty (size_t capacity, CStr* out_cstr)
{
  return c_str_create_empty_unmanaged (
      capacity, malloc, (CStrUnmanaged*) out_cstr
  );
}

c_str_error_t
c_str_create_empty_unmanaged (
    size_t capacity, void* malloc_fn (size_t), CStrUnmanaged* out_cstr
)
{
  C_STR_CHECK_PARAMS (capacity > 0);

  if (!out_cstr)
    {
      return C_STR_ERROR_out_is_null;
    }

  *out_cstr = (CStrUnmanaged){ 0 };

  out_cstr->data = (char*) malloc_fn (capacity);
  if (!out_cstr->data)
    {
      return C_STR_ERROR_mem_allocation;
    }

  out_cstr->capacity = capacity;

  return C_STR_ERROR_none;
}

c_str_error_t
c_str_insert (CStr* self, char const cstr[], size_t cstr_len, size_t index)
{
  return c_str_insert_unmanaged (
      (CStrUnmanaged*) self, cstr, cstr_len, index, realloc
  );
}

c_str_error_t
c_str_insert_unmanaged (
    CStrUnmanaged* self,
    char const cstr[],
    size_t cstr_len,
    size_t index,
    void* realloc_fn (void*, size_t)
)
{
  C_STR_CHECK_PARAMS (self && self->data);
  C_STR_CHECK_PARAMS (cstr);
  C_STR_CHECK_PARAMS (cstr_len > 0);

  index %= self->len;

  if ((self->len + cstr_len + 1) > self->capacity)
    {
      char* reallocated_data =
          realloc_fn (self->data, self->capacity + cstr_len);
      if (!reallocated_data)
        {
          return C_STR_ERROR_mem_allocation;
        }

      self->data = reallocated_data;
      self->capacity += cstr_len;
    }

  memmove (
      self->data + index + cstr_len, self->data + index, self->len - index + 1
  );
  memcpy (self->data + index, cstr, cstr_len);
  self->len += cstr_len;

  return C_STR_ERROR_none;
}

c_str_error_t
c_str_remove (CStr* self, char const cstr[], size_t cstr_len)
{
  return c_str_remove_unmanaged (
      (CStrUnmanaged*) self, cstr, cstr_len, realloc
  );
}

c_str_error_t
c_str_remove_unmanaged (
    CStrUnmanaged* self,
    char const cstr[],
    size_t cstr_len,
    void* realloc_fn (void*, size_t)
)
{
  C_STR_CHECK_PARAMS (self && self->data);
  C_STR_CHECK_PARAMS (cstr);
  C_STR_CHECK_PARAMS (cstr_len > 0);

  c_str_error_t err = C_STR_ERROR_none;

  char* substring_ptr = internal_c_str_find ((CStr*) self, cstr, cstr_len);
  if (substring_ptr)
    {
      memmove (
          substring_ptr,
          substring_ptr + cstr_len,
          (self->data + self->len) - (substring_ptr + cstr_len)
      );
      *substring_ptr = '\0';
      self->len -= cstr_len;

      if ((self->len > 0) && (self->len <= self->capacity / 4))
        {
          err = c_str_set_capacity_unmanaged (
              self, self->capacity / 2, realloc_fn
          );
        }
    }

  return err;
}

c_str_error_t
c_str_remove_at (CStr* self, size_t index, size_t range, size_t* out_range_size)
{
  return c_str_remove_at_unmanaged (
      (CStrUnmanaged*) self, index, range, realloc, out_range_size
  );
}

c_str_error_t
c_str_remove_at_unmanaged (
    CStrUnmanaged* self,
    size_t index,
    size_t range,
    void* realloc_fn (void*, size_t),
    size_t* out_range_size
)
{
  C_STR_CHECK_PARAMS (self && self->data);

  if (index >= self->len)
    {
      return C_STR_ERROR_wrong_index;
    }

  if ((index + range) >= self->len)
    {
      range = self->len - index;
    }

  c_str_error_t err = C_STR_ERROR_none;

  memmove (
      self->data + index,
      self->data + index + range,
      self->len - index - range + 1
  );
  self->len -= self->len - index - range - 1;

  if ((self->len > 0) && (self->len <= self->capacity / 4))
    {
      err = c_str_set_capacity_unmanaged (self, self->capacity / 2, realloc_fn);
    }

  if (out_range_size)
    *out_range_size = range;

  return err;
}

c_str_error_t
c_str_clone (CStr* self, CStr* out_cstr)
{
  return c_str_clone_unmanaged (
      (CStrUnmanaged*) self, malloc, (CStrUnmanaged*) out_cstr
  );
}

c_str_error_t
c_str_clone_unmanaged (
    CStrUnmanaged* self, void* malloc_fn (size_t), CStrUnmanaged* out_cstr
)
{
  return c_str_create_unmanaged (self->data, self->len, malloc_fn, out_cstr);
}

c_str_error_t
c_str_find (CStr* self, char const cstr[], size_t cstr_len, char* out_result[])
{
  return c_str_find_unmanaged (
      (CStrUnmanaged*) self, cstr, cstr_len, out_result
  );
}

c_str_error_t
c_str_find_unmanaged (
    CStrUnmanaged* self, char const cstr[], size_t cstr_len, char* out_result[]
)
{
  C_STR_CHECK_PARAMS (self && self->data);
  C_STR_CHECK_PARAMS (cstr);
  C_STR_CHECK_PARAMS (cstr_len > 0);

  if (!out_result)
    {
      return C_STR_ERROR_out_is_null;
    }

  *out_result = internal_c_str_find ((CStr*) self, cstr, cstr_len);

  return C_STR_ERROR_none;
}

c_str_error_t
c_str_replace (
    CStr* self,
    char const needle[],
    size_t needle_len,
    char const with[],
    size_t with_len
)
{
  return c_str_replace_unmanaged (
      (CStrUnmanaged*) self, needle, needle_len, with, with_len, realloc
  );
}

c_str_error_t
c_str_replace_unmanaged (
    CStrUnmanaged* self,
    char const needle[],
    size_t needle_len,
    char const with[],
    size_t with_len,
    void* realloc_fn (void*, size_t)
)
{
  char* found_str = internal_c_str_find ((CStr*) self, needle, needle_len);
  if (!found_str || needle_len == 0)
    {
      return C_STR_ERROR_needle_not_found;
    }

  return c_str_replace_at_unmanaged (
      self, found_str - self->data, needle_len, with, with_len, realloc_fn
  );
}

c_str_error_t
c_str_replace_at (
    CStr* self, size_t index, size_t range, char const with[], size_t with_len
)
{
  return c_str_replace_at_unmanaged (
      (CStrUnmanaged*) self, index, range, with, with_len, realloc
  );
}

c_str_error_t
c_str_replace_at_unmanaged (
    CStrUnmanaged* self,
    size_t index,
    size_t range,
    char const with[],
    size_t with_len,
    void* realloc_fn (void*, size_t)
)
{
  C_STR_CHECK_PARAMS (self && self->data);
  C_STR_CHECK_PARAMS (with);
  C_STR_CHECK_PARAMS (with_len > 0);

  c_str_error_t err = C_STR_ERROR_none;

  if ((index + range) >= self->len)
    {
      range = self->len - index;
    }

  if ((self->len - range + with_len + 1) > self->capacity)
    {
      char* reallocated_data =
          realloc_fn (self->data, self->capacity - range + with_len);
      if (!reallocated_data)
        {
          return C_STR_ERROR_mem_allocation;
        }
      self->data = reallocated_data;
      self->capacity += with_len - range;
    }

  if (with_len < range || with_len > range)
    {
      memmove (
          self->data + index + with_len,
          self->data + index + range,
          self->len - index - range + 1
      );
      self->len -= range;
      self->len += with_len;

      if ((self->len > 0) && (self->len <= self->capacity / 4))
        {
          err = c_str_set_capacity_unmanaged (
              self, self->capacity / 2, realloc_fn
          );
        }
    }

  memcpy (self->data + index, with, with_len);
  self->data[self->len] = '\0';

  return err;
}

c_str_error_t
c_str_append (CStr* str1, CStr const* str2)
{
  return c_str_append_unmanaged (
      (CStrUnmanaged*) str1, (CStrUnmanaged*) str2, realloc
  );
}

c_str_error_t
c_str_append_unmanaged (
    CStrUnmanaged* str1,
    CStrUnmanaged const* str2,
    void* realloc_fn (void*, size_t)
)
{
  C_STR_CHECK_PARAMS (str1);
  C_STR_CHECK_PARAMS (str2 && str2->data);

  return c_str_append_with_cstr_unmanaged (
      str1, str2->data, str2->len, realloc_fn
  );
}

c_str_error_t
c_str_append_with_cstr (CStr* str1, char const cstr[], size_t cstr_len)
{
  return c_str_append_with_cstr_unmanaged (
      (CStrUnmanaged*) str1, cstr, cstr_len, realloc
  );
}

c_str_error_t
c_str_append_with_cstr_unmanaged (
    CStrUnmanaged* str1,
    char const cstr[],
    size_t cstr_len,
    void* realloc_fn (void*, size_t)
)
{
  C_STR_CHECK_PARAMS (str1 && str1->data);
  C_STR_CHECK_PARAMS (cstr);
  C_STR_CHECK_PARAMS (cstr_len > 0);

  if ((str1->len + cstr_len + 1) > str1->capacity)
    {
      char* reallocated_data =
          realloc_fn (str1->data, str1->capacity + cstr_len);
      if (!reallocated_data)
        {
          return C_STR_ERROR_mem_allocation;
        }
      str1->data = reallocated_data;
      str1->capacity += cstr_len;
    }

  memcpy (str1->data + str1->len, cstr, cstr_len);
  str1->len += cstr_len;
  str1->data[str1->len] = '\0';

  return C_STR_ERROR_none;
}

c_str_error_t
c_str_utf8_valid (CStr* self, bool* out_is_valid)
{
  return c_str_utf8_valid_unmanaged ((CStrUnmanaged*) self, out_is_valid);
}

c_str_error_t
c_str_utf8_valid_unmanaged (CStrUnmanaged* self, bool* out_is_valid)
{
  C_STR_CHECK_PARAMS (self && self->data);

  if (out_is_valid)
    {
      size_t codepoint_len = 0;
      for (size_t iii = 0; iii < self->len; iii += codepoint_len)
        {
          c_str_error_t err = c_str_utf8_next_codepoint_unmanaged (
              self, codepoint_len, &codepoint_len
          );
          if (err.code != 0)
            {
              *out_is_valid = false;
              return C_STR_ERROR_none;
            }
        }

      *out_is_valid = true;
      return C_STR_ERROR_none;
    }

  return C_STR_ERROR_out_is_null;
}

c_str_error_t
c_str_utf8_next_codepoint (CStr* self, size_t index, size_t* out_next_cp_index)
{
  return c_str_utf8_next_codepoint_unmanaged (
      (CStrUnmanaged*) self, index, out_next_cp_index
  );
}

c_str_error_t
c_str_utf8_next_codepoint_unmanaged (
    CStrUnmanaged* self, size_t index, size_t* out_next_cp_index
)
{
  C_STR_CHECK_PARAMS (self && self->data && self->len > 0);

  if (index >= self->len)
    {
      return C_STR_ERROR_wrong_index;
    }

  // Determine the number of bytes in the current character
  size_t codepoint_size = 0;
  unsigned char ch = self->data[index];
  if ((ch & 0x80) == 0)
    {
      // Single byte character
      codepoint_size = 1;
    }
  else if ((ch & 0xE0) == 0xC0)
    {
      // Two byte character
      codepoint_size = 2;
    }
  else if ((ch & 0xF0) == 0xE0)
    {
      // Three byte character
      codepoint_size = 3;
    }
  else if ((ch & 0xF8) == 0xF0)
    {
      // Four byte character
      codepoint_size = 4;
    }
  // else if ((ch & 0xFC) == 0xF8)
  //   {
  //     // Five byte character (should not occur in valid UTF-8)
  //     codepoint_size = 5;
  //   }
  // else if ((ch & 0xFE) == 0xFC)
  //   {
  //     // Six byte character (should not occur in valid UTF-8)
  //     codepoint_size = 6;
  //   }
  else
    {
      // Invalid UTF-8 sequence
      if (out_next_cp_index)
        {
          *out_next_cp_index = 1;
        }
      return C_STR_ERROR_invalid_utf8;
    }

  // Extract the remaining bytes of the character
  for (size_t iii = 1; iii < codepoint_size; ++iii)
    {
      if (((unsigned char) (self->data[iii]) & 0xC0) != 0x80)
        {
          // Invalid UTF-8 sequence
          if (out_next_cp_index)
            {
              *out_next_cp_index = 1;
            }
          return C_STR_ERROR_invalid_utf8;
        }
    }

  if (out_next_cp_index)
    {
      *out_next_cp_index = codepoint_size;
    }
  return C_STR_ERROR_none;
}

c_str_error_t
c_str_format (
    CStr* self, size_t index, size_t format_len, char const* format, ...
)
{
  va_list va;
  va_start (va, format);
  c_str_error_t err = c_str_format_va_unmanaged (
      (CStrUnmanaged*) self, realloc, index, format_len, format, va
  );
  va_end (va);

  return err;
}

c_str_error_t
c_str_format_va (
    CStr* self, size_t index, size_t format_len, char const* format, va_list va
)
{
  return c_str_format_va_unmanaged (
      (CStrUnmanaged*) self, realloc, index, format_len, format, va
  );
}

c_str_error_t
c_str_format_unmanaged (
    CStrUnmanaged* self,
    void* realloc_fn (void*, size_t),
    size_t index,
    size_t format_len,
    char const* format,
    ...
)
{
  va_list va;
  va_start (va, format);
  c_str_error_t err = c_str_format_va_unmanaged (
      (CStrUnmanaged*) self, realloc_fn, index, format_len, format, va
  );
  va_end (va);

  return err;
}

c_str_error_t
c_str_format_va_unmanaged (
    CStrUnmanaged* self,
    void* realloc_fn (void*, size_t),
    size_t index,
    size_t format_len,
    char const* format,
    va_list va
)
{
  C_STR_CHECK_PARAMS (self && self->data);
  C_STR_CHECK_PARAMS (format);
  C_STR_CHECK_PARAMS (format_len > 0);

  if (index > self->len)
    {
      return C_STR_ERROR_wrong_index;
    }

  errno = 0;
  va_list va_tmp;
  va_copy (va_tmp, va);
  int needed_len = vsnprintf (NULL, 0, format, va_tmp);
  if (needed_len < 0)
    {
      return (c_str_error_t){ .code = errno, .msg = strerror (errno) };
    }

  if ((size_t) needed_len >= self->capacity - index)
    {
      c_str_error_t err = c_str_set_capacity_unmanaged (
          self, self->capacity + needed_len + 1, realloc_fn
      );
      if (err.code != C_STR_ERROR_none.code)
        {
          return err;
        }
    }

  errno = 0;
  needed_len =
      vsnprintf (self->data + index, self->capacity - index, format, va);
  if (needed_len < 0)
    {
      return (c_str_error_t){ .code = errno, .msg = strerror (errno) };
    }

  self->len += needed_len;
  self->data[self->len] = '\0';

  return C_STR_ERROR_none;
}

c_str_error_t
c_str_len (CStr const* self, size_t* out_len)
{
  return c_str_len_unmanaged ((CStrUnmanaged*) self, out_len);
}

c_str_error_t
c_str_len_unmanaged (CStrUnmanaged const* self, size_t* out_len)
{
  C_STR_CHECK_PARAMS (self && self->data);

  if (out_len)
    {
      *out_len = self->len;
      return C_STR_ERROR_none;
    }
  else
    {
      return C_STR_ERROR_out_is_null;
    }
}

c_str_error_t
c_str_set_len (CStr* self, size_t len)
{
  return c_str_set_len_unmanaged ((CStrUnmanaged*) self, len, realloc);
}

c_str_error_t
c_str_set_len_unmanaged (
    CStrUnmanaged* self, size_t len, void* realloc_fn (void*, size_t)
)
{
  C_STR_CHECK_PARAMS (self);

  if (self->len < len)
    {
      return c_str_set_capacity_unmanaged (self, len + 1, realloc_fn);
    }

  self->len = len;
  return C_STR_ERROR_none;
}

c_str_error_t
c_str_capacity (CStr const* self, size_t* out_capacity)
{
  return c_str_capacity_unmanaged ((CStrUnmanaged*) self, out_capacity);
}

c_str_error_t
c_str_capacity_unmanaged (CStrUnmanaged const* self, size_t* out_capacity)
{
  C_STR_CHECK_PARAMS (self && self->data);

  if (out_capacity)
    {
      *out_capacity = self->capacity;
      return C_STR_ERROR_none;
    }
  else
    {
      return C_STR_ERROR_out_is_null;
    }
}

c_str_error_t
c_str_set_capacity (CStr* self, size_t capacity)
{
  return c_str_set_capacity_unmanaged (
      (CStrUnmanaged*) self, capacity, realloc
  );
}

c_str_error_t
c_str_set_capacity_unmanaged (
    CStrUnmanaged* self, size_t capacity, void* realloc_fn (void*, size_t)
)
{
  C_STR_CHECK_PARAMS (self);

  char* reallocated_data = realloc_fn (self->data, capacity);
  if (!reallocated_data)
    {
      return C_STR_ERROR_mem_allocation;
    }

  self->data = reallocated_data;
  self->capacity = capacity;

  return C_STR_ERROR_none;
}

char const*
c_str_get_whitespaces (void)
{
  return C_STR_WHITESPACES;
}

void
c_str_destroy (CStr* self)
{
  c_str_destroy_unmanaged ((CStrUnmanaged*) self);
}

void
c_str_destroy_unmanaged (CStrUnmanaged* self)
{
  if (self && self->data)
    {
      free (self->data);
      *self = (CStrUnmanaged){ 0 };
    }
}

/* ------------------------- internal ------------------------- */
char*
internal_c_str_find (CStr* self, char const cstr[], size_t cstr_len)
{
  if (cstr)
    {
      for (size_t c_str_counter = 0; c_str_counter < self->len; ++c_str_counter)
        {
          if (memcmp (&(self->data)[c_str_counter], cstr, cstr_len) == 0)
            {
              return &(self->data)[c_str_counter];
            }
        }
    }

  return NULL;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#undef CSTDLIB_STR_IMPLEMENTATION
#endif /* CSTDLIB_STR_IMPLEMENTATION */

/* ------------------------------------------------------------------------ */
/* -------------------------------- tests --------------------------------- */
/* ------------------------------------------------------------------------ */

#ifdef CSTDLIB_STR_UNIT_TESTS
#ifdef NDEBUG
#define NDEBUG_
#undef NDEBUG
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996) // disable warning about unsafe functions
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR(str) str, (sizeof (str) - 1)
#define STR_INV(str) (sizeof (str) - 1), str
#define STR_TEST_PRINT_ABORT(msg) (fprintf (stderr, "%s\n", msg), abort ())
#define STR_TEST(cond, err)                                                    \
  (!(cond) ? STR_TEST_PRINT_ABORT (err.msg) : (void) 0)
#define STR_TEST_ERR(err) (STR_TEST (err.code == 0, err))

int
main (void)
{
  c_str_error_t err = C_STR_ERROR_none;

  /* test: create, remove, append */
  {
    CStr str;
    err = c_str_create (STR ("Ahmed is here"), &str);
    STR_TEST_ERR (err);

    err = c_str_remove (&str, STR ("here"));
    STR_TEST_ERR (err);

    assert (strncmp (str.data, STR ("Ahmed is ")) == 0);

    err = c_str_append_with_cstr (&str, STR ("here"));
    STR_TEST_ERR (err);
    assert (strncmp (str.data, STR ("Ahmed is here")) == 0);

    c_str_destroy (&str);
  }

  /* test: empty string */
  {
    CStr str;
    err = c_str_create (STR (""), &str);

    STR_TEST_ERR (err);
    assert (str.data[0] == "\0"[0]);

    c_str_destroy (&str);
  }

  /* test: insert */
  {
    CStr str;
    err = c_str_create (STR ("My is Mohamed"), &str);
    STR_TEST_ERR (err);

    err = c_str_insert (&str, STR ("name "), 3);
    STR_TEST_ERR (err);
    assert (strcmp (str.data, "My name is Mohamed") == 0);

    c_str_destroy (&str);
  }

  /* test: replace */
  {
    CStr str;
    err = c_str_create (STR ("My name is Mohamed"), &str);
    STR_TEST_ERR (err);

    err = c_str_replace (&str, STR ("name"), STR ("game"));
    STR_TEST_ERR (err);
    assert (strcmp (str.data, "My game is Mohamed") == 0);

    err = c_str_replace (&str, STR ("is"), STR ("is not"));
    STR_TEST_ERR (err);
    assert (strcmp (str.data, "My game is not Mohamed") == 0);

    err = c_str_replace (&str, STR ("is not"), STR ("is"));
    STR_TEST_ERR (err);
    assert (strcmp (str.data, "My game is Mohamed") == 0);

    c_str_destroy (&str);
  }

  /* test: replace_at */
  {
    CStr str;
    err = c_str_create (STR ("My name is Mohamed"), &str);
    STR_TEST_ERR (err);

    err = c_str_replace_at (&str, 3, 4, STR ("game"));
    STR_TEST_ERR (err);
    assert (strcmp (str.data, "My game is Mohamed") == 0);

    err = c_str_replace_at (&str, 8, 2, STR ("is not"));
    STR_TEST_ERR (err);
    assert (strcmp (str.data, "My game is not Mohamed") == 0);

    err = c_str_replace_at (&str, 8, 6, STR ("is"));
    STR_TEST_ERR (err);
    assert (strcmp (str.data, "My game is Mohamed") == 0);

    c_str_destroy (&str);
  }

  /* test: concatenation */
  {
    CStr str1;
    err = c_str_create (STR ("Hello, "), &str1);
    STR_TEST_ERR (err);
    CStr str2;
    err = c_str_create (STR ("world!"), &str2);
    STR_TEST_ERR (err);

    err = c_str_append (&str1, &str2);
    STR_TEST_ERR (err);
    assert (strcmp (str1.data, "Hello, world!") == 0);

    c_str_destroy (&str1);
    c_str_destroy (&str2);
  }

  /* test: format */
  {
    CStr str;
    err = c_str_create_empty (100, &str);
    STR_TEST_ERR (err);

    err = c_str_format (
        &str,
        0,
        STR_INV ("smile, smile, smile, %s :), @ %d street"),
        "Mohamed",
        32
    );
    STR_TEST_ERR (err);
    assert (
        strcmp (str.data, "smile, smile, smile, Mohamed :), @ 32 street") == 0
    );

    c_str_destroy (&str);
  }

  /* test: format2 */
  {
    CStr str;
    err = c_str_create_empty (100, &str);
    STR_TEST_ERR (err);

    c_str_format (
        &str, 0, STR_INV ("%d %s %d, %02d:%02d"), 22, "Mar", 2024, 8, 23
    );
    STR_TEST_ERR (err);
    assert (strcmp (str.data, "22 Mar 2024, 08:23") == 0);

    c_str_destroy (&str);
  }

  /* test: codepoint */
  {
    CStr str;
    err = c_str_create (STR ("🤦🏼‍♂️"), &str);
    STR_TEST_ERR (err);

    size_t next_index = 0;
    size_t const ground_truth_size[] = { 4, 4, 3, 3, 3 };
    size_t gt_index = 0;
    size_t codepoint_size;
    for (err = c_str_utf8_next_codepoint (&str, next_index, &codepoint_size);
         err.code == 0;
         err = c_str_utf8_next_codepoint (&str, next_index, &codepoint_size))
      {
        STR_TEST_ERR (err);
        assert (codepoint_size == ground_truth_size[gt_index]);
        gt_index++;

        next_index += codepoint_size;
      }

    c_str_destroy (&str);
  }

  /* test: valid utf8 */
  {
    CStr str;
    err = c_str_create (STR ("🤦🏼‍♂️"), &str);
    STR_TEST_ERR (err);

    bool is_valid;
    err = c_str_utf8_valid (&str, &is_valid);
    assert (is_valid);

    c_str_destroy (&str);
  }

  /* test: invalid utf8 */
  {
    CStr str;
    err = c_str_create (STR ("\xe2\x80\x8d\x99\x82\xef\xb8"), &str);
    STR_TEST_ERR (err);

    bool is_valid;
    err = c_str_utf8_valid (&str, &is_valid);
    assert (!is_valid);

    c_str_destroy (&str);
  }

  /* test: remove at */
  {
    CStr str;
    err = c_str_create (STR ("This is a good place!"), &str);
    STR_TEST_ERR (err);

    size_t removed_size;
    err = c_str_remove_at (&str, 10, 5, &removed_size);
    STR_TEST_ERR (err);
    assert (removed_size == 5);
    assert (strcmp (str.data, "This is a place!") == 0);

    err = c_str_remove_at (&str, 7, 100, &removed_size);
    STR_TEST_ERR (err);
    assert (removed_size == 9);
    assert (strcmp (str.data, "This is") == 0);

    c_str_destroy (&str);
  }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef NDEBUG_
#define NDEBUG
#undef NDEBUG_
#endif

#undef CSTDLIB_STR_UNIT_TESTS
#endif /* CSTDLIB_STR_UNIT_TESTS */

/*
 * MIT License
 *
 * Copyright (c) 2024 Mohamed A. Elmeligy
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
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
