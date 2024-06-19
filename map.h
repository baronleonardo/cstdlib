/* How To : To use this module, do this in *ONE* C file:
 *              #define CSTDLIB_MAP_IMPLEMENTATION
 *              #include "map.h"
 * Tests  : To use run test, do this in *ONE* C file:
 *              #define CSTDLIB_MAP_UNIT_TESTS
 *              #include "map.h"
 * License: MIT (go to the end of this file for details)
 */

#ifndef CSTDLIB_MAP_H
#define CSTDLIB_MAP_H

#include <stdbool.h>
#include <stddef.h>

typedef struct CMap
{
  void* data;
  size_t capacity;
  size_t len;
  size_t key_size;
  size_t value_size;
} CMap;

/// @brief Every element of CMap* has shape of
///        Element {
///            bool is_filled,
///            uint8_t key[max_key_size],
///            uint8_t value[max_value_size],
///        }

CMap c_map_create (size_t max_capacity,
                   size_t max_key_size,
                   size_t max_value_size,
                   int* err);

void c_map_insert (CMap* self,
                   void* key,
                   size_t key_size,
                   void* value,
                   size_t value_size,
                   int* err);

void* c_map_get (CMap const* self,
                 void* key,
                 size_t key_size);

void* c_map_remove (CMap* self,
                    void* key,
                    size_t key_size,
                    int* err);

size_t c_map_len (CMap const* self);

void c_map_foreach (CMap* self,
                    void handler (void* key, void* value, void* extra_data),
                    void* extra_data);

void c_map_destroy (CMap* self);

#endif // CSTDLIB_MAP_H

#ifdef CSTDLIB_MAP_IMPLEMENTATION
#include <assert.h>
#include <malloc.h>

typedef struct CMapElement
{
  bool* is_filled;
  void* key;
  void* value;
} CMapElement;

typedef struct CMapElement_
{
  bool is_filled;
  void* data;
} CMapElement_;

static size_t internal_c_map_hasing_algo (void* key,
                                          size_t key_size,
                                          size_t capacity);
static CMapElement internal_c_map_get_element (CMap const* self,
                                               size_t index);
static CMapElement internal_c_map_search_and_get (CMap const* self,
                                                  void* key,
                                                  size_t key_size);

CMap
c_map_create (size_t max_capacity,
              size_t max_key_size,
              size_t max_value_size,
              int* err)
{
  assert (max_key_size > 0);
  assert (max_value_size > 0);
  assert (max_capacity > 0);

  int error_;
  err = err ? err : &error_;
  *err = 0;

  CMap map = {
    .data = calloc (
        1, sizeof (CMap) + (max_capacity * (max_key_size + max_value_size + sizeof (bool) + sizeof (void*)))),
    .capacity = max_capacity,
    .key_size = max_key_size,
    .value_size = max_value_size,
  };

  if (!map.data)
    {
      *err = -1;
      return (CMap){ 0 };
    }

  return map;
}

void
c_map_insert (CMap* self,
              void* key,
              size_t key_size,
              void* value,
              size_t value_size,
              int* err)
{
  assert (self && self->data);
  assert (key_size > 0);
  assert (value);
  assert (value_size > 0);
  assert (key_size <= self->key_size);
  assert (value_size <= self->value_size);

  int error_;
  err = err ? err : &error_;
  *err = 0;

  if (self->capacity <= self->len)
    {
      *err = -1;
      return;
    }

  size_t index = internal_c_map_hasing_algo (key, key_size, self->capacity);
  CMapElement element = internal_c_map_get_element (self, index);

  // collision
  // find an empty slot using `Quadratic Probing`
  if (*element.is_filled && (memcmp (key, element.key, key_size) != 0))
    {
      for (size_t iii = 1; *element.is_filled; ++iii)
        {
          index = (index + (iii * iii)) % self->capacity;
          element = internal_c_map_get_element (self, index);
        }
    }

  assert (memcpy (element.key, key, key_size));
  assert (memcpy (element.value, value, value_size));
  *element.is_filled = true;

  self->len++;
}

void*
c_map_get (CMap const* self,
           void* key,
           size_t key_size)
{
  assert (self && self->data);
  assert (key);
  assert (key_size > 0);

  CMapElement element = internal_c_map_search_and_get (self, key, key_size);
  if (element.key != NULL)
    {
      return (uint8_t*) (element.value);
    }
  else
    {
      return NULL;
    }
}

void*
c_map_remove (CMap* self,
              void* key,
              size_t key_size,
              int* err)
{
  assert (self && self->data);
  assert (key);
  assert (key_size > 0);

  int error_;
  err = err ? err : &error_;
  *err = 0;

  if (key_size > self->key_size)
    {
      *err = -1;
      return NULL;
    }

  CMapElement element = internal_c_map_search_and_get (self, key, key_size);
  if (element.key)
    {

      self->len--;
      *element.is_filled = false;
      assert (memset (element.key, 0, self->key_size));
      return element.value;
    }
  else
    {
      return NULL;
    }
}

