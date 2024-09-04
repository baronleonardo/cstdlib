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

CStr c_str_create (char const cstr[],
                   size_t cstr_len,
                   int* err);

CStr c_str_create_empty (size_t capacity,
                         int* err);

CStr c_str_create_unmanaged (char const* cstr,
                             size_t cstr_len,
                             int* err);

CStr c_str_clone (CStr* self, int* err);

char* c_str_search (CStr* self, char const cstr[], size_t cstr_len);

void c_str_insert (CStr* self,
                   char const cstr[],
                   size_t cstr_len,
                   size_t index,
                   bool could_realloc,
                   int* err);

void c_str_remove (CStr* self,
                   char const cstr[],
                   size_t cstr_len,
                   int* err);

size_t c_str_remove_at (CStr* self,
                        size_t index,
                        size_t range,
                        int* err);

void c_str_replace (CStr* self,
                    char const needle[],
                    size_t needle_len,
                    char const with[],
                    size_t with_len,
                    bool could_realloc,
                    int* err);

void c_str_replace_at (CStr* self,
                       size_t index,
                       size_t range,
                       char const with[],
                       size_t with_len,
                       bool could_realloc,
                       int* err);

void c_str_concatenate (CStr* str1,
                        CStr const* str2,
                        bool could_realloc,
                        int* err);

void c_str_concatenate_with_cstr (CStr* str1,
                                  char const cstr[],
                                  size_t cstr_len,
                                  bool could_realloc,
                                  int* err);

void c_str_format (CStr* self,
                   int* err,
                   size_t format_len,
                   char const* format,
                   ...);

bool c_str_utf8_valid (CStr* self);

size_t c_str_utf8_next_codepoint (CStr* self,
                                  size_t index,
                                  int* err);

// size_t c_str_utf8_next_grapheme (CStr *self,
//                                  size_t index,
//                                  int *err);

size_t
c_str_len (CStr const* self);

size_t c_str_capacity (CStr const* self);

void c_str_destroy (CStr* self);
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
#define ON_ERROR(error, code) (error && (*error = code))

static char* internal_c_str_search (CStr* self,
                                    char const cstr[],
                                    size_t cstr_len,
                                    size_t* return_cstr_len);
static size_t internal_cstr_len (char const cstr[],
                                 size_t cstr_len);

CStr
c_str_create (char const cstr[],
              size_t cstr_len,
              int* err)
{
  assert (cstr_len > 0);
  assert (cstr);

  CStr str = c_str_create_empty (cstr_len + 1, err);
  if (*err == CSTR_ERROR_none)
    {
      memcpy (str.data, cstr, cstr_len);

      str.data[cstr_len] = '\0';
      str.len = cstr_len;
    }

  return str;
}

CStr
c_str_create_empty (size_t capacity,
                    int* err)
{
  assert (capacity > 0);

  CStr str = { 0 };

  str.data = (char*) malloc (capacity);
  if (!str.data)
    {
      *err = -1;
      return str;
    }

  str.capacity = capacity;

  *err = 0;
  return str;
}

CStr
c_str_create_unmanaged (char const* cstr,
                        size_t cstr_len,
                        int* err)
{
  assert (cstr_len > 0);
  assert (cstr);

  CStr str = { 0 };
  int error_ = -1;
  err = err ? err : &error_;

  size_t cstr_len = internal_cstr_len (cstr, cstr_len);
  if (cstr_len == 0)
    {
      *err = -1;
      return str;
    }

  str.data = (char*) cstr;
  str.len = str.capacity = cstr_len;

  *err = 0;
  return str;
}

void
c_str_insert (CStr* self,
              char const cstr[],
              size_t cstr_len,
              size_t index,
              bool could_realloc,
              int* err)
{
  assert (self && self->data);
  assert (cstr);
  assert (cstr_len > 0);

  int error_ = -1;
  err = err ? err : &error_;

  index %= self->len;

  size_t cstr_len = internal_cstr_len (cstr, cstr_len);
  if (cstr_len == 0)
    {
      *err = -1;
      return;
    }

  if ((self->len + cstr_len + 1) > self->capacity)
    {
      if (could_realloc)
        {
          self->data = realloc (self->data, self->capacity + cstr_len);
          if (!self->data)
            {
              *err = -1;
              return;
            }

          self->capacity += cstr_len;
        }
      else
        {
          *err = -1;
          return;
        }
    }

  memmove (self->data + index + cstr_len, self->data + index, self->len - index + 1);
  memcpy (self->data + index, cstr, cstr_len);
  self->len += cstr_len;

  *err = 0;
}

