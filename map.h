/* How To : To use this module, do this in *ONE* C file:
 *              #define CSTDLIB_MAP_IMPLEMENTATION
 *              #include "map.h"
 * Tests  : To use run test, do this in *ONE* C file:
 *              #define CSTDLIB_MAP_UNIT_TESTS
 *              #include "map.h"
 * Options :
 *           - C_MAP_DONT_CHECK_PARAMS: parameters will not get checked
 *                                      (this is off by default)
 * License: MIT (go to the end of this file for details)
 */

#ifndef CSTDLIB_MAP_H
#define CSTDLIB_MAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CMAP_DEFAULT_CAPACITY 16U

typedef struct CMap {
  void*  buckets;       // { [CMapBucket, key, value], ... }
  void*  spare_bucket1; // { [CMapBucket, key, value] }
  void*  spare_bucket2; // { [CMapBucket, key, value] }
  size_t capacity;
  size_t len;
  struct {
    size_t orig;
    size_t aligned;
  } key_size;
  struct {
    size_t orig;
    size_t aligned;
  } value_size;
  size_t bucket_size;
  size_t mask;
} CMap;

typedef struct c_map_error_t {
  int         code;
  char const* desc;
} c_map_error_t;

#define C_MAP_ERROR_none ((c_map_error_t){.code = 0, .desc = ""})
#define C_MAP_ERROR_mem_allocation                                             \
  ((c_map_error_t){.code = 1, .desc = "map: memory allocation error"})
#define C_MAP_ERROR_wrong_len                                                  \
  ((c_map_error_t){.code = 2, .desc = "map: wrong length"})
#define C_MAP_ERROR_wrong_capacity                                             \
  ((c_map_error_t){.code = 3, .desc = "map: wrong capactiy"})
#define C_MAP_ERROR_key_not_found                                              \
  ((c_map_error_t){.code = 4, .desc = "map: key not found"})
#define C_MAP_ERROR_capacity_full                                              \
  ((c_map_error_t){.code = 5, .desc = "map: capacity is full"})
#define C_MAP_ERROR_needle_not_found                                           \
  ((c_map_error_t){.code = 6, .desc = "map: needle not found"})
#define C_MAP_ERROR_empty ((c_map_error_t){.code = 7, .desc = "map: is empty"})
#define C_MAP_ERROR_wrong_range                                                \
  ((c_map_error_t){.code = 8, .desc = "map: wrong range"})
#define C_MAP_ERROR_invalid_parameters                                         \
  ((c_map_error_t){.code = 9, .desc = "map: invalid parameters"})
#define C_MAP_ERROR_wrong_iter                                                 \
  ((c_map_error_t){.code = 10, .desc = "map: invalid iter"})
#define C_MAP_iter_done ((c_map_error_t){.code = 11, .desc = ""})

c_map_error_t c_map_create(size_t key_size, size_t value_size, CMap* out_map);

c_map_error_t c_map_create_with_capacity(size_t key_size,
                                         size_t value_size,
                                         size_t capacity,
                                         CMap*  out_map);

c_map_error_t c_map_insert(CMap* self, void* key, void* value);

c_map_error_t c_map_get(CMap const* self, void* key, void** out_value);

c_map_error_t c_map_remove(CMap* self, void* key, void** out_value);

void
c_map_clear(CMap* self,
            void  element_destroy_fn(void* key, void* value, void* user_data),
            void* user_data);

size_t c_map_len(CMap const* self);

bool c_map_iter(CMap* self, size_t* iter, void** key, void** value);

void
c_map_destroy(CMap* self,
              void  element_destroy_fn(void* key, void* value, void* user_data),
              void* user_data);

#endif // CSTDLIB_MAP_H

#ifdef CSTDLIB_MAP_IMPLEMENTATION
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
  if (!(params)) return C_MAP_ERROR_invalid_parameters;
#else
#define C_ARR_CHECK_PARAMS(params) ((void)0)
#endif

