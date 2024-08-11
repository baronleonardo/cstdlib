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
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct CStr
{
  char* data;
  size_t capacity;
  size_t len;
  int err;
} CStr;

typedef struct CGrapheme
{
  size_t index;
  size_t size;
} CGrapheme;

typedef struct CCodePoint
{
  size_t index;
  size_t size;
} CCodePoint;

typedef enum
{
  CSTR_ERROR_none,
  CSTR_ERROR_allocation,
  CSTR_ERROR_wrong_capacity,
  CSTR_ERROR_wrong_length,
  CSTR_ERROR_wrong_index,
  CSTR_ERROR_capacity_full,
  CSTR_ERROR_invalid_utf8,
  CSTR_ERROR_format
} CStrError;

CStr c_str_create (char const cstr[restrict],
                   size_t cstr_len);

CStr c_str_create_empty (size_t capacity);

/// @brief
/// @param cstr
/// @param cstr_len
/// @return
/// @todo better handle unmanaged data
CStr c_str_create_unmanaged (char* restrict cstr,
                             size_t cstr_len);

CStr c_str_clone (CStr* restrict self);

char* c_str_search (CStr* restrict self, char const cstr[restrict], size_t cstr_len);

void c_str_insert (CStr* restrict self,
                   char const cstr[restrict],
                   size_t cstr_len,
                   size_t index,
                   bool could_realloc);

void c_str_remove (CStr* self,
                   char const cstr[],
                   size_t cstr_len);

size_t c_str_remove_at (CStr* restrict self,
                        size_t index,
                        size_t range);

void c_str_replace (CStr* self,
                    char const needle[],
                    size_t needle_len,
                    char const with[],
                    size_t with_len,
                    bool could_realloc);

void c_str_replace_at (CStr* restrict self,
                       size_t index,
                       size_t range,
                       char const with[restrict],
                       size_t with_len,
                       bool could_realloc);

void c_str_concatenate (CStr* restrict str1,
                        CStr const* restrict str2,
                        bool could_realloc);

void c_str_concatenate_with_cstr (CStr* restrict str1,
                                  char const cstr[restrict],
                                  size_t cstr_len,
                                  bool could_realloc);

void c_str_format (CStr* restrict self,
                   size_t format_len,
                   char const* restrict format,
                   ...);

bool c_str_utf8_is_valid (CStr* restrict self);

size_t c_str_utf8_codepoint_len (CStr* restrict self,
                                 size_t index);

void c_str_set_error_handler (void (*error_handler) (CStr* self, int err));

// size_t c_str_utf8_next_grapheme (CStr *self,
//                                  size_t index,
//                                  int *err);

size_t
c_str_len (CStr const* self);

size_t c_str_capacity (CStr const* self);

void c_str_destroy (CStr* restrict self);
#endif /* CSTDLIB_STR_H */

/* ------------------------------------------------------------------------ */
/* ---------------------------- implementation ---------------------------- */
/* ------------------------------------------------------------------------ */

#ifdef CSTDLIB_STR_IMPLEMENTATION
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define MAX_FORMAT_STR_LEN 10000
#define MAX_FORMAT_BUF_LEN 32

static char* internal_c_str_search (CStr* restrict self,
                                    char const cstr[restrict],
                                    size_t cstr_len);
// static size_t internal_cstr_len (char const cstr[],
//                                  size_t cstr_len);

static inline void internal_c_str_error_handler (CStr* self, int err);
static void (*c_str_error_handler) (CStr* self, int err) = internal_c_str_error_handler;

CStr
c_str_create (char const cstr[restrict],
              size_t cstr_len)
{
  assert (cstr);
  // assert (cstr_len > 0);

  CStr str = c_str_create_empty (cstr_len + 1);
  if (str.err == CSTR_ERROR_none)
    {

      memcpy (str.data, cstr, cstr_len);
      str.data[cstr_len] = '\0';
      str.len = cstr_len;
    }

  return str;
}

