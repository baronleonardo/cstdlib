/* How To : To use this module, do this in *ONE* C file:
 *              #define CSTDLIB_MAP_IMPLEMENTATION
 *              #include "map.h"
 * Tests  : To use run test, do this in *ONE* C file:
 *              #define CSTDLIB_MAP_UNIT_TESTS
 *              #include "map.h"
 * License: MIT (send end of the file for details)
 */

#ifndef CSTDLIB_MAP_H
#define CSTDLIB_MAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

    typedef struct Map {
        void* data;
    } Map;

    /// @brief Every element of Map* has shape of
    ///        Element {
    ///            bool is_filled,
    ///            uint8_t key[max_key_size],
    ///            uint8_t value[max_value_size],
    ///        }

    Map
    map_create(size_t max_capacity, size_t max_key_size, size_t max_value_size);

    bool
    map_insert(Map* self, void* key, size_t key_size, void* value, size_t value_size);

    void*
    map_get(const Map* self, void* key, size_t key_size);

    void*
    map_remove(Map* self, void* key, size_t key_size);

    size_t
    map_len(const Map* self);

    void
    map_foreach(Map* self, void handler(void* key, void* value, void* extra_data), void* extra_data);

    void
    map_destroy(Map* self);

#endif // CSTDLIB_MAP_H


#ifdef CSTDLIB_MAP_IMPLEMENTATION
    #ifdef NDEBUG
    #define assert(expr) ((void)(expr))
    #else
    #include <assert.h>
    #endif

    #define MAP_ELEMENT_SIZE(map) (sizeof(Map) + sizeof(bool) + (map)->max_key_size + (map)->max_value_size)
    #define internal_map_get_meta(map) (((MapMeta*)(self->data))[-1])
    #define internal_map_get_data(meta) ((void*)(&meta[1]))

    typedef struct MapMeta {
        size_t capacity;
        size_t len;
        size_t max_key_size;
        size_t max_value_size;
    } MapMeta;

    typedef struct MapElement {
        bool* is_filled;
        void* key;
        void* value;
    } MapElement;

    static size_t internal_map_hasing_algo(void* key, size_t key_size, size_t capacity);
    static MapElement internal_map_get_element(const Map* self, size_t index);
    static MapElement internal_map_search_and_get(const Map* self, void* key, size_t key_size);
    
    Map
    map_create(size_t max_capacity, size_t max_key_size, size_t max_value_size)
    {
        assert(max_key_size > 0);
        assert(max_value_size > 0);

        MapMeta* meta = (MapMeta*)calloc(1, sizeof(MapMeta) + (max_capacity * (max_key_size + max_value_size + sizeof(bool))));
        assert(meta);

        meta->capacity = max_capacity;
        meta->max_key_size = max_key_size;
        meta->max_value_size = max_value_size;

        return (Map) { .data = internal_map_get_data(meta) };
    }

    bool
    map_insert(Map* self, void* key, size_t key_size, void* value, size_t value_size)
    {
        assert(self);
        assert(key_size > 0);
        assert(value);
        assert(value_size > 0);

        MapMeta* meta = &internal_map_get_meta(self);

        assert(meta->capacity > meta->len);
        assert(key_size <= meta->max_key_size);
        assert(value_size <= meta->max_value_size);

        size_t index = internal_map_hasing_algo(key, key_size, meta->capacity);
        MapElement element = internal_map_get_element(self, index);

        // collision
        // find an empty slot using `Quadratic Probing`
        if(*element.is_filled && (memcmp(key, element.key, key_size) != 0))
        {
            for(size_t iii = 1; *element.is_filled; ++iii)
            {
                index = (index + (iii * iii)) % meta->capacity;
                element = internal_map_get_element(self, index);
            }
        }

        assert(memcpy(element.key, key, key_size));
        assert(memcpy(element.value, value, value_size));
        *element.is_filled = true;

        meta->len++;
        return true;
    }

    void*
    map_get(const Map* self, void* key, size_t key_size)
    {
        assert(self && self->data);
        assert(key);
        assert(key_size > 0);

        MapElement element = internal_map_search_and_get(self, key, key_size);
        if(element.key != NULL)
        {
            return (uint8_t*)(element.value);
        }
        else
        {
            return NULL;
        }
    }

    void*
    map_remove(Map* self, void* key, size_t key_size)
    {
        assert(self && self->data);
        assert(key);
        assert(key_size > 0);

        MapElement element = internal_map_search_and_get(self, key, key_size);
        if(element.key)
        {
            MapMeta* meta = &internal_map_get_meta(self);

            meta->len--;
            *element.is_filled = false;
            assert(memset(element.key, 0, meta->max_key_size));
            return element.value;
        }
        else
        {
            return NULL;
        }
    }

    size_t
    map_len(const Map* self)
    {
        assert(self && self->data);

        MapMeta* meta = &internal_map_get_meta(self);
        return meta->len;
    }

    void
    map_foreach(Map* self, void handler(void* key, void* value, void* extra_data), void* extra_data)
    {
        assert(self && self->data);
        assert(handler);

        MapMeta* meta = &internal_map_get_meta(self);

        for(size_t iii = 0; iii < meta->len; ++iii)
        {
            MapElement element = internal_map_get_element(self, iii);
            handler(element.key, element.value, extra_data);
        }
    }

    void
    map_destroy(Map* self)
    {
        assert(self && self->data);

        MapMeta* meta = &internal_map_get_meta(self);
        free(meta);
        self->data = NULL;
    }

    // ------------------------- internal ------------------------- //
    static inline size_t
    internal_map_hasing_algo(void* key, size_t key_size, size_t capacity) {
        size_t sum = 0;
        for(size_t iii = 0; iii < key_size; ++iii) {
            sum += ((uint8_t*)key)[iii];
        }

        return sum % capacity;
    }

    static inline MapElement
    internal_map_get_element(const Map* self, size_t index)
    {
        assert(self);

        MapMeta* meta = &internal_map_get_meta(self);

        bool* is_filled_addr = (bool*)self->data + 
            (index * (meta->max_key_size + meta->max_value_size + sizeof(bool)));
        uint8_t* key_addr = (uint8_t*)is_filled_addr + sizeof(bool);
        uint8_t* value_addr = key_addr + meta->max_key_size;

        MapElement element = {
            .is_filled = is_filled_addr,
            .key = key_addr,
            .value = value_addr
        };

        return element;
    }

    static inline MapElement
    internal_map_search_and_get(const Map* self, void* key, size_t key_size)
    {
        assert(self);

        if(key)
        {
            MapMeta* meta = &internal_map_get_meta(self);
            size_t index = internal_map_hasing_algo(key, key_size, meta->capacity);
            MapElement element = internal_map_get_element(self, index);

            if(*element.is_filled)
            {
                void* element_key = element.key;

                // check for collision `Quadratic Probing`
                for(size_t iii = 0; memcmp(key, element_key, key_size) != 0; ++iii)
                {
                    if(!*element.is_filled)
                    {
                        return (MapElement){0};
                    }

                    index = (index + (iii * iii)) % meta->capacity;
                    element = internal_map_get_element(self, index);
                    element_key = element.key;
                }

                return element;
            }
        }

        return (MapElement){0};
    }