void
c_str_remove (CStr* self,
              char const cstr[],
              size_t cstr_len,
              int* err)
{
  assert (self && self->data);
  assert (cstr);
  assert (cstr_len > 0);

  int error_ = -1;
  err = err ? err : &error_;

  size_t cstr_len = 0;
  char* substring_ptr = internal_c_str_search (self, cstr, cstr_len, &cstr_len);
  if (substring_ptr)
    {
      memmove (substring_ptr, substring_ptr + cstr_len,
               (self->data + self->len) - (substring_ptr + cstr_len));
      *substring_ptr = '\0';
      self->len -= cstr_len;
      *err = 0;
    }
  else
    {
      *err = 1;
    }
}

size_t
c_str_remove_at (CStr* self,
                 size_t index,
                 size_t range,
                 int* err)
{
  assert (self && self->data);

  int error_ = -1;
  err = err ? err : &error_;

  if (index >= self->len)
    {
      *err = -1;
      return 0;
    }

  if ((index + range) >= self->len)
    {
      range = self->len - index;
    }

  memmove (self->data + index, self->data + index + range, self->len - index - range + 1);
  self->len -= self->len - index - range - 1;
  *err = 0;

  return range;
}

CStr
c_str_clone (CStr* self, int* err)
{
  return c_str_create (self->data, self->len, err);
}

char*
c_str_search (CStr* self,
              char const cstr[],
              size_t cstr_len)
{
  assert (self && self->data);
  assert (cstr);
  assert (cstr_len > 0);

  return internal_c_str_search (self, cstr, cstr_len, NULL);
}

void
c_str_replace (CStr* self,
               char const needle[],
               size_t needle_len,
               char const with[],
               size_t with_len,
               bool could_realloc,
               int* err)
{
  size_t needle_len = 0;
  char* searched_str = internal_c_str_search (self, needle, needle_len, &needle_len);
  if (!searched_str || needle_len == 0)
    {
      if (err)
        *err = -1;
      return;
    }

  c_str_replace_at (self, searched_str - self->data, needle_len, with, with_len, could_realloc, err);
}

void
c_str_replace_at (CStr* self,
                  size_t index,
                  size_t range,
                  char const with[],
                  size_t with_len,
                  bool could_realloc,
                  int* err)
{
  assert (self && self->data);
  assert (with);
  assert (with_len > 0);

  int error_ = -1;
  err = err ? err : &error_;

  size_t with_len = internal_cstr_len (with, with_len);
  if (with_len == 0)
    {
      *err = -1;
      return;
    }

  if ((index + range) >= self->len)
    {
      range = self->len - index;
    }

  if ((self->len - range + with_len + 1) > self->capacity)
    {
      if (could_realloc)
        {
          self->data = realloc (self->data, self->capacity - range + with_len);
          if (!self->data)
            {
              *err = -1;
              return;
            }

          self->capacity += with_len - range;
        }
      else
        {
          *err = -1;
          return;
        }
    }

  if (with_len < range || with_len > range)
    {
      memmove (self->data + index + with_len, self->data + index + range, self->len - index - range + 1);
      self->len += (with_len < range) ? (range - with_len) : (with_len - range);
    }

  memcpy (self->data + index, with, with_len);

  *err = 0;
}

void
c_str_concatenate (CStr* str1,
                   CStr const* str2,
                   bool could_realloc,
                   int* err)
{
  assert (str2);

  c_str_concatenate_with_cstr (str1, str2->data, str2->len, could_realloc, err);
}

void
c_str_concatenate_with_cstr (CStr* str1,
                             char const cstr[],
                             size_t cstr_len,
                             bool could_realloc,
                             int* err)
{
  assert (str1 && str1->data);
  assert (cstr);
  assert (cstr_len > 0);

  int error_ = -1;
  err = err ? err : &error_;

  size_t cstr_len = internal_cstr_len (cstr, cstr_len);
  if (cstr_len == 0)
    {
      *err = -1;
      return;
    }

  if ((str1->len + cstr_len + 1) > str1->capacity)
    {
      if (could_realloc)
        {
          str1->data = realloc (str1->data, str1->capacity + cstr_len);
          if (!str1->data)
            {
              *err = -1;
              return;
            }

          str1->capacity += cstr_len;
        }
      else
        {
          *err = -1;
          return;
        }
    }

  memcpy (str1->data + str1->len, cstr, cstr_len);
  str1->len += cstr_len;
  str1->data[str1->len] = '\0';

  *err = 0;
}