size_t
c_map_len (CMap const* self)
{
  assert (self && self->data);

  return self->len;
}

void
c_map_foreach (CMap* self,
               void handler (void* key, void* value, void* extra_data),
               void* extra_data)
{
  assert (self && self->data);
  assert (handler);

  for (size_t iii = 0; iii < self->len; ++iii)
    {
      CMapElement element = internal_c_map_get_element (self, iii);
      handler (element.key, element.value, extra_data);
    }
}

void
c_map_destroy (CMap* self)
{
  assert (self && self->data);

  free (self->data);

  *self = (CMap){ 0 };
}

// ------------------------- internal ------------------------- //
static inline size_t
internal_c_map_hasing_algo (void* key, size_t key_size, size_t capacity)
{
  size_t sum = 0;
  for (size_t iii = 0; iii < key_size; ++iii)
    {
      sum += ((uint8_t*) key)[iii];
    }

  return sum % capacity;
}

static inline CMapElement
internal_c_map_get_element (CMap const* self, size_t index)
{
  bool* is_filled_addr = (bool*) self->data + (index * (self->key_size + self->value_size + sizeof (bool)));
  uint8_t* key_addr = (uint8_t*) is_filled_addr + sizeof (bool);
  uint8_t* value_addr = key_addr + self->key_size;

  CMapElement element = { .is_filled = is_filled_addr,
                          .key = key_addr,
                          .value = value_addr };

  return element;
}

static inline CMapElement
internal_c_map_search_and_get (CMap const* self, void* key, size_t key_size)
{
  assert (self);

  if (key)
    {
      size_t index = internal_c_map_hasing_algo (key, key_size, self->capacity);
      CMapElement element = internal_c_map_get_element (self, index);

      if (*element.is_filled)
        {
          void* element_key = element.key;

          // check for collision `Quadratic Probing`
          for (size_t iii = 0; memcmp (key, element_key, key_size) != 0;
               ++iii)
            {
              if (!*element.is_filled)
                {
                  return (CMapElement){ 0 };
                }

              index = (index + (iii * iii)) % self->capacity;
              element = internal_c_map_get_element (self, index);
              element_key = element.key;
            }

          return element;
        }
    }

  return (CMapElement){ 0 };
}

#undef CSTDLIB_MAP_IMPLEMENTATION
#endif // CSTDLIB_MAP_IMPLEMENTATION

#ifdef CSTDLIB_MAP_UNIT_TESTS
#ifdef NDEBUG
#define NDEBUG_
#undef NDEBUG
#endif

#include <assert.h>

#define STR_W_LEN(str) str, sizeof (str)
#define INT_W_LEN(i) &(int){ i }, sizeof (int)

int err = 0;

void
c_map_unit_tests (void)
{

  CMap map = c_map_create (100, 11, sizeof (int), &err);
  assert (err == 0);

  {
    c_map_insert (&map, STR_W_LEN ("abc"), INT_W_LEN (1), &err);
    assert (err == 0);
    c_map_insert (&map, STR_W_LEN ("ahmed here"), INT_W_LEN (2), &err);
    assert (err == 0);
    c_map_insert (&map, STR_W_LEN ("abcd"), INT_W_LEN (3), &err); // test collision
    assert (err == 0);
    c_map_insert (&map, STR_W_LEN ("abc"), INT_W_LEN (4), &err); // test override
    assert (err == 0);

    assert (*(int*) c_map_get (&map, STR_W_LEN ("abcd")) == 3);
    assert (*(int*) c_map_get (&map, STR_W_LEN ("abc")) == 4);
    assert (c_map_get (&map, STR_W_LEN ("xyz")) == NULL); // test not exist
  }

  // test: foreach
  {
    /// FIXME: this test case will not work as foreach implementation was wrong
    ///        this will loop on first map->len elements,
    ///        instead of the occupied ones
    int sum = 0;
    void c_map_handler (void* key, void* value, void* extra_data);
    c_map_foreach (&map, c_map_handler, &sum);
    assert (sum == 10);
  }

  c_map_destroy (&map);
}

void
c_map_handler (void* key, void* value, void* extra_data)
{
  (void) key;
  *(int*) extra_data += *(int*) value;
}

#ifdef NDEBUG_
#define NDEBUG
#undef NDEBUG_
#endif

#undef CSTDLIB_MAP_UNIT_TESTS
#endif // CSTDLIB_MAP_UNIT_TESTS

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