typedef struct CMapBucket CMapBucket;
struct CMapBucket {
  uint64_t distance_from_initial_bucket : 16;
  uint64_t hash : 48;
};

static const size_t spare_buckets_count = 2U;

static size_t        c_internal_map_hash(const void* data, size_t data_len);
static size_t        c_internal_map_hash_fnv(const void* data, size_t data_len);
static size_t        c_internal_map_clip_hash(size_t hash);
static c_map_error_t c_internal_map_resize(CMap* self, size_t new_capacity);
static inline void*  c_internal_map_get_key(CMap const* self, size_t index);
static inline void*  c_internal_map_get_value(CMap const* self, size_t index);
static inline CMapBucket* c_internal_map_get_bucket(CMap const* self,
                                                    size_t      index);

c_map_error_t
c_map_create(size_t key_size, size_t value_size, CMap* out_map)
{
  return c_map_create_with_capacity(key_size, value_size, CMAP_DEFAULT_CAPACITY,
                                    out_map);
}

c_map_error_t
c_map_create_with_capacity(size_t key_size,
                           size_t value_size,
                           size_t capacity,
                           CMap*  out_map)
{
  C_ARR_CHECK_PARAMS(key_size > 0);
  C_ARR_CHECK_PARAMS(value_size > 0);

  if (!out_map) { return C_MAP_ERROR_none; }

  *out_map = (CMap){0};

  // make sure the capacity is multiplicate of 2
  if (capacity < CMAP_DEFAULT_CAPACITY) {
    capacity = CMAP_DEFAULT_CAPACITY;
  } else {
    size_t new_capacity = CMAP_DEFAULT_CAPACITY;
    while (new_capacity < capacity) {
      new_capacity *= 2;
    }
    capacity = new_capacity;
  }

  // make sure the key and value sizes are multiplicate of sizeof(uintptr_t)
  size_t aligned_key_size   = key_size;
  size_t aligned_value_size = value_size;
  while (aligned_key_size & (sizeof(uintptr_t) - 1)) {
    aligned_key_size++;
  }
  while (aligned_value_size & (sizeof(uintptr_t) - 1)) {
    aligned_value_size++;
  }
  size_t bucket_size
      = sizeof(CMapBucket) + aligned_key_size + aligned_value_size;

  out_map->spare_bucket1
      = malloc((capacity + spare_buckets_count) * bucket_size);
  if (!out_map->spare_bucket1) return C_MAP_ERROR_mem_allocation;
  memset(out_map->spare_bucket1, 0,
         ((capacity + spare_buckets_count) * bucket_size));
  out_map->spare_bucket2 = (char*)(out_map->spare_bucket1) + bucket_size;
  out_map->buckets       = (char*)(out_map->spare_bucket2) + bucket_size;

  out_map->capacity           = capacity;
  out_map->key_size.orig      = key_size;
  out_map->key_size.aligned   = aligned_key_size;
  out_map->value_size.orig    = value_size;
  out_map->value_size.aligned = aligned_value_size;
  out_map->bucket_size        = bucket_size;
  out_map->mask               = out_map->capacity - 1;

  return C_MAP_ERROR_none;
}

