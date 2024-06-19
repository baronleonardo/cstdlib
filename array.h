/* How To : To use this module, do this in *ONE* C file:
 *              #define CSTDLIB_ARRAY_IMPLEMENTATION
 *              #include "array.h"
 * Tests  : To use run test, do this in *ONE* C file:
 *              #define CSTDLIB_ARRAY_UNIT_TESTS
 *              #include "array.h"
 * License: MIT (send end of the file for details)
 */

#ifndef CSTDLIB_ARRAY_H
#define CSTDLIB_ARRAY_H
#include <stdbool.h>
#include <stddef.h>

/// @brief you can always cast 'CArray' to anytype you like
///        and use it as oridinary heap allocated array
typedef struct CArray
{
  void* data;
  size_t capacity;     /// maximum data that can be hold, note: this unit based
                       /// not bytes based
  size_t len;          /// current length, note: this unit based not bytes based
  size_t element_size; /// size of the unit
} CArray;

enum
{
  CARRAY_ERROR_none,
  CARRAY_ERROR_wrong_len,
  CARRAY_ERROR_wrong_element,
  CARRAY_ERROR_wrong_index,
  CARRAY_ERROR_wrong_range_len,
  CARRAY_ERROR_remove_on_empty,
  CARRAY_ERROR_memory_issue,
} CArrayError;

CArray c_array_create (size_t element_size,
                       size_t initial_capacity,
                       int* err);
CArray c_array_create_unmanaged (size_t element_size,
                                 size_t initial_capacity,
                                 void* c_array_calloc (size_t, size_t),
                                 int* err);
/// @brief create an array with capacity of 1
/// @param element_size
/// @param err return error (any value but zero is treated as an error)
/// @return
CArray c_array_create_empty (size_t element_size,
                             int* err);

/// @brief same as `c_array_create` but with allocating capacity
/// @param element_size
/// @param capacity maximum number of elements to be allocated, minimum
///                 capacity is 1
/// @param c_array_calloc optional: pass a calloc function to be used for memory
///                               allocation
/// @param err return error (any value but zero is treated as an error)
/// @return
CArray c_array_create_with_capacity (size_t element_size,
                                     size_t capacity,
                                     int* err);
CArray
c_array_create_with_capacity_unmanaged (size_t element_size,
                                        size_t capacity,
                                        void* c_array_calloc (size_t, size_t),
                                        int* err);

/// @brief check wether the array is empty
/// @param self
/// @param err return error (any value but zero is treated as an error)
/// @return
bool c_array_is_empty (CArray* self);

/// @brief get array length
/// @param self
/// @return array length
size_t c_array_len (CArray* self);

/// @brief set array length
///        [beware this is DANGEROUS]
///        this is useful if you want
///        to manipulate the data by yourself
/// @param self
/// @param new_len
/// @param err return error (any value but zero is treated as an error)
void c_array_set_len (CArray* self,
                      size_t new_len,
                      int* err);

/// @brief get array capacity
///        this will return the capacity in 'element_size' wise
///        example: 'capacity = 10' means
///                 we can have up to '10 * element_size' bytes
/// @param self
/// @return the capacity of the array
size_t c_array_capacity (CArray* self);

/// @brief set capacity
///        [beware this is DANGEROUS]
///        this is useful if you want
///        to manipulate the data by yourself
/// @param self address of self
/// @param new_capacity
/// @param err return error (any value but zero is treated as an error)
void c_array_set_capacity (CArray* self,
                           size_t new_capacity,
                           int* err);
void c_array_set_capacity_unmanaged (CArray* self,
                                     size_t new_capacity,
                                     void* c_array_realloc (void*, size_t),
                                     int* err);

/// @brief get elemet_size in bytes
/// @param self
/// @return elemet_size in bytes
size_t c_array_element_size (CArray* self);

/// @brief push one element at the end
/// @param self pointer to self
/// @param element if you want to push literals (example: 3, 5 or 10 ...)
///                c_array_push(array, &(int){3});
/// @param err return error (any value but zero is treated as an error)
void c_array_push (CArray* self,
                   void const* element,
                   int* err);
void c_array_push_unmanaged (CArray* self,
                             void const* element,
                             void* c_array_realloc (void*, size_t),
                             int* err);

/// @brief pop one element from the end
///        [this will NOT resize the array]
/// @param self
/// @param err return error (any value but zero is treated as an error)
/// @return pointer of the popped element
void* c_array_pop (CArray* self,
                   int* err);