#endif // CSTDLIB_MAP_IMPLEMENTATION


#ifdef CSTDLIB_MAP_UNIT_TESTS
    #ifdef NDEBUG
    #define NDEBUG_
    #undef NDEBUG
    #endif

    #include <assert.h>

    #define STR_W_LEN(str) str, sizeof(str)
    #define INT_W_LEN(i) &(int){i}, sizeof(int)

    void map_unit_tests(void)
    {
        Map map = map_create(100, 11, sizeof(int));

        assert(map_insert(&map, STR_W_LEN("abc"), INT_W_LEN(1)));
        assert(map_insert(&map, STR_W_LEN("ahmed here"), INT_W_LEN(2)));
        assert(map_insert(&map, STR_W_LEN("abcd"), INT_W_LEN(3)));  // test collision
        assert(map_insert(&map, STR_W_LEN("abc"), INT_W_LEN(4)));   // test override

        assert(*(int*)map_get(&map, STR_W_LEN("abcd")) == 3);
        assert(*(int*)map_get(&map, STR_W_LEN("abc")) == 4);
        assert(map_get(&map, STR_W_LEN("xyz")) == NULL);             // test not exist

        map_destroy(&map);
    }

    #ifdef NDEBUG_
    #define NDEBUG
    #undef NDEBUG_
    #endif
#endif // CSTDLIB_MAP_UNIT_TESTS

/*
 * MIT License
 *
 * Copyright (c) 2019 Sean Barrett
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
