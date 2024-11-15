/* How To   : To use this module, do this in *ONE* C file:
 *              #define CSTDLIB_ARRAY_IMPLEMENTATION
 *              #include "array.h"
 * Tests    : To use run test, do this in *ONE* C file:
 *              #define CSTDLIB_ARRAY_UNIT_TESTS
 *              #include "array.h"
 * Options :
 *           - C_ARR_DONT_CHECK_PARAMS: parameters will not get checked
 *                                      (this is off by default)
 * License: MIT (go to the end of the file for details)
 */

/* ------------------------------------------------------------------------ */
/* -------------------------------- header -------------------------------- */
/* ------------------------------------------------------------------------ */

#ifndef CSTDLIB_ARRAY_H
#define CSTDLIB_ARRAY_H
#include <stdbool.h>
#include <stddef.h>

/// @brief you can always cast 'CArray' data member variable
/// to anytype you like and use it as oridinary heap allocated array
typedef struct CArray {
  void*  data;
  size_t capacity;     /// maximum data that can be hold, note: this unit based
                       /// not bytes based
  size_t len;          /// current length, note: this unit based not bytes based
  size_t element_size; /// size of the unit
} CArray;

typedef struct CArrayUnmanaged {
  void*  data;
  size_t capacity;     /// maximum data that can be hold, note: this unit based
                       /// not bytes based
  size_t len;          /// current length, note: this unit based not bytes based
  size_t element_size; /// size of the unit
} CArrayUnmanaged;

typedef struct c_array_error_t {
  int         code;
  char const* desc;
} c_array_error_t;

#define C_ARRAY_ERROR_none ((c_array_error_t){.code = 0, .desc = ""})
#define C_ARRAY_ERROR_mem_allocation                                           \
  ((c_array_error_t){.code = 1, .desc = "array: memory allocation error"})
#define C_ARRAY_ERROR_wrong_len                                                \
  ((c_array_error_t){.code = 2, .desc = "array: wrong length"})
#define C_ARRAY_ERROR_wrong_capacity                                           \
  ((c_array_error_t){.code = 3, .desc = "array: wrong capactiy"})
#define C_ARRAY_ERROR_wrong_index                                              \
  ((c_array_error_t){.code = 4, .desc = "array: wrong index"})
#define C_ARRAY_ERROR_capacity_full                                            \
  ((c_array_error_t){.code = 5, .desc = "array: capacity is full"})
#define C_ARRAY_ERROR_needle_not_found                                         \
  ((c_array_error_t){.code = 6, .desc = "array: needle not found"})
#define C_ARRAY_ERROR_empty                                                    \
  ((c_array_error_t){.code = 7, .desc = "array: is empty"})
#define C_ARRAY_ERROR_wrong_range                                              \
  ((c_array_error_t){.code = 8, .desc = "array: wrong range"})
#define C_ARRAY_ERROR_invalid_parameters                                       \
  ((c_array_error_t){.code = 9, .desc = "array: invalid parameters"})

/// @brief create a new array
///        (NOTE: calloc_fn is a calloc like function)
/// @param element_size
/// @param out_c_array the result CArray object created
/// @return return error (any value but zero is treated as an error)
c_array_error_t c_array_create(size_t element_size, CArray* out_c_array);
c_array_error_t c_array_create_unmanaged(size_t element_size,
                                         void*  calloc_fn(size_t, size_t),
                                         CArrayUnmanaged* out_c_array);

/// @brief same as `c_array_create` but with allocating capacity
///        (NOTE: calloc_fn is a calloc like function)
/// @param element_size
/// @param capacity maximum number of elements to be allocated, minimum
///                 capacity is 1
/// @param out_c_array the result CArray object created
/// @param return error (any value but zero is treated as an error)
/// @return
c_array_error_t c_array_create_with_capacity(size_t  element_size,
                                             size_t  capacity,
                                             CArray* out_c_array);
c_array_error_t
c_array_create_with_capacity_unmanaged(size_t element_size,
                                       size_t capacity,
                                       void*  calloc_fn(size_t, size_t),
                                       CArrayUnmanaged* out_c_array);

/// @brief check wether the array is empty
/// @param self
/// @param out_is_empty the returned result
/// @return return error (any value but zero is treated as an error)
c_array_error_t c_array_is_empty(CArray* self, bool* out_is_empty);
c_array_error_t c_array_is_empty_unmanaged(CArrayUnmanaged* self,
                                           bool*            out_is_empty);