c_map_error_t
c_map_insert(CMap* self, void* key, void* value)
{
  C_ARR_CHECK_PARAMS(self && self->buckets);

  if (!key || !value) { return C_MAP_ERROR_none; }

  size_t hash  = c_internal_map_hash(key, self->key_size.orig);
  size_t index = hash & self->mask;

  if (self->capacity <= self->len) {
    c_map_error_t err = c_internal_map_resize(self, self->capacity * 2);
    if (err.code != 0) { return err; }
  }

  CMapBucket* new_bucket                   = self->spare_bucket1;
  new_bucket->distance_from_initial_bucket = 1;
  new_bucket->hash                         = hash;
  memcpy((char*)(&new_bucket[1]), key, self->key_size.orig);
  memcpy((char*)(&new_bucket[1]) + self->key_size.aligned, value,
         self->value_size.orig);

  for (;;) {
    CMapBucket* bucket     = c_internal_map_get_bucket(self, index);
    void*       bucket_key = c_internal_map_get_key(self, index);

    // [1] empty bucket
    if (bucket->distance_from_initial_bucket == 0) {
      memcpy(bucket, new_bucket, self->bucket_size);
      self->len++;
      return C_MAP_ERROR_none;
    }

    // [2] found one, same hash, update it
    /// TODO: we need external compare method
    /// TODO: we need to return old data
    if ((new_bucket->hash == bucket->hash)
        && (memcmp(bucket_key, key, self->key_size.orig) == 0)) {
      memcpy(bucket, new_bucket, self->bucket_size);
      return C_MAP_ERROR_none;
    }

    // [3] found one, different hash, collision
    if (bucket->distance_from_initial_bucket
        < new_bucket->distance_from_initial_bucket) {
      // swap
      CMapBucket* tmp = (CMapBucket*)(self->spare_bucket2);
      memcpy(tmp, bucket, self->bucket_size);
      memcpy(bucket, new_bucket, self->bucket_size);
      memcpy(new_bucket, tmp, self->bucket_size);
    }

    index = (index + 1) & self->mask;
    new_bucket->distance_from_initial_bucket++;
  }
}

c_map_error_t
c_map_get(CMap const* self, void* key, void** out_value)
{
  C_ARR_CHECK_PARAMS(self && self->buckets);

  if (!key) return C_MAP_ERROR_none;

  size_t hash = c_internal_map_hash(key, self->key_size.orig);

  for (size_t index = hash & self->mask;; index = (index + 1) & self->mask) {
    CMapBucket* bucket       = c_internal_map_get_bucket(self, index);
    void*       bucket_key   = c_internal_map_get_key(self, index);
    void*       bucket_value = c_internal_map_get_value(self, index);

    if (!bucket->distance_from_initial_bucket) {
      *out_value = NULL;
      break;
    }

    if (bucket->hash == hash) {
      if (memcmp(bucket_key, key, self->key_size.orig) == 0) {
        *out_value = bucket_value;
        break;
      }
    }
  }

  return C_MAP_ERROR_none;
}

c_map_error_t
c_map_remove(CMap* self, void* key, void** out_value)
{
  C_ARR_CHECK_PARAMS(self && self->buckets);

  if (!key || !out_value) return C_MAP_ERROR_none;

  size_t hash = c_internal_map_hash(key, self->key_size.orig);

  for (size_t index = hash & self->mask;; index = (index + 1) & self->mask) {
    CMapBucket* bucket = c_internal_map_get_bucket(self, index);
    if (!bucket->distance_from_initial_bucket) {
      return C_MAP_ERROR_key_not_found;
    }

    void* bucket_key = c_internal_map_get_key(self, index);
    if (bucket->hash == hash
        && (memcmp(bucket_key, key, self->key_size.orig) == 0)) {
      memcpy(self->spare_bucket1, bucket, self->bucket_size);
      bucket->distance_from_initial_bucket = 0;

      for (;;) {
        CMapBucket* prev   = bucket;
        index              = (index + 1) & self->mask;
        CMapBucket* bucket = c_internal_map_get_bucket(self, index);
        if (bucket->distance_from_initial_bucket <= 1) {
          prev->distance_from_initial_bucket = 0;
          break;
        }

        memcpy(prev, bucket, self->bucket_size);
        prev->distance_from_initial_bucket--;
      }

      self->len--;

      if ((self->len <= (self->capacity / 4))
          && (self->len > CMAP_DEFAULT_CAPACITY)) {
        c_internal_map_resize(self, self->capacity / 2);
      }

      *out_value = (char*)self->spare_bucket1 + sizeof(CMapBucket)
                   + self->key_size.aligned;
      break;
    }
  }

  return C_MAP_ERROR_none;
}

