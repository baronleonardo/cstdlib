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
    #include <stddef.h>
    #include <stdbool.h>

    /// @brief you can always cast 'Array' to anytype you like
    ///        and use it as oridinary heap allocated array
    typedef struct Array {
        void* data;
    } Array;

    /// @brief create an array with capacity of 1, and element_size
    ///        the meta data will be create followed by the data itself
    ///        <pre>
    ///        +----------+-----+--------------+---------+
    ///        | capacity | len | element_size | data... |
    ///        +----------+-----+--------------+---------+
    ///                                        ^
    ///                                        | user's pointer
    ///        </pre>
    ///        [NOTE: always pass the created array by reference, don't pass it with value]
    /// @param element_size 
    /// @return 
    Array
    array_create(size_t element_size);


    /// @brief same as `array_create` but with allocating capacity
    /// @param element_size 
    /// @param capacity maximum number of elements to be allocated, minimum capacity is 1
    /// @return 
    Array
    array_create_with_capacity(size_t element_size, size_t capacity);


    /// @brief check wether the array is empty
    /// @param self 
    /// @return
    bool
    array_is_empty(Array* self);


    /// @brief get array length
    /// @param self 
    /// @return array length
    size_t
    array_len(Array* self);


    /// @brief set array length
    ///        [beware this is DANGEROUS]
    ///        this is useful if you want
    ///        to manipulate the data by yourself
    /// @param self 
    /// @param new_len 
    void
    array_set_len(Array* self, size_t new_len);


    /// @brief get array capacity
    ///        this will return the capacity in 'element_size' wise
    ///        example: 'capacity = 10' means
    ///                 we can have up to '10 * element_size' bytes
    /// @param self 
    /// @return the capacity of the array
    size_t
    array_capacity(Array* self);


    /// @brief set capacity
    ///        [beware this is DANGEROUS]
    ///        this is useful if you want
    ///        to manipulate the data by yourself
    /// @param self address of self
    /// @param new_capacity 
    void
    array_set_capacity(Array* self, size_t new_capacity);


    /// @brief get elemet_size in bytes
    /// @param self 
    /// @return elemet_size in bytes
    size_t
    array_element_size(Array* self);


    /// @brief push one element at the end
    /// @param self pointer to self
    /// @param element if you want to push literals (example: 3, 5 or 10 ...)
    ///                array_push(array, &(int){3});
    void
    array_push(Array* self, const void* element);


    /// @brief pop one element from the end
    ///        [this will NOT resize the array]
    /// @param self 
    /// @return pointer of the popped element
    void*
    array_pop(Array* self);


    /// @brief insert 1 element at index
    /// @param self pointer to self
    /// @param element 
    /// @param index 
    void
    array_insert(Array* self, const void* element, size_t index);


    /// @brief insert multiple elements at index
    /// @param self 
    /// @param index 
    /// @param data 
    /// @param data_len 
    void
    array_insert_range(Array* self, size_t index, const void* data, size_t data_len);


    /// @brief remove element from Array
    ///        [beware this function is costy]
    /// @param self 
    /// @param index index to be removed
    void
    array_remove(Array* self, size_t index);


    /// @brief remove a range of elements from Array
    /// @param self 
    /// @param start_index 
    /// @param range_len range length
    void
    array_remove_range(Array* self, size_t start_index, size_t range_len);

    /// @brief destroy the array from the memory
    /// @param self 
    void
    array_destroy(Array* self);
#endif // CSTDLIB_ARRAY_H