/// @brief get array length
/// @param self
/// @param out_len the returned result
/// @return return error (any value but zero is treated as an error)
c_array_error_t c_array_len(CArray* self, size_t* out_len);
c_array_error_t c_array_len_unmanaged(CArrayUnmanaged* self, size_t* out_len);

/// @brief set array length
///        [beware this is DANGEROUS]
///        this is useful if you want
///        to manipulate the data by yourself
/// @param self
/// @param new_len
/// @return return error (any value but zero is treated as an error)
c_array_error_t c_array_set_len(CArray* self, size_t new_len);
c_array_error_t c_array_set_len_unmanaged(CArrayUnmanaged* self,
                                          size_t           new_len,
                                          void* realloc_fn(void*, size_t));

/// @brief get array capacity
///        this will return the capacity in 'element_size' wise
///        example: 'capacity = 10' means
///                 we can have up to '10 * element_size' bytes
/// @param self
/// @param out_capacity the returned result
/// @return return error (any value but zero is treated as an error)
c_array_error_t c_array_capacity(CArray* self, size_t* out_capacity);
c_array_error_t c_array_capacity_unmanaged(CArrayUnmanaged* self,
                                           size_t*          out_capacity);

/// @brief set capacity
///        (NOTE: realloc_fn is a realloc like function)
/// @param self address of self
/// @param new_capacity
/// @return return error (any value but zero is treated as an error)
c_array_error_t c_array_set_capacity(CArray* self, size_t new_capacity);
c_array_error_t c_array_set_capacity_unmanaged(CArrayUnmanaged* self,
                                               size_t           new_capacity,
                                               void* realloc_fn(void*, size_t));

/// @brief get elemet_size in bytes
/// @param self
/// @param out_element_size the returned result
/// @return return error (any value but zero is treated as an error)
c_array_error_t c_array_element_size(CArray* self, size_t* out_element_size);
c_array_error_t c_array_element_size_unmanaged(CArrayUnmanaged* self,
                                               size_t* out_element_size);

/// @brief push one element at the end
///        (NOTE: realloc_fn is a realloc like function)
/// @param self pointer to self
/// @param element if you want to push literals (example: 3, 5 or 10 ...)
///                c_array_push(array, &(int){3});
/// @return return error (any value but zero is treated as an error)
c_array_error_t c_array_push(CArray* self, void const* element);
c_array_error_t c_array_push_unmanaged(CArrayUnmanaged* self,
                                       void const*      element,
                                       void* realloc_fn(void*, size_t));

/// @brief pop one element from the end
///        [this will NOT resize the array]
///        (NOTE: realloc_fn is a realloc like function)
/// @param self
/// @param out_element the returned result
/// @return return error (any value but zero is treated as an error)
c_array_error_t c_array_pop(CArray* self, void** out_element);
c_array_error_t c_array_pop_unmanaged(CArrayUnmanaged* self,
                                      void**           out_element,
                                      void* realloc_fn(void*, size_t));

/// @brief insert 1 element at index
///        (NOTE: realloc_fn is a realloc like function)
/// @param self pointer to self
/// @param element
/// @param index
/// @return return error (any value but zero is treated as an error)
c_array_error_t c_array_insert(CArray* self, void const* element, size_t index);
c_array_error_t c_array_insert_unmanaged(CArrayUnmanaged* self,
                                         void const*      element,
                                         size_t           index,
                                         void* realloc_fn(void*, size_t));

/// @brief insert multiple elements at index
///        (NOTE: realloc_fn is a realloc like function)
/// @param self
/// @param index
/// @param data
/// @param data_len
/// @return return error (any value but zero is treated as an error)
c_array_error_t c_array_insert_range(CArray*     self,
                                     size_t      index,
                                     void const* data,
                                     size_t      data_len);
c_array_error_t c_array_insert_range_unmanaged(CArrayUnmanaged* self,
                                               size_t           index,
                                               void const*      data,
                                               size_t           data_len,
                                               void* realloc_fn(void*, size_t));

/// @brief remove element from CArray
///        [beware this function is costy]
/// @param self
/// @param index index to be removed
/// @return return error (any value but zero is treated as an error)
c_array_error_t c_array_remove(CArray* self, size_t index);
c_array_error_t c_array_remove_unmanaged(CArrayUnmanaged* self, size_t index);