void
c_map_clear(CMap* self,
            void  element_destroy_fn(void* key, void* value, void* user_data),
            void* user_data)
{
  if (element_destroy_fn) {
    size_t iter = 0;
    void*  key;
    void*  value;
    while (c_map_iter(self, &iter, &key, &value)) {
      element_destroy_fn(key, value, user_data);
    }
  }

  memset(self->spare_bucket1, 0, (self->capacity + 2) * self->bucket_size);
  self->len = 0;
}

size_t
c_map_len(CMap const* self)
{
  return self->len;
}

bool
c_map_iter(CMap* self, size_t* iter, void** key, void** value)
{
  if (!iter) return false;

  for (; *iter < self->capacity; ++(*iter)) {
    CMapBucket* bucket = c_internal_map_get_bucket(self, *iter);
    if (bucket->distance_from_initial_bucket) {
      if (key) { *key = c_internal_map_get_key(self, *iter); }
      if (value) { *value = c_internal_map_get_value(self, *iter); }
      (*iter)++;
      break;
    }
  }

  if (*iter >= self->capacity) {
    return false;
  } else {
    return true;
  }
}

void
c_map_destroy(CMap* self,
              void  element_destroy_fn(void* key, void* value, void* user_data),
              void* user_data)
{
  if (self && self->buckets) {
    if (element_destroy_fn) {
      c_map_clear(self, element_destroy_fn, user_data);
    }
    free(self->spare_bucket1);
    *self = (CMap){0};
  }
}

// ------------------------- internal ------------------------- //

size_t
c_internal_map_hash(const void* data, size_t data_len)
{
  /// TODO: make the hash algo changeable
  return c_internal_map_clip_hash(c_internal_map_hash_fnv(data, data_len));
}

size_t
c_internal_map_hash_fnv(const void* data, size_t data_len)
{
  // Constants for the FNV-1a hash function
  const size_t fnv_prime = 1099511628211U;
  size_t       hash      = 14695981039346656037U; // FNV offset basis

  const uint8_t* bytes = (const uint8_t*)data;

  for (size_t i = 0; i < data_len; i++) {
    hash ^= bytes[i];  // XOR the byte with the hash
    hash *= fnv_prime; // Multiply by the FNV prime
  }

  return hash;
}

c_map_error_t
c_internal_map_resize(CMap* self, size_t new_capacity)
{
  CMap          new_map = {0};
  c_map_error_t err     = c_map_create_with_capacity(
      self->key_size.orig, self->value_size.orig, new_capacity, &new_map);
  if (err.code != 0) return err;

  for (size_t iii = 0; iii < self->capacity; iii++) {
    CMapBucket* bucket = c_internal_map_get_bucket(self, iii);
    if (!bucket->distance_from_initial_bucket) continue;

    bucket->distance_from_initial_bucket = 1;

    for (size_t new_index = bucket->hash & new_map.mask;;
         new_index        = (new_index + 1) & new_map.mask) {
      CMapBucket* new_map_bucket
          = c_internal_map_get_bucket(&new_map, new_index);

      if (new_map_bucket->distance_from_initial_bucket == 0) {
        memcpy(new_map_bucket, bucket, self->bucket_size);
        break;
      }

      if (new_map_bucket->distance_from_initial_bucket
          < bucket->distance_from_initial_bucket) {
        // swap
        memcpy(self->spare_bucket2, bucket, self->bucket_size);
        memcpy(bucket, new_map_bucket, self->bucket_size);
        memcpy(new_map_bucket, self->spare_bucket2, self->bucket_size);
      }

      bucket->distance_from_initial_bucket += 1;
    }
  }

  memcpy(new_map.spare_bucket1, self->spare_bucket1, self->bucket_size);
  memcpy(new_map.spare_bucket2, self->spare_bucket2, self->bucket_size);
  free(self->spare_bucket1);

  self->spare_bucket1 = new_map.spare_bucket1;
  self->spare_bucket2 = new_map.spare_bucket2;
  self->buckets       = new_map.buckets;
  self->capacity      = new_map.capacity;
  self->mask          = new_map.mask;

  return C_MAP_ERROR_none;
}