CStr
c_str_create_empty (size_t capacity)
{
  CStr str = { .capacity = capacity };

  if (capacity)
    {
      str.data = (char*) malloc (capacity);
      if (!str.data)
        {
          c_str_error_handler (&str, CSTR_ERROR_allocation);
        }
    }
  else
    {
      c_str_error_handler (&str, CSTR_ERROR_wrong_capacity);
    }

  return str;
}

CStr
c_str_create_unmanaged (char* restrict cstr,
                        size_t cstr_len)
{
  assert (cstr_len > 0);
  assert (cstr);

  CStr str = { .data = cstr };
  str.len = str.capacity = cstr_len;

  return str;
}

void
c_str_insert (CStr* restrict self,
              char const cstr[restrict],
              size_t cstr_len,
              size_t index,
              bool could_realloc)
{
  assert (self && self->data);
  assert (cstr);
  assert (cstr_len > 0);

  index %= self->len ? self->len : 1;

  if ((self->len + cstr_len + 1) > self->capacity)
    {
      if (could_realloc)
        {
          char* reallocted_data = realloc (self->data, self->capacity + cstr_len);
          if (!self->data)
            {
              c_str_error_handler (self, CSTR_ERROR_allocation);
              return;
            }

          self->data = reallocted_data;
          self->capacity += cstr_len;
        }
      else
        {
          c_str_error_handler (self, CSTR_ERROR_capacity_full);
          return;
        }
    }

  memmove (self->data + index + cstr_len, self->data + index, self->len - index + 1);
  memcpy (self->data + index, cstr, cstr_len);
  self->len += cstr_len;

  return;
}

void
c_str_remove (CStr* self,
              char const cstr[],
              size_t cstr_len)
{
  assert (self && self->data);
  assert (cstr);
  assert (cstr_len > 0);

  char* substring_ptr = internal_c_str_search (self, cstr, cstr_len);
  if (substring_ptr)
    {
      memmove (substring_ptr, substring_ptr + cstr_len,
               (self->data + self->len) - (substring_ptr + cstr_len));
      *substring_ptr = '\0';
      self->len -= cstr_len;

      return;
    }

  return;
}

size_t
c_str_remove_at (CStr* restrict self,
                 size_t index,
                 size_t range)
{
  assert (self && self->data);

  if ((index + range) >= self->len)
    {
      if (index >= self->len)
        {
          c_str_error_handler (self, CSTR_ERROR_wrong_index);
          return 0;
        }

      range = self->len - index;
    }

  memmove (self->data + index, self->data + index + range, self->len - index - range + 1);
  self->len -= self->len - index - range - 1;

  return range;
}

CStr
c_str_clone (CStr* restrict self)
{
  return c_str_create (self->data, self->len);
}

char*
c_str_search (CStr* restrict self,
              char const cstr[restrict],
              size_t cstr_len)
{
  assert (self && self->data);
  assert (cstr);
  assert (cstr_len > 0);

  return internal_c_str_search (self, cstr, cstr_len);
}

void
c_str_replace (CStr* self,
               char const needle[],
               size_t needle_len,
               char const with[],
               size_t with_len,
               bool could_realloc)
{
  char* searched_str = internal_c_str_search (self, needle, needle_len);
  if (!searched_str || needle_len == 0)
    {
      return;
    }

  c_str_replace_at (self, searched_str - self->data, needle_len, with, with_len, could_realloc);
}

void
c_str_replace_at (CStr* restrict self,
                  size_t index,
                  size_t range,
                  char const with[restrict],
                  size_t with_len,
                  bool could_realloc)
{
  assert (self && self->data);
  assert (with);
  assert (with_len > 0);

  if ((index + range) >= self->len)
    {
      range = self->len - index;
    }

  if ((self->len - range + with_len + 1) > self->capacity)
    {
      if (could_realloc)
        {
          char* reallocated_data = realloc (self->data, self->capacity - range + with_len);
          if (!self->data)
            {
              c_str_error_handler (self, CSTR_ERROR_allocation);
              return;
            }

          self->data = reallocated_data;
          self->capacity += with_len - range;
        }
      else
        {
          c_str_error_handler (self, CSTR_ERROR_capacity_full);
          return;
        }
    }

  if (with_len < range || with_len > range)
    {
      memmove (self->data + index + with_len, self->data + index + range, self->len - index - range + 1);
      self->len += (with_len < range) ? (range - with_len) : (with_len - range);
    }

  memcpy (self->data + index, with, with_len);

  return;
}