/// @brief remove a range of elements from CArray
/// @param self
/// @param start_index
/// @param range_len range length
/// @return return error (any value but zero is treated as an error)
c_array_error_t
c_array_remove_range(CArray* self, size_t start_index, size_t range_len);
c_array_error_t c_array_remove_range_unmanaged(CArrayUnmanaged* self,
                                               size_t           start_index,
                                               size_t           range_len,
                                               void* realloc_fn(void*, size_t));

/// @brief destroy the array from the memory
/// @param self
/// @param c_array_free optional: pass a free function to be used for memory
///                               freeing
void c_array_destroy(CArray* self);
void c_array_destroy_unmanaged(CArrayUnmanaged* self, void c_array_free(void*));
#endif // CSTDLIB_ARRAY_H

/* ------------------------------------------------------------------------ */
/* ---------------------------- implementation ---------------------------- */
/* ------------------------------------------------------------------------ */

#ifdef CSTDLIB_ARRAY_IMPLEMENTATION
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if _WIN32 && (!_MSC_VER || !(_MSC_VER >= 1900))
#error "You need MSVC must be higher that or equal to 1900"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996) // disable warning about unsafe functions
#endif

#ifndef C_ARR_DONT_CHECK_PARAMS
#define C_ARR_CHECK_PARAMS(params)                                             \
  if (!(params)) return C_ARRAY_ERROR_invalid_parameters;
#else
#define C_ARR_CHECK_PARAMS(params) ((void)0)
#endif

c_array_error_t
c_array_create(size_t element_size, CArray* out_c_array)
{
  return c_array_create_unmanaged(element_size, calloc,
                                  (CArrayUnmanaged*)out_c_array);
}

c_array_error_t
c_array_create_unmanaged(size_t           element_size,
                         void*            calloc_fn(size_t, size_t),
                         CArrayUnmanaged* out_c_array)
{
  return c_array_create_with_capacity_unmanaged(element_size, 1U, calloc_fn,
                                                out_c_array);
}

c_array_error_t
c_array_create_with_capacity(size_t  element_size,
                             size_t  capacity,
                             CArray* out_c_array)
{
  return c_array_create_with_capacity_unmanaged(element_size, capacity, calloc,
                                                (CArrayUnmanaged*)out_c_array);
}

c_array_error_t
c_array_create_with_capacity_unmanaged(size_t element_size,
                                       size_t capacity,
                                       void*  calloc_fn(size_t, size_t),
                                       CArrayUnmanaged* out_c_array)
{
  C_ARR_CHECK_PARAMS(element_size > 0 || capacity > 0 || calloc_fn);

  if (out_c_array) {
    *out_c_array = (CArrayUnmanaged){
        .data         = calloc_fn ? calloc_fn(capacity, element_size) : NULL,
        .capacity     = capacity,
        .element_size = element_size,
    };

    if (!out_c_array->data) { return C_ARRAY_ERROR_mem_allocation; }
  }

  return C_ARRAY_ERROR_none;
}

c_array_error_t
c_array_is_empty(CArray* self, bool* out_is_empty)
{
  return c_array_is_empty_unmanaged((CArrayUnmanaged*)self, out_is_empty);
}

c_array_error_t
c_array_is_empty_unmanaged(CArrayUnmanaged* self, bool* out_is_empty)
{
  C_ARR_CHECK_PARAMS(self);

  if (out_is_empty) { *out_is_empty = self->len; }

  return C_ARRAY_ERROR_none;
}

c_array_error_t
c_array_len(CArray* self, size_t* out_len)
{
  return c_array_len_unmanaged((CArrayUnmanaged*)self, out_len);
}

c_array_error_t
c_array_len_unmanaged(CArrayUnmanaged* self, size_t* out_len)
{
  C_ARR_CHECK_PARAMS(self && self->data);

  if (out_len) { *out_len = self->len; }

  return C_ARRAY_ERROR_none;
}

c_array_error_t
c_array_set_len(CArray* self, size_t new_len)
{
  return c_array_set_len_unmanaged((CArrayUnmanaged*)self, new_len, realloc);
}

c_array_error_t
c_array_set_len_unmanaged(CArrayUnmanaged* self,
                          size_t           new_len,
                          void*            realloc_fn(void*, size_t))
{
  C_ARR_CHECK_PARAMS(self && self->data);
  C_ARR_CHECK_PARAMS(new_len > 0);

  if (new_len > self->capacity) {
    c_array_error_t err
        = c_array_set_capacity_unmanaged(self, new_len, realloc_fn);
    if (err.code != C_ARRAY_ERROR_none.code) { return err; }
  }

  self->len = new_len;
  return C_ARRAY_ERROR_none;
}