/// @brief insert 1 element at index
/// @param self pointer to self
/// @param element
/// @param index
/// @param c_array_realloc optional: pass a realloc function to be used for
///                                memory re-allocation
/// @param err return error (any value but zero is treated as an error)
void c_array_insert (CArray* self,
                     void const* element,
                     size_t index,
                     int* err);
void c_array_insert_unmanaged (CArray* self,
                               void const* element,
                               size_t index,
                               void* c_array_realloc (void*, size_t),
                               int* err);

/// @brief insert multiple elements at index
/// @param self
/// @param index
/// @param data
/// @param data_len
/// @param err return error (any value but zero is treated as an error)
void c_array_insert_range (CArray* self,
                           size_t index,
                           void const* data,
                           size_t data_len,
                           int* err);
void c_array_insert_range_unmanaged (CArray* self,
                                     size_t index,
                                     void const* data,
                                     size_t data_len,
                                     void* c_array_realloc (void*, size_t),
                                     int* err);

/// @brief remove element from CArray
///        [beware this function is costy]
/// @param self
/// @param index index to be removed
/// @param err return error (any value but zero is treated as an error)
void c_array_remove (CArray* self,
                     size_t index,
                     int* err);

/// @brief remove a range of elements from CArray
/// @param self
/// @param start_index
/// @param range_len range length
/// @param err return error (any value but zero is treated as an error)
void c_array_remove_range (CArray* self,
                           size_t start_index,
                           size_t range_len,
                           int* err);

/// @brief destroy the array from the memory
/// @param self
/// @param c_array_free optional: pass a free function to be used for memory
///                               freeing
/// @param err return error (any value but zero is treated as an error)
void c_array_destroy (CArray* self);
void c_array_destroy_unmanaged (CArray* self,
                                void c_array_free (void*));
#endif // CSTDLIB_ARRAY_H

#ifdef CSTDLIB_ARRAY_IMPLEMENTATION
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

CArray
c_array_create (size_t element_size,
                size_t initial_capacity,
                int* err)
{
  return c_array_create_with_capacity_unmanaged (element_size, initial_capacity, calloc, err);
}

CArray
c_array_create_unmanaged (size_t element_size,
                          size_t initial_capacity,
                          void* c_array_calloc (size_t, size_t),
                          int* err)
{
  return c_array_create_with_capacity_unmanaged (element_size,
                                                 initial_capacity,
                                                 c_array_calloc,
                                                 err);
}

CArray
c_array_create_empty (size_t element_size,
                      int* err)
{
  return c_array_create_with_capacity_unmanaged (element_size,
                                                 1U,
                                                 calloc,
                                                 err);
}

CArray
c_array_create_with_capacity (size_t element_size,
                              size_t capacity,
                              int* err)
{
  return c_array_create_with_capacity_unmanaged (element_size,
                                                 capacity,
                                                 calloc,
                                                 err);
}

CArray
c_array_create_with_capacity_unmanaged (size_t element_size,
                                        size_t capacity,
                                        void* c_array_calloc (size_t, size_t),
                                        int* err)
{
  assert (element_size > 0);
  assert (capacity > 0);
  assert (c_array_calloc);

  int error_;
  err = err ? err : &error_;
  *err = 0;

  CArray arr = {
    .data = c_array_calloc (capacity, element_size),
    .capacity = capacity,
    .element_size = element_size,
  };

  if (!arr.data)
    {
      *err = CARRAY_ERROR_memory_issue;
    }

  return arr;
}

bool
c_array_is_empty (CArray* self)
{
  return c_array_len (self) == 0;
}

size_t
c_array_len (CArray* self)
{
  assert (self && self->data);

  return self->len;
}

void
c_array_set_len (CArray* self,
                 size_t new_len,
                 int* err)
{
  assert (self && self->data);
  assert (new_len > 0);

  int error_;
  err = err ? err : &error_;
  *err = 0;

  if (new_len < self->capacity)
    {
      self->len = new_len;
    }
  else
    {
      *err = CARRAY_ERROR_wrong_len;
    }
}

size_t
c_array_capacity (CArray* self)
{
  assert (self && self->data);

  return self->capacity;
}

void
c_array_set_capacity (CArray* self,
                      size_t new_capacity,
                      int* err)
{
  c_array_set_capacity_unmanaged (self, new_capacity, realloc, err);
}

void
c_array_set_capacity_unmanaged (CArray* self,
                                size_t new_capacity,
                                void* c_array_realloc (void*, size_t),
                                int* err)
{
  assert (self && self->data);
  assert (new_capacity > 0);
  assert (c_array_realloc);

  int error_;
  err = err ? err : &error_;
  *err = 0;

  self->data = realloc (self->data, new_capacity * self->element_size);
  if (!self->data)
    {
      *err = CARRAY_ERROR_memory_issue;
      return;
    }

  self->capacity = new_capacity;
}