void
c_str_concatenate (CStr* restrict str1,
                   CStr const* restrict str2,
                   bool could_realloc)
{
  assert (str2);

  c_str_concatenate_with_cstr (str1, str2->data, str2->len, could_realloc);
}

void
c_str_concatenate_with_cstr (CStr* restrict str1,
                             char const cstr[restrict],
                             size_t cstr_len,
                             bool could_realloc)
{
  assert (str1 && str1->data);
  assert (cstr);
  assert (cstr_len > 0);

  if (!str1)
    {
      return;
    }

  if ((str1->len + cstr_len + 1) > str1->capacity)
    {
      if (could_realloc)
        {
          char* reallocated_data = realloc (str1->data, str1->capacity + cstr_len);
          if (!str1->data)
            {
              c_str_error_handler (str1, CSTR_ERROR_allocation);
              return;
            }

          str1->data = reallocated_data;
          str1->capacity += cstr_len;
        }
      else
        {
          c_str_error_handler (str1, CSTR_ERROR_capacity_full);
          return;
        }
    }

  memcpy (str1->data + str1->len, cstr, cstr_len);
  str1->len += cstr_len;
  str1->data[str1->len] = '\0';

  return;
}

bool
c_str_utf8_is_valid (CStr* restrict self)
{
  assert (self && self->data);

  size_t codepoint_len = 0;
  size_t total_codepoint_len = 0;
  for (size_t iii = 0; iii < self->len; iii += codepoint_len)
    {
      codepoint_len = c_str_utf8_codepoint_len (self, total_codepoint_len);
      if (!codepoint_len)
        {
          return false;
        }

      total_codepoint_len += codepoint_len;
    }

  return true;
}