void*
c_internal_map_get_key(const CMap* self, size_t index)
{
  return (void*)(((char*)(c_internal_map_get_bucket(self, index)))
                 + sizeof(CMapBucket));
}

void*
c_internal_map_get_value(const CMap* self, size_t index)
{
  return (void*)(((char*)(c_internal_map_get_bucket(self, index)))
                 + sizeof(CMapBucket) + self->key_size.aligned);
}

CMapBucket*
c_internal_map_get_bucket(const CMap* self, size_t index)
{
  return (CMapBucket*)(((char*)(self->buckets)) + (self->bucket_size * index));
}

size_t
c_internal_map_clip_hash(size_t hash)
{
  return hash & 0xFFFFFFFFFFFF;
}

#undef CSTDLIB_MAP_IMPLEMENTATION
#endif // CSTDLIB_MAP_IMPLEMENTATION

#ifdef CSTDLIB_MAP_UNIT_TESTS
#ifdef NDEBUG
#define NDEBUG_
#undef NDEBUG
#endif

#include <stdio.h>

#define MAP_STR(str) str, (sizeof(str) - 1)
#define MAP_TEST_PRINT_ABORT(msg) (fprintf(stderr, "%s\n", msg), abort())
#define MAP_TEST(err)                                                          \
  ((err.code != C_MAP_ERROR_none.code) ? MAP_TEST_PRINT_ABORT(err.desc)        \
                                       : (void)0)
#define MAP_ASSERT(cond) (!(cond)) ? MAP_TEST_PRINT_ABORT(#cond) : (void)0

int
main(void)
{
  c_map_error_t err = {0};
  CMap          map = {0};
  err               = c_map_create(sizeof(char[20]), sizeof(int), &map);
  MAP_TEST(err);

  {
    err = c_map_insert(&map, (char[20]){"abc"}, &(int){1});
    MAP_TEST(err);
    err = c_map_insert(&map, (char[20]){"ahmed here"}, &(int){2});
    MAP_TEST(err);
    err = c_map_insert(&map, (char[20]){"abcd"}, &(int){3});
    MAP_TEST(err);
    err = c_map_insert(&map, (char[20]){"abc"}, &(int){4}); // test override
    MAP_TEST(err);

    int* value = NULL;

    err = c_map_get(&map, (char[20]){"abc"}, (void**)&value);
    MAP_TEST(err);
    MAP_ASSERT(value && *value == 4);

    err = c_map_get(&map, (char[20]){"abcd"}, (void**)&value);
    MAP_TEST(err);
    MAP_ASSERT(value && *value == 3);

    err = c_map_get(&map, (char[20]){"xyz"}, (void**)&value); // test not exist
    MAP_TEST(err);
    MAP_ASSERT(!value);
  }

  // test: foreach
  {
    size_t iter  = 0;
    char*  key   = NULL;
    int*   value = NULL;
    while (c_map_iter(&map, &iter, (void**)&key, (void**)&value)) {
      printf("%s: %d\n", key, *value);
    }
  }

  // test: remove
  {
    err = c_map_insert(&map, (char[20]){"new bucket"}, &(int){100});
    MAP_TEST(err);

    int* value;
    err = c_map_remove(&map, (char[20]){"new bucket"}, (void**)&value);
    MAP_TEST(err);
    MAP_ASSERT(*value == 100);
  }

  c_map_destroy(&map, NULL, NULL);
}

void
c_map_handler(void* key, void* value, void* extra_data)
{
  (void)key;
  *(int*)extra_data += *(int*)value;
}

#ifdef NDEBUG_
#define NDEBUG
#undef NDEBUG_
#endif

#undef MAP_STR
#undef MAP_TEST_PRINT_ABORT
#undef MAP_TEST
#undef MAP_ASSERT
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