size_t
c_array_element_size (CArray* self)
{
  assert (self && self->data);

  return self->element_size;
}

void
c_array_push (CArray* self,
              void const* element,
              int* err)
{
  c_array_push_unmanaged (self, element, realloc, err);
}

void
c_array_push_unmanaged (CArray* self,
                        void const* element,
                        void* c_array_realloc (void*, size_t),
                        int* err)
{
  assert (self && self->data);
  assert (element);
  assert (c_array_realloc);

  int error_;
  err = err ? err : &error_;
  *err = 0;

  if ((self->len + 1U) > self->capacity)
    {
      self->capacity *= 2;
      self->data = c_array_realloc (self->data, self->capacity * self->element_size);
      if (!self->data)
        {
          *err = CARRAY_ERROR_memory_issue;
          return;
        }
    }

  memcpy ((uint8_t*) self->data + (self->len * self->element_size),
          element,
          self->element_size);
  self->len++;
}

void*
c_array_pop (CArray* self,
             int* err)
{
  assert (self && self->data);

  if (self->len > 0)
    {
      uint8_t* element = (uint8_t*) self->data + ((self->len - 1U) * self->element_size);
      self->len--;

      return element;
    }
  else if (err)
    {
      *err = CARRAY_ERROR_wrong_len;
    }

  return NULL;
}

void
c_array_insert (CArray* self,
                void const* element,
                size_t index,
                int* err)
{
  c_array_insert_unmanaged (self, element, index, realloc, err);
}

void
c_array_insert_unmanaged (CArray* self,
                          void const* element,
                          size_t index,
                          void* c_array_realloc (void*, size_t),
                          int* err)
{
  assert (self && self->data);

  int error_;
  err = err ? err : &error_;
  *err = 0;

  if (self->len > index)
    {
      if ((self->len + 1U) > self->capacity)
        {
          c_array_set_capacity_unmanaged (self, self->capacity * 2, c_array_realloc, err);

          if (*err != 0)
            {
              return;
            }
        }

      if (index < self->len)
        {
          memmove ((uint8_t*) self->data + ((index + 1) * self->element_size),
                   (uint8_t*) self->data + (index * self->element_size),
                   (self->len - index) * self->element_size);
        }

      memcpy ((uint8_t*) self->data + (index * self->element_size),
              element,
              self->element_size);
      self->len++;
    }
  else
    {
      *err = CARRAY_ERROR_wrong_index;
    }
}

void
c_array_insert_range (CArray* self,
                      size_t index,
                      void const* data,
                      size_t data_len,
                      int* err)
{
  c_array_insert_range_unmanaged (self, index, data, data_len, realloc, err);
}

void
c_array_insert_range_unmanaged (CArray* self,
                                size_t index,
                                void const* data,
                                size_t data_len,
                                void* c_array_realloc (void*, size_t),
                                int* err)
{
  assert (self && self->data);
  assert (data);
  assert (data_len > 0);
  assert (c_array_realloc);

  int error_;
  err = err ? err : &error_;
  *err = 0;

  if (self->len > index)
    {
      while ((self->len + data_len) > self->capacity)
        {
          c_array_set_capacity_unmanaged (self, self->capacity * 2, c_array_realloc, err);
          if (*err != 0)
            {
              return;
            }
        }

      if (index < self->len)
        {
          memmove ((uint8_t*) self->data + ((index + data_len) * self->element_size),
                   (uint8_t*) self->data + (index * self->element_size),
                   (self->len - index) * self->element_size);
        }

      memcpy ((uint8_t*) self->data + (index * self->element_size),
              data, data_len * self->element_size);

      self->len += data_len;
    }
  else
    {
      *err = CARRAY_ERROR_wrong_index;
    }
}

void
c_array_remove (CArray* self,
                size_t index,
                int* err)
{
  assert (self && self->data);

  int error_;
  err = err ? err : &error_;
  *err = 0;

  if (self->len > 0U)
    {
      if (self->len > index)
        {
          uint8_t* element = (uint8_t*) self->data + (index * self->element_size);

          memmove (element,
                   element + self->element_size,
                   (self->len - index) * self->element_size);

          self->len--;
        }
      else
        {
          *err = CARRAY_ERROR_wrong_index;
        }
    }
  else
    {
      *err = CARRAY_ERROR_remove_on_empty;
    }
}