#ifdef CSTDLIB_ARRAY_IMPLEMENTATION
    #include <stdlib.h>
    #include <string.h>
    #include <stdint.h>

    #ifdef NDEBUG
    #define assert(expr) ((void)(expr))
    #else
    #include <assert.h>
    #endif

    /// @brief the design inspired by the wonderful cvector
    ///        https://github.com/eteran/c-vector
    typedef struct ArrayMeta {
        size_t capacity;     /// maximum data that can be hold, note: this unit based not bytes based
        size_t len;          /// current length, note: this unit based not bytes based
        size_t element_size; /// size of the unit
    } ArrayMeta;

    #define internal_array_meta(array) (((ArrayMeta*)(array->data))[-1])
    #define internal_array_data(array_meta) ((void*)(&array_meta[1]))

    Array
    array_create(size_t element_size) {
        return array_create_with_capacity(element_size, 1U);
    }

    Array
    array_create_with_capacity(size_t element_size, size_t capacity) {
        assert(element_size > 0);
        assert(capacity > 0);

        ArrayMeta* meta = (ArrayMeta*)calloc(1U, sizeof(ArrayMeta) + (capacity * element_size));
        assert(meta);

        meta->capacity = capacity;
        meta->element_size = element_size;

        return (Array){ .data = internal_array_data(meta) };
    }

    bool
    array_is_empty(Array* self) {
        assert(self && self->data);

        return internal_array_meta(self).len == 0;
    }

    size_t
    array_len(Array* self) {
        assert(self && self->data);

        return internal_array_meta(self).len;
    }

    void
    array_set_len(Array* self, size_t new_len) {
        assert(self && self->data);

        ArrayMeta* meta = &internal_array_meta(self);

        assert(new_len < meta->capacity);
        meta->len = new_len;
    }

    size_t
    array_capacity(Array* self) {
        assert(self && self->data);

        return internal_array_meta(self).capacity;
    }

    void
    array_set_capacity(Array* self, size_t new_capacity) {
        assert(self && self->data);
        assert(new_capacity > 0);

        ArrayMeta* meta = &internal_array_meta(self);

        meta = realloc(meta, sizeof(ArrayMeta) + (new_capacity * meta->element_size));
        assert(meta);

        meta->capacity = new_capacity;

        self->data = internal_array_data(meta);
    }

    size_t
    array_element_size(Array* self) {
        assert(self && self->data);

        return internal_array_meta(self).element_size;
    }

    void
    array_push(Array* self, const void* element) {
        assert(self && self->data);
        assert(element);

        ArrayMeta* meta = &internal_array_meta(self);
        assert(meta->len < SIZE_MAX);

        if((meta->len + 1U) > meta->capacity) {
            meta->capacity *= 2;
            meta = realloc(meta, sizeof(ArrayMeta) + (meta->capacity * meta->element_size));
            assert(meta);
        }

        assert(memcpy(
            (uint8_t*)internal_array_data(meta) + (meta->len * meta->element_size),
            element,
            meta->element_size
        ));

        meta->len++;

        self->data = internal_array_data(meta);
    }

    void*
    array_pop(Array* self) {
        assert(self && self->data);

        ArrayMeta* meta = &internal_array_meta(self);
        assert(meta->len > 0U);

        uint8_t* element = (uint8_t*)internal_array_data(meta) + ((meta->len - 1U) * meta->element_size);
        meta->len--;

        return element;
    }

    void
    array_insert(Array* self, const void* element, size_t index) {
        assert(self && self->data);

        ArrayMeta* meta = &internal_array_meta(self);
        assert(meta->len < SIZE_MAX);
        assert(meta->len > index);

        if((meta->len + 1U) > meta->capacity) {
            meta->capacity *= 2;
            meta = realloc(meta, sizeof(ArrayMeta) + (meta->capacity * meta->element_size));
            assert(meta);
        }

        if(index < meta->len) {
            assert(memmove(
                (uint8_t*)internal_array_data(meta) + ((index + 1) * meta->element_size),
                (uint8_t*)internal_array_data(meta) + (index * meta->element_size),
                (meta->len - index) * meta->element_size
            ));
        }

        assert(memcpy(
                (uint8_t*)internal_array_data(meta) + (index * meta->element_size),
                element,
                meta->element_size
            ));

        meta->len++;

        self->data = internal_array_data(meta);
    }

    void
    array_insert_range(Array* self, size_t index, const void* data, size_t data_len) {
        assert(self && self->data);

        ArrayMeta* meta = &internal_array_meta(self);
        assert(meta->len < SIZE_MAX);
        assert(meta->len > index);

        while((meta->len + data_len) > meta->capacity) {
            meta->capacity *= 2;
            meta = realloc(meta, sizeof(ArrayMeta) + (meta->capacity * meta->element_size));
            assert(meta);
        }

        if(index < meta->len) {
            assert(memmove(
                (uint8_t*)internal_array_data(meta) + ((index + data_len) * meta->element_size),
                (uint8_t*)internal_array_data(meta) + (index * meta->element_size),
                (meta->len - index) * meta->element_size
            ));
        }

        assert(memcpy(
                (uint8_t*)internal_array_data(meta) + (index * meta->element_size),
                data,
                data_len * meta->element_size
            ));

        meta->len += data_len;

        self->data = internal_array_data(meta);
    }

    void
    array_remove(Array* self, size_t index) {
        assert(self && self->data);

        ArrayMeta* meta = &internal_array_meta(self);
        assert(meta->len > 0);

        void* array_data = internal_array_data(meta);
        uint8_t* element = (uint8_t*)array_data + (index * meta->element_size);

        assert(memmove(
            element,
            element + meta->element_size,
            (meta->len - index) * meta->element_size
        ));

        meta->len--;
    }

    void
    array_remove_range(Array* self, size_t start_index, size_t range_len) {
        assert(self && self->data);

        ArrayMeta* meta = &internal_array_meta(self);
        assert(meta->len > 0U);
        assert(start_index < (meta->len - 1U));
        assert((start_index + range_len) <= meta->len);

        uint8_t* start_ptr = (uint8_t*)internal_array_data(meta) + (start_index * meta->element_size);

        const uint8_t* end_ptr = (uint8_t*)internal_array_data(meta) + ((start_index + range_len) * meta->element_size);
        size_t right_range_size = (meta->len - (start_index + range_len)) * meta->element_size;

        assert(memmove(start_ptr, end_ptr, right_range_size));

        meta->len -= range_len;
    }

    void
    array_destroy(Array* self) {
        assert(self && self->data);

        ArrayMeta* meta = &internal_array_meta(self);
        free(meta);

        self->data = NULL;
    }