size_t
c_str_utf8_codepoint_len (CStr* restrict self,
                          size_t index)
{
  assert (self && self->data);

  if (index >= self->len)
    {
      c_str_error_handler (self, CSTR_ERROR_wrong_index);
      return 0;
    }

  if (self->len == 0)
    {
      c_str_error_handler (self, CSTR_ERROR_wrong_length);
      return 0;
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
  else if ((ch & 0xFC) == 0xF8)
    {
      // Five byte character (should not occur in valid UTF-8)
      codepoint_size = 5;
    }
  else if ((ch & 0xFE) == 0xFC)
    {
      // Six byte character (should not occur in valid UTF-8)
      codepoint_size = 6;
    }
  else
    {
      // Invalid UTF-8 sequence
      c_str_error_handler (self, CSTR_ERROR_invalid_utf8);
      return 0;
    }

  // Extract the remaining bytes of the character
  for (size_t iii = 1; iii < codepoint_size; ++iii)
    {
      if (((unsigned char) (self->data[iii]) & 0xC0) != 0x80)
        {
          // Invalid UTF-8 sequence
          c_str_error_handler (self, CSTR_ERROR_invalid_utf8);
          return 0;
        }
    }

  return codepoint_size;
}

void
c_str_format (CStr* restrict self,
              size_t format_len,
              char const* restrict format,
              ...)
{
  assert (self && self->data);
  assert (format);
  assert (format_len > 0);

  char* format_ = NULL;
  if (format[format_len] != '\0')
    {
      format_ = malloc (format_len);
      if (!format_)
        {
          c_str_error_handler (self, CSTR_ERROR_allocation);
          return;
        }

      format_len = format_len - 1;
      memcpy (format_, format, format_len);
      format_[format_len - 1] = '\0';
    }

  va_list va;
  va_start (va, format);
  int res_len = vsnprintf (self->data, self->capacity, format_ ? format_ : format, va);
  va_end (va);

  if (res_len <= 0)
    {
      if (format_)
        {
          free (format_);
        }

      c_str_error_handler (self, CSTR_ERROR_format);
      return;
    }

  self->len = res_len >= (int) self->capacity ? self->capacity - 1 : (size_t) res_len;
  self->data[self->len] = '\0';

  if (format_)
    {
      free (format_);
    }

  return;
}

size_t
c_str_len (CStr const* self)
{
  assert (self && self->data);

  return self->len;
}

size_t
c_str_capacity (CStr const* self)
{
  assert (self && self->data);

  return self->capacity;
}

void
c_str_destroy (CStr* restrict self)
{
  assert (self && self->data);

  free (self->data);
  *self = (CStr){ 0 };
}

void
c_str_set_error_handler (void (*error_handler) (CStr* self, int err))
{
  if (error_handler)
    {
      c_str_error_handler = error_handler;
    }
}

/* ------------------------- internal ------------------------- */
char*
internal_c_str_search (CStr* restrict self,
                       char const cstr[restrict],
                       size_t cstr_len)
{
  for (size_t c_str_counter = 0; c_str_counter < self->len; ++c_str_counter)
    {
      if (memcmp (&(self->data)[c_str_counter], cstr, cstr_len) == 0)
        {
          return &(self->data)[c_str_counter];
        }
    }

  return NULL;
}

void
internal_c_str_error_handler (CStr* self, int err)
{
  assert (self->err == CSTR_ERROR_none);
  self->err = err;
}

// size_t
// internal_cstr_len (char const cstr[],
//                    size_t cstr_max_len)
// {
//   char* found = memchr (cstr, (int) '\0', cstr_max_len);
//   return found ? (size_t) (found - cstr) : cstr_max_len;
// }

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

#include <assert.h>
#include <string.h>

#define CSTR_STR(s) s, sizeof (s) - 1
#define CSTR_STR2(s) sizeof (s) - 1, s

void
c_str_unit_tests (void)
{
  /* test: create, remove, concatenate */
  {
    CStr str = c_str_create (CSTR_STR ("Ahmed is here"));
    assert (str.err == 0);

    c_str_remove (&str, CSTR_STR ("here"));
    assert (str.err == 0);
    assert (strcmp (str.data, "Ahmed is ") == 0);

    c_str_concatenate_with_cstr (&str, CSTR_STR ("here"), true);
    assert (str.err == 0);
    assert (strcmp (str.data, "Ahmed is here") == 0);

    c_str_destroy (&str);
  }

  /* test: empty string */
  {
    CStr str = c_str_create (CSTR_STR (""));

    assert (str.err == 0);
    assert (str.data != NULL);
    assert (strlen (str.data) == 0);
  }

  /* test: insert */
  {
    CStr str = c_str_create (CSTR_STR ("My is Mohamed"));
    assert (str.err == 0);

    c_str_insert (&str, CSTR_STR ("name "), 3, true);
    assert (str.err == 0);
    assert (strcmp (str.data, "My name is Mohamed") == 0);

    c_str_destroy (&str);
  }

  /* test: replace */
  {
    CStr str = c_str_create (CSTR_STR ("My name is Mohamed"));
    assert (str.err == 0);

    c_str_replace (&str, CSTR_STR ("name"), CSTR_STR ("game"), true);
    assert (str.err == 0);
    assert (strcmp (str.data, "My game is Mohamed") == 0);

    c_str_replace (&str, CSTR_STR ("is"), CSTR_STR ("is not"), true);
    assert (str.err == 0);
    assert (strcmp (str.data, "My game is not Mohamed") == 0);

    c_str_replace (&str, CSTR_STR ("is not"), CSTR_STR ("is"), true);
    assert (str.err == 0);
    assert (strcmp (str.data, "My game is Mohamed") == 0);

    c_str_destroy (&str);
  }

  /* test: replace_at */
  {
    CStr str = c_str_create (CSTR_STR ("My name is Mohamed"));
    assert (str.err == 0);

    c_str_replace_at (&str, 3, 4, CSTR_STR ("game"), true);
    assert (str.err == 0);
    assert (strcmp (str.data, "My game is Mohamed") == 0);

    c_str_replace_at (&str, 8, 2, CSTR_STR ("is not"), true);
    assert (str.err == 0);
    assert (strcmp (str.data, "My game is not Mohamed") == 0);

    c_str_replace_at (&str, 8, 6, CSTR_STR ("is"), true);
    assert (str.err == 0);
    assert (strcmp (str.data, "My game is Mohamed") == 0);

    c_str_destroy (&str);
  }

  /* test: concatenation */
  {
    CStr str1 = c_str_create (CSTR_STR ("Hello, "));
    assert (str1.err == 0);
    CStr str2 = c_str_create (CSTR_STR ("world!"));
    assert (str2.err == 0);

    c_str_concatenate (&str1, &str2, true);
    assert (str1.err == 0);
    assert (strcmp (str1.data, "Hello, world!") == 0);

    c_str_destroy (&str1);
    c_str_destroy (&str2);
  }

  /* test: unmanaged string */
  {
    CStr str = c_str_create_unmanaged (CSTR_STR ("hello"));
    assert (str.err == 0);

    assert (strcmp (str.data, "hello") == 0);
  }

  /* test: format */
  {
    CStr str = c_str_create_empty (100);
    assert (str.err == 0);

    c_str_format (&str, CSTR_STR2 ("smile, smile, smile, %s :), @ %d street"), "Mohamed", 32);
    assert (str.err == 0);
    assert (strcmp (str.data, "smile, smile, smile, Mohamed :), @ 32 street") == 0);

    c_str_destroy (&str);
  }

  /* test: format2 */
  {
    CStr str = c_str_create_empty (100);
    assert (str.err == 0);

    c_str_format (&str, CSTR_STR2 ("%d %s %d, %02d:%02d"), 22, "Mar", 2024, 8, 23);
    assert (str.err == 0);
    assert (strcmp (str.data, "22 Mar 2024, 08:23") == 0);

    c_str_destroy (&str);
  }

  /* test: codepoint */
  {
    CStr str = c_str_create_unmanaged (CSTR_STR ("🤦🏼‍♂️"));
    assert (str.err == 0);

    size_t next_index = 0;
    size_t const ground_truth_size[] = { 4, 4, 3, 3, 3 };
    size_t gt_index = 0;
    while (next_index < str.len)
      {
        size_t codepoint_size = c_str_utf8_codepoint_len (&str, next_index);
        assert (str.err == 0);
        assert (codepoint_size == ground_truth_size[gt_index]);
        gt_index++;

        next_index += codepoint_size;
      }
  }

  /* test: valid utf8 */
  {
    CStr str = c_str_create_unmanaged (CSTR_STR ("🤦🏼‍♂️"));
    assert (str.err == 0);

    bool is_valid = c_str_utf8_is_valid (&str);
    assert (is_valid);
  }

  /* test: invalid utf8 */
  {
    CStr str = c_str_create_unmanaged (CSTR_STR ("\xe2\x80\x8d\x99\x82\xef\xb8"));
    assert (str.err == 0);

    bool is_valid = c_str_utf8_is_valid (&str);
    assert (!is_valid);
  }

  /* test: remove at */
  {
    CStr str = c_str_create (CSTR_STR ("This is a good place!"));
    assert (str.err == 0);

    size_t removed_size = c_str_remove_at (&str, 10, 5);
    assert (str.err == 0);
    assert (removed_size == 5);
    assert (strcmp (str.data, "This is a place!") == 0);

    removed_size = c_str_remove_at (&str, 7, 100);
    assert (str.err == 0);
    assert (removed_size == 9);
    assert (strcmp (str.data, "This is") == 0);

    c_str_destroy (&str);
  }
}

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