bool
c_str_utf8_valid (CStr* self)
{
  assert (self && self->data);

  size_t codepoint_len = 0;
  int err = 0;
  for (size_t iii = 0; iii < self->len; iii += codepoint_len)
    {
      codepoint_len = c_str_utf8_next_codepoint (self, codepoint_len, &err);
      if (err != 0)
        {
          return false;
        }
    }

  return true;
}

size_t
c_str_utf8_next_codepoint (CStr* self,
                           size_t index,
                           int* err)
{
  assert (self && self->data);

  int error_ = -1;
  err = err ? err : &error_;

  if (index >= self->len)
    {
      *err = 1;
      return 0;
    }

  if (self->len == 0)
    {
      *err = -1;
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
      *err = -1;
      return 1;
    }

  // Extract the remaining bytes of the character
  for (size_t iii = 1; iii < codepoint_size; ++iii)
    {
      if (((unsigned char) (self->data[iii]) & 0xC0) != 0x80)
        {
          // Invalid UTF-8 sequence
          *err = -1;
          return 1;
        }
    }

  *err = 0;
  return codepoint_size;
}

void
c_str_format (CStr* self,
              int* err,
              size_t format_len,
              char const* format,
              ...)
{
  assert (self && self->data);
  assert (format);
  assert (format_len > 0);

  size_t format_len = internal_cstr_len (format, format_len);
  if (format_len == 0)
    {
      *err = -1;
      return;
    }

  int error_ = -1;
  err = err ? err : &error_;

  char* format_ = NULL;
  if (format_len < format_len)
    {
      format_ = malloc (format_len);
      if (!format_)
        {
          *err = -1;
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

      *err = -1;
      return;
    }

  self->len = res_len >= (int) self->capacity ? self->capacity - 1 : (size_t) res_len;
  self->data[self->len] = '\0';

  if (format_)
    {
      free (format_);
    }
  *err = 0;
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
c_str_destroy (CStr* self)
{
  assert (self && self->data);

  free (self->data);
  *self = (CStr){ 0 };
}

/* ------------------------- internal ------------------------- */
char*
internal_c_str_search (CStr* self,
                       char const cstr[],
                       size_t cstr_len,
                       size_t* return_cstr_len)
{
  assert (self && self->data);
  assert (cstr);
  assert (cstr_len > 0);
  assert (return_cstr_len);

  if (cstr)
    {
      size_t cstr_len = internal_cstr_len (cstr, cstr_len);
      *return_cstr_len = cstr_len;

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

size_t
internal_cstr_len (char const cstr[],
                   size_t cstr_len)
{
  char* found = memchr (cstr, (int) '\0', cstr_len);
  return found ? (size_t) (found - cstr) : cstr_len;
}

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

void
c_str_unit_tests (void)
{
  int err = 0;

  /* test: create, remove, concatenate */
  {
    CStr str = c_str_create ("Ahmed is here", 100, &err);
    assert (err == 0);

    c_str_remove (&str, "here", 100, &err);
    assert (err == 0);
    assert (strcmp (str.data, "Ahmed is ") == 0);

    c_str_concatenate_with_cstr (&str, "here", 100, true, &err);
    assert (err == 0);
    assert (strcmp (str.data, "Ahmed is here") == 0);

    c_str_destroy (&str);
  }

  /* test: empty string */
  {
    CStr str = c_str_create ("", 100, &err);

    assert (err != 0);
    assert (str.data == NULL);
  }

  /* test: insert */
  {
    CStr str = c_str_create ("My is Mohamed", 100, &err);
    assert (err == 0);

    c_str_insert (&str, "name ", 100, 3, true, &err);
    assert (err == 0);
    assert (strcmp (str.data, "My name is Mohamed") == 0);

    c_str_destroy (&str);
  }

  /* test: replace */
  {
    CStr str = c_str_create ("My name is Mohamed", 100, &err);
    assert (err == 0);

    c_str_replace (&str, "name", 100, "game", 100, true, &err);
    assert (err == 0);
    assert (strcmp (str.data, "My game is Mohamed") == 0);

    c_str_replace (&str, "is", 100, "is not", 100, true, &err);
    assert (err == 0);
    assert (strcmp (str.data, "My game is not Mohamed") == 0);

    c_str_replace (&str, "is not", 100, "is", 100, true, &err);
    assert (err == 0);
    assert (strcmp (str.data, "My game is Mohamed") == 0);

    c_str_destroy (&str);
  }

  /* test: replace_at */
  {
    CStr str = c_str_create ("My name is Mohamed", 100, &err);
    assert (err == 0);

    c_str_replace_at (&str, 3, 4, "game", 100, true, &err);
    assert (err == 0);
    assert (strcmp (str.data, "My game is Mohamed") == 0);

    c_str_replace_at (&str, 8, 2, "is not", 100, true, &err);
    assert (err == 0);
    assert (strcmp (str.data, "My game is not Mohamed") == 0);

    c_str_replace_at (&str, 8, 6, "is", 100, true, &err);
    assert (err == 0);
    assert (strcmp (str.data, "My game is Mohamed") == 0);

    c_str_destroy (&str);
  }

  /* test: concatenation */
  {
    CStr str1 = c_str_create ("Hello, ", 100, &err);
    assert (err == 0);
    CStr str2 = c_str_create ("world!", 100, &err);
    assert (err == 0);

    c_str_concatenate (&str1, &str2, true, &err);
    assert (err == 0);
    assert (strcmp (str1.data, "Hello, world!") == 0);

    c_str_destroy (&str1);
    c_str_destroy (&str2);
  }

  /* test: unmanaged string */
  {
    CStr str = c_str_create_unmanaged ("hello", 100, &err);
    assert (err == 0);

    assert (strcmp (str.data, "hello") == 0);
  }

  /* test: format */
  {
    CStr str = c_str_create_empty (100, &err);
    assert (err == 0);

    c_str_format (&str, &err, 100, "smile, smile, smile, %s :), @ %d street", "Mohamed", 32);
    assert (err == 0);
    assert (strcmp (str.data, "smile, smile, smile, Mohamed :), @ 32 street") == 0);

    c_str_destroy (&str);
  }

  /* test: format2 */
  {
    CStr str = c_str_create_empty (100, &err);
    assert (err == 0);

    c_str_format (&str, &err, 100, "%d %s %d, %02d:%02d", 22, "Mar", 2024, 8, 23);
    assert (err == 0);
    assert (strcmp (str.data, "22 Mar 2024, 08:23") == 0);

    c_str_destroy (&str);
  }

  /* test: codepoint */
  {
    CStr str = c_str_create_unmanaged ("🤦🏼‍♂️", 100, &err);
    assert (err == 0);

    size_t next_index = 0;
    size_t const ground_truth_size[] = { 4, 4, 3, 3, 3 };
    size_t gt_index = 0;
    for (size_t codepoint_size = c_str_utf8_next_codepoint (&str, next_index, &err);
         err == 0;
         codepoint_size = c_str_utf8_next_codepoint (&str, next_index, &err))
      {
        assert (err == 0);
        assert (codepoint_size == ground_truth_size[gt_index]);
        gt_index++;

        next_index += codepoint_size;
      }
  }

  /* test: valid utf8 */
  {
    CStr str = c_str_create_unmanaged ("🤦🏼‍♂️", 100, &err);
    assert (err == 0);

    bool is_valid = c_str_utf8_valid (&str);
    assert (is_valid);
  }

  /* test: invalid utf8 */
  {
    CStr str = c_str_create_unmanaged ("\xe2\x80\x8d\x99\x82\xef\xb8", 100, &err);
    assert (err == 0);

    bool is_valid = c_str_utf8_valid (&str);
    assert (!is_valid);
  }

  /* test: remove at */
  {
    CStr str = c_str_create ("This is a good place!", 100, &err);
    assert (err == 0);

    size_t removed_size = c_str_remove_at (&str, 10, 5, &err);
    assert (err == 0);
    assert (removed_size == 5);
    assert (strcmp (str.data, "This is a place!") == 0);

    removed_size = c_str_remove_at (&str, 7, 100, &err);
    assert (err == 0);
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