#endif // CSTDLIB_ARRAY_IMPLEMENTATION


#ifdef CSTDLIB_ARRAY_UNIT_TESTS
    #ifdef NDEBUG
    #define NDEBUG_
    #undef NDEBUG
    #endif

    #include <assert.h>

    void array_unit_tests(void)
    {
        // test: general
        {
            Array array = array_create (sizeof (int));

            // test_push
            array_push (&array, &(int){ 12 });
            array_push (&array, &(int){ 13 });
            array_push (&array, &(int){ 14 });
            array_push (&array, &(int){ 15 });
            array_push (&array, &(int){ 16 });

            assert (array_len (&array) == 5);

            // test array_pop
            const int *data = array_pop (&array);
            assert (*data == 16);

            // test array_remove_range
            array_remove_range (&array, 1, 3);
            assert (array_len (&array) == 1);
            assert (((int *)array.data)[0] == 12);

            // test array_insert
            array_insert (&array, &(int){ 20 }, 0);
            assert (((int *)array.data)[0] == 20);
            assert (((int *)array.data)[1] == 12);

            // test array_insert
            array_insert_range (&array, 1, &(int[]){ 1, 2, 3 }, 3);
            assert (((int *)array.data)[0] == 20);
            assert (((int *)array.data)[1] == 1);
            assert (((int *)array.data)[2] == 2);
            assert (((int *)array.data)[3] == 3);
            assert (((int *)array.data)[4] == 12);

            array_destroy (&array);
        }

        // test: array_insert at last
        {
            Array array2 = array_create (sizeof (char));
            array_push (&array2, &(char){ '\0' });
            array_insert (&array2, &(char){ 'a' }, 0);
            assert (((char *)array2.data)[0] == 'a');
            assert (((char *)array2.data)[1] == '\0');

            array_destroy (&array2);
        }
    }

    #ifdef NDEBUG_
    #define NDEBUG
    #undef NDEBUG_
    #endif
#endif // CSTDLIB_ARRAY_UNIT_TESTS

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