c_array_error_t
c_array_capacity(CArray* self, size_t* out_capacity)
{
  return c_array_capacity_unmanaged((CArrayUnmanaged*)self, out_capacity);
}

c_array_error_t
c_array_capacity_unmanaged(CArrayUnmanaged* self, size_t* out_capacity)
{
  C_ARR_CHECK_PARAMS(self && self->data);

  if (out_capacity) { *out_capacity = self->capacity; }

  return C_ARRAY_ERROR_none;
}

c_array_error_t
c_array_set_capacity(CArray* self, size_t new_capacity)
{
  return c_array_set_capacity_unmanaged((CArrayUnmanaged*)self, new_capacity,
                                        realloc);
}

c_array_error_t
c_array_set_capacity_unmanaged(CArrayUnmanaged* self,
                               size_t           new_capacity,
                               void*            realloc_fn(void*, size_t))
{
  C_ARR_CHECK_PARAMS(self && self->data);
  C_ARR_CHECK_PARAMS(new_capacity > 0);

  void* reallocated_data
      = realloc_fn ? realloc_fn(self->data, new_capacity * self->element_size)
                   : NULL;
  if (!reallocated_data) { return C_ARRAY_ERROR_mem_allocation; }
  self->data     = reallocated_data;
  self->capacity = new_capacity;

  return C_ARRAY_ERROR_none;
}

c_array_error_t
c_array_element_size(CArray* self, size_t* out_element_size)
{
  return c_array_element_size_unmanaged((CArrayUnmanaged*)self,
                                        out_element_size);
}

c_array_error_t
c_array_element_size_unmanaged(CArrayUnmanaged* self, size_t* out_element_size)
{
  C_ARR_CHECK_PARAMS(self && self->data);

  if (out_element_size) { *out_element_size = self->element_size; }

  return C_ARRAY_ERROR_none;
}

c_array_error_t
c_array_push(CArray* self, void const* element)
{
  return c_array_push_unmanaged((CArrayUnmanaged*)self, element, realloc);
}

c_array_error_t
c_array_push_unmanaged(CArrayUnmanaged* self,
                       void const*      element,
                       void*            realloc_fn(void*, size_t))
{
  C_ARR_CHECK_PARAMS(self && self->data);
  C_ARR_CHECK_PARAMS(element);

  if (self->len == self->capacity) {
    self->capacity *= 2;
    void* reallocated_data
        = realloc_fn
              ? realloc_fn(self->data, self->capacity * self->element_size)
              : NULL;
    if (!self->data) { return C_ARRAY_ERROR_mem_allocation; }
    self->data = reallocated_data;
  }

  memcpy((uint8_t*)self->data + (self->len * self->element_size), element,
         self->element_size);
  self->len++;

  return C_ARRAY_ERROR_none;
}

c_array_error_t
c_array_pop(CArray* self, void** out_element)
{
  return c_array_pop_unmanaged((CArrayUnmanaged*)self, out_element, realloc);
}

c_array_error_t
c_array_pop_unmanaged(CArrayUnmanaged* self,
                      void**           out_element,
                      void*            realloc_fn(void*, size_t))
{
  C_ARR_CHECK_PARAMS(self && self->data);

  if (self->len > 0) {
    c_array_error_t err = C_ARRAY_ERROR_none;

    uint8_t* element
        = (uint8_t*)self->data + ((self->len - 1U) * self->element_size);
    self->len--;

    if (self->len <= self->capacity / 4) {
      err = c_array_set_capacity_unmanaged(self, self->capacity / 2,
                                           realloc_fn);
    }

    if (out_element) *out_element = element;

    return err;
  } else {
    return C_ARRAY_ERROR_wrong_len;
  }
}

c_array_error_t
c_array_insert(CArray* self, void const* element, size_t index)
{
  return c_array_insert_unmanaged((CArrayUnmanaged*)self, element, index,
                                  realloc);
}