void
c_array_remove_range (CArray* self,
                      size_t start_index,
                      size_t range_len,
                      int* err)
{
  assert (self && self->data);

  int error_;
  err = err ? err : &error_;
  *err = 0;

  if (self->len == 0U)
    {
      *err = CARRAY_ERROR_wrong_len;
    }

  if (start_index > (self->len - 1U))
    {
      *err = CARRAY_ERROR_wrong_index;
    }

  if ((start_index + range_len) > self->len)
    {
      *err = CARRAY_ERROR_wrong_range_len;
    }

  uint8_t* start_ptr = (uint8_t*) self->data + (start_index * self->element_size);

  if (self->len > 0U)
    {
      if (start_index < (self->len - 1U))
        {
          if ((start_index + range_len) <= self->len)
            {
              uint8_t const* end_ptr = (uint8_t*) self->data + ((start_index + range_len) * self->element_size);
              size_t right_range_size = (self->len - (start_index + range_len)) * self->element_size;

              memmove (start_ptr, end_ptr, right_range_size);

              self->len -= range_len;
            }
          else
            {
              *err = CARRAY_ERROR_wrong_range_len;
            }
        }
      else
        {
          *err = CARRAY_ERROR_wrong_index;
        }
    }
  else
    {
      *err = CARRAY_ERROR_remove_on_empty;
    }
}

void
c_array_destroy (CArray* self)
{
  c_array_destroy_unmanaged (self, free);
}

void
c_array_destroy_unmanaged (CArray* self,
                           void c_array_free (void*))
{
  assert (self && self->data);
  assert (c_array_free);

  c_array_free (self->data);

  *self = (CArray){ 0 };
}

#undef CSTDLIB_ARRAY_IMPLEMENTATION
#endif // CSTDLIB_ARRAY_IMPLEMENTATION

#ifdef CSTDLIB_ARRAY_UNIT_TESTS
#ifdef NDEBUG
#define NDEBUG_
#undef NDEBUG
#endif

#include <assert.h>

void
c_array_unit_tests (void)
{
  int err = 0;

  // test: general
  {
    CArray array = c_array_create_empty (sizeof (int), &err);

    // test_push
    c_array_push (&array, &(int){ 12 }, &err);
    assert (err == 0);
    c_array_push (&array, &(int){ 13 }, &err);
    assert (err == 0);
    c_array_push (&array, &(int){ 14 }, &err);
    assert (err == 0);
    c_array_push (&array, &(int){ 15 }, &err);
    assert (err == 0);
    c_array_push (&array, &(int){ 16 }, &err);
    assert (err == 0);

    assert (c_array_len (&array) == 5);

    // test c_array_pop
    int const* data = c_array_pop (&array, &err);
    assert (err == 0);
    assert (*data == 16);

    // test c_array_remove_range
    c_array_remove_range (&array, 1, 3, &err);
    assert (err == 0);
    assert (c_array_len (&array) == 1);
    assert (((int*) array.data)[0] == 12);

    // test c_array_insert
    c_array_insert (&array, &(int){ 20 }, 0, &err);
    assert (err == 0);
    assert (((int*) array.data)[0] == 20);
    assert (((int*) array.data)[1] == 12);

    // test c_array_insert
    c_array_insert_range (&array, 1, &(int[]){ 1, 2, 3 }, 3, &err);
    assert (err == 0);
    assert (((int*) array.data)[0] == 20);
    assert (((int*) array.data)[1] == 1);
    assert (((int*) array.data)[2] == 2);
    assert (((int*) array.data)[3] == 3);
    assert (((int*) array.data)[4] == 12);

    c_array_destroy (&array);
  }

  // test: c_array_insert at last
  {
    CArray array2 = c_array_create_empty (sizeof (char), &err);
    assert (err == 0);
    c_array_push (&array2, &(char){ '\0' }, &err);
    assert (err == 0);
    c_array_insert (&array2, &(char){ 'a' }, 0, &err);
    assert (err == 0);
    assert (((char*) array2.data)[0] == 'a');
    assert (((char*) array2.data)[1] == '\0');

    c_array_destroy (&array2);
  }

  // test: errors
  {
    CArray array = c_array_create_empty (sizeof (char), &err);
    assert (err == 0);
    c_array_push (&array, &(char){ '\0' }, &err);
    assert (err == 0);
    c_array_insert (&array, &(char){ 'a' }, 1, &err);
    assert (err == CARRAY_ERROR_wrong_index);

    c_array_destroy (&array);
  }
}

#ifdef NDEBUG_
#define NDEBUG
#undef NDEBUG_
#endif

#undef CSTDLIB_ARRAY_UNIT_TESTS
#endif // CSTDLIB_ARRAY_UNIT_TESTS

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
