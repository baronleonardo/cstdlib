/* How To : To use this module, do this in *ONE* C file:
 *              #define CSTDLIB_STR_IMPLEMENTATION
 *              #include "str.h"
 * Tests  : To use run test, do this in *ONE* C file:
 *              #define CSTDLIB_STR_UNIT_TESTS
 *              #include "str.h"
 * License: MIT (send end of the file for details)
 */

#ifndef CSTDLIB_STR_H
#define CSTDLIB_STR_H
    #include <stddef.h>
    #include <stdint.h>
    #include <stdbool.h>

    typedef struct Str {
        char* data;
    } Str;

    Str
    str_create(const char* cstring, size_t max_len);

    void
    str_add(Str* self, const char* cstring, size_t max_len);

    char*
    str_search(Str* self, const char* cstring, size_t max_len);

    bool
    str_remove(Str* self, const char* cstring, size_t max_len);

    size_t
    str_len(const Str* self);

    size_t
    str_capacity(const Str* self);

    void
    str_destroy(Str* self);
#endif // CSTDLIB_STR_H


#ifdef CSTDLIB_STR_IMPLEMENTATION
    #include <string.h>
    #include <stdlib.h>

    #ifdef NDEBUG
    #define assert(expr) ((void)(expr))
    #else
    #include <assert.h>
    #endif

    #define internal_str_get_meta(str) (((StrMeta*)(self->data))[-1])
    #define internal_str_get_data(meta) ((char*)(&(meta[1])))

    /// @brief the design inspired by the wonderful sds
    ///        https://github.com/antirez/sds
    typedef struct StrMeta {
        size_t capacity;
        size_t len;
    } StrMeta;

    static char* internal_str_search(Str* self, const char* cstring, size_t max_len, StrMeta** return_meta, size_t* return_cstring_len);

    Str
    str_create(const char* cstring, size_t max_len)
    {
        assert(max_len > 0);

        size_t cstring_len = strnlen(cstring, max_len);

        StrMeta* meta = (StrMeta*)malloc(sizeof(StrMeta) + cstring_len + 1);
        assert(meta);

        meta->capacity = cstring_len + 1;
        meta->len = cstring_len;
        assert(memcpy(internal_str_get_data(meta), cstring, cstring_len));
        internal_str_get_data(meta)[cstring_len] = '\0';

        return (Str) { .data = internal_str_get_data(meta) };
    }

    void
    str_add(Str* self, const char* cstring, size_t max_len)
    {
        assert(self && self->data);
        assert(cstring);
        assert(max_len > 0);

        StrMeta* meta = &internal_str_get_meta(self);
        size_t cstring_len = strnlen(cstring, max_len);

        // resize
        if((meta->len + cstring_len + 1) > meta->capacity) {
            meta = realloc(meta, sizeof(StrMeta) + (meta->capacity + cstring_len));
            assert(meta);

            meta->capacity += cstring_len;
            self->data = internal_str_get_data(meta);
        }

    #if defined(_WIN32)
        assert(strncpy_s((Str*)internal_str_get_data(meta) + meta->len, meta->capacity - meta->len, cstring, cstring_len) == 0);
    #else
        assert(strncpy(internal_str_get_data(meta) + meta->len, cstring, cstring_len));
    #endif
        meta->len += cstring_len;
        internal_str_get_data(meta)[meta->len] = '\0';
    }

    bool
    str_remove(Str* self, const char* cstring, size_t max_len)
    {
        assert(self && self->data);
        assert(cstring);
        assert(max_len > 0);

        StrMeta* meta = NULL;
        size_t cstring_len = 0;
        char* substring_ptr = internal_str_search(self, cstring, max_len, &meta, &cstring_len);

        if(substring_ptr)
        {
            assert(memmove(
                substring_ptr,
                substring_ptr + cstring_len,
                (internal_str_get_data(meta) + meta->len) - (substring_ptr + cstring_len))
            );
            *substring_ptr = '\0';
            meta->len -= cstring_len;
            return true;
        }
        else
        {
            return false;
        }
    }

    char*
    str_search(Str* self, const char* cstring, size_t max_len)
    {
        assert(self && self->data);
        assert(cstring);
        assert(max_len > 0);

        return internal_str_search(self, cstring, max_len, NULL, NULL);
    }

    size_t
    str_len(const Str* self)
    {
        assert(self && self->data);

        StrMeta* meta = &internal_str_get_meta(self);
        return meta->len;
    }

    size_t
    str_capacity(const Str* self)
    {
        assert(self && self->data);

        StrMeta* meta = &internal_str_get_meta(self);
        return meta->capacity;
    }

    void
    str_destroy(Str* self)
    {
        assert(self && self->data);

        StrMeta* meta = &internal_str_get_meta(self);
        free(meta);
        self->data = NULL;
    }

    // ------------------------- internal ------------------------- //
    char*
    internal_str_search(Str* self, const char* cstring, size_t max_len, StrMeta** return_meta, size_t* return_cstring_len)
    {
        assert(self);

        if(cstring)
        {
            StrMeta* meta = &internal_str_get_meta(self);
            *return_meta = meta;
            size_t cstring_len = strnlen(cstring, max_len);
            *return_cstring_len = cstring_len;

            for(size_t str_counter = 0; str_counter < meta->len; ++str_counter)
            {
                if(memcmp(&(self->data)[str_counter], cstring, cstring_len) == 0)
                {
                    return &(self->data)[str_counter];
                }
            }
        }

        return NULL;
    }
#endif // CSTDLIB_STR_IMPLEMENTATION


#ifdef CSTDLIB_STR_UNIT_TESTS
    #ifdef NDEBUG
    #define NDEBUG_
    #undef NDEBUG
    #endif

    #include <assert.h>
    #include <string.h>

    void str_unit_tests(void)
    {
        Str str = str_create("Ahmed is here", 100);

        assert(str_remove(&str, "here", 100));
        assert(strcmp(str.data, "Ahmed is ") == 0);

        str_add(&str, "here", 100);
        assert(strcmp(str.data, "Ahmed is here") == 0);

        str_destroy(&str);
    }

    #ifdef NDEBUG_
    #define NDEBUG
    #undef NDEBUG_
    #endif
#endif // CSTDLIB_STR_UNIT_TESTS

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