c_array_error_t
c_array_insert_unmanaged(CArrayUnmanaged* self,
                         void const*      element,
                         size_t           index,
                         void*            realloc_fn(void*, size_t))
{
  C_ARR_CHECK_PARAMS(self && self->data);

  if (self->len > index) {
    if (self->len == self->capacity) {
      c_array_error_t err = c_array_set_capacity_unmanaged(
          self, self->capacity * 2, realloc_fn);

      if (err.code != C_ARRAY_ERROR_none.code) { return err; }
    }

    if (index < self->len) {
      memmove((uint8_t*)self->data + ((index + 1) * self->element_size),
              (uint8_t*)self->data + (index * self->element_size),
              (self->len - index) * self->element_size);
    }

    memcpy((uint8_t*)self->data + (index * self->element_size), element,
           self->element_size);
    self->len++;

    return C_ARRAY_ERROR_none;
  } else {
    return C_ARRAY_ERROR_wrong_index;
  }
}

c_array_error_t
c_array_insert_range(CArray*     self,
                     size_t      index,
                     void const* data,
                     size_t      data_len)
{
  return c_array_insert_range_unmanaged((CArrayUnmanaged*)self, index, data,
                                        data_len, realloc);
}

c_array_error_t
c_array_insert_range_unmanaged(CArrayUnmanaged* self,
                               size_t           index,
                               void const*      data,
                               size_t           data_len,
                               void*            realloc_fn(void*, size_t))
{
  C_ARR_CHECK_PARAMS(self && self->data);
  C_ARR_CHECK_PARAMS(data);
  C_ARR_CHECK_PARAMS(data_len > 0);

  if (self->len > index) {
    while ((self->len + data_len) > self->capacity) {
      c_array_error_t err = c_array_set_capacity_unmanaged(
          self, self->capacity * 2, realloc_fn);
      if (err.code != C_ARRAY_ERROR_none.code) { return err; }
    }

    if (index < self->len) {
      memmove((uint8_t*)self->data + ((index + data_len) * self->element_size),
              (uint8_t*)self->data + (index * self->element_size),
              (self->len - index) * self->element_size);
    }

    memcpy((uint8_t*)self->data + (index * self->element_size), data,
           data_len * self->element_size);

    self->len += data_len;

    return C_ARRAY_ERROR_none;
  } else {
    return C_ARRAY_ERROR_wrong_index;
  }
}

c_array_error_t
c_array_remove(CArray* self, size_t index)
{
  return c_array_remove_unmanaged((CArrayUnmanaged*)self, index);
}

c_array_error_t
c_array_remove_unmanaged(CArrayUnmanaged* self, size_t index)
{
  C_ARR_CHECK_PARAMS(self && self->data);

  if (index > self->len) { return C_ARRAY_ERROR_wrong_index; }

  uint8_t* element = (uint8_t*)self->data + (index * self->element_size);

  memmove(element, element + self->element_size,
          (self->len - index - 1) * self->element_size);

  self->len--;

  return C_ARRAY_ERROR_none;
}

c_array_error_t
c_array_remove_range(CArray* self, size_t start_index, size_t range_len)
{
  return c_array_remove_range_unmanaged((CArrayUnmanaged*)self, start_index,
                                        range_len, realloc);
}

c_array_error_t
c_array_remove_range_unmanaged(CArrayUnmanaged* self,
                               size_t           start_index,
                               size_t           range_len,
                               void*            realloc_fn(void*, size_t))
{
  C_ARR_CHECK_PARAMS(self && self->data);

  c_array_error_t err = C_ARRAY_ERROR_none;

  if (self->len == 0U) { return C_ARRAY_ERROR_wrong_len; }

  if (start_index > (self->len - 1U)) { return C_ARRAY_ERROR_wrong_index; }

  if ((start_index + range_len) > self->len) { return C_ARRAY_ERROR_wrong_len; }

  uint8_t* start_ptr
      = (uint8_t*)self->data + (start_index * self->element_size);
  uint8_t const* end_ptr
      = (uint8_t*)self->data + ((start_index + range_len) * self->element_size);
  size_t right_range_size
      = (self->len - (start_index + range_len)) * self->element_size;

  memmove(start_ptr, end_ptr, right_range_size);

  self->len -= range_len;

  if (self->len <= self->capacity / 4) {
    err = c_array_set_capacity_unmanaged(self, self->capacity / 2, realloc_fn);
  }

  return err;
}

void
c_array_destroy(CArray* self)
{
  c_array_destroy_unmanaged((CArrayUnmanaged*)self, free);
}

void
c_array_destroy_unmanaged(CArrayUnmanaged* self, void c_array_free(void*))
{
  if (self && self->data) {
    c_array_free(self->data);
    *self = (CArrayUnmanaged){0};
  }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#undef C_ARR_CHECK_PARAMS
#undef CSTDLIB_ARRAY_IMPLEMENTATION
#endif // CSTDLIB_ARRAY_IMPLEMENTATION

/* ------------------------------------------------------------------------ */
/* -------------------------------- tests --------------------------------- */
/* ------------------------------------------------------------------------ */

#ifdef CSTDLIB_ARRAY_UNIT_TESTS
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

#define ARRAY_STR(str) str, (sizeof(str) - 1)
#define ARRAY_TEST_PRINT_ABORT(msg) (fprintf(stderr, "%s\n", msg), abort())
#define ARRAY_TEST(err)                                                        \
  (err.code != C_ARRAY_ERROR_none.code) ? ARRAY_TEST_PRINT_ABORT(err.desc)     \
                                        : (void)0
#define ARRAY_ASSERT(cond) (!(cond)) ? ARRAY_TEST_PRINT_ABORT(#cond) : (void)0

int
main(void)
{
  c_array_error_t err = C_ARRAY_ERROR_none;

  // test: general
  {
    CArray array;
    err = c_array_create(sizeof(int), &array);
    ARRAY_TEST(err);

    // test_push
    err = c_array_push(&array, &(int){12});
    ARRAY_TEST(err);
    err = c_array_push(&array, &(int){13});
    ARRAY_TEST(err);
    err = c_array_push(&array, &(int){14});
    ARRAY_TEST(err);
    err = c_array_push(&array, &(int){15});
    ARRAY_TEST(err);
    err = c_array_push(&array, &(int){16});
    ARRAY_TEST(err);

    size_t array_len;
    err = c_array_len(&array, &array_len);
    ARRAY_TEST(err);
    ARRAY_ASSERT(array_len == 5);

    // test c_array_pop
    int const* data;
    err = c_array_pop(&array, (void**)&data);
    ARRAY_TEST(err);
    ARRAY_ASSERT(*data == 16);

    // test c_array_remove_range
    err = c_array_remove_range(&array, 1, 3);
    ARRAY_TEST(err);
    err = c_array_len(&array, &array_len);
    ARRAY_TEST(err);
    ARRAY_ASSERT(array_len == 1);
    ARRAY_ASSERT(((int*)array.data)[0] == 12);

    // test c_array_insert
    err = c_array_insert(&array, &(int){20}, 0);
    ARRAY_TEST(err);
    ARRAY_ASSERT(((int*)array.data)[0] == 20);
    ARRAY_ASSERT(((int*)array.data)[1] == 12);

    // test c_array_insert
    err = c_array_insert_range(&array, 1, &(int[]){1, 2, 3}, 3);
    ARRAY_TEST(err);
    ARRAY_ASSERT(((int*)array.data)[0] == 20);
    ARRAY_ASSERT(((int*)array.data)[1] == 1);
    ARRAY_ASSERT(((int*)array.data)[2] == 2);
    ARRAY_ASSERT(((int*)array.data)[3] == 3);
    ARRAY_ASSERT(((int*)array.data)[4] == 12);

    c_array_destroy(&array);
  }

  // test: c_array_insert at last
  {
    CArray array2;
    err = c_array_create(sizeof(char), &array2);
    ARRAY_TEST(err);
    err = c_array_push(&array2, &(char){'\0'});
    ARRAY_TEST(err);
    err = c_array_insert(&array2, &(char){'a'}, 0);
    ARRAY_TEST(err);
    ARRAY_ASSERT(((char*)array2.data)[0] == 'a');
    ARRAY_ASSERT(((char*)array2.data)[1] == '\0');

    c_array_destroy(&array2);
  }

  // test: errors
  {
    CArray array;
    err = c_array_create(sizeof(char), &array);
    ARRAY_TEST(err);
    err = c_array_push(&array, &(char){'\0'});
    ARRAY_TEST(err);
    err = c_array_insert(&array, &(char){'a'}, 1);
    ARRAY_ASSERT(err.code == C_ARRAY_ERROR_wrong_index.code);

    c_array_destroy(&array);
  }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef NDEBUG_
#define NDEBUG
#undef NDEBUG_
#endif

#undef ARARY_STR
#undef ARARY_TEST_PRINT_ABORT
#undef ARARY_TEST
#undef ARARY_ASSERT
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
