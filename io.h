/* How To : To use this module, do this in *ONE* C file:
 *              #define CSTDLIB_IO_IMPLEMENTATION
 *              #include "io.h"
 * Tests  : To use run test, do this in *ONE* C file:
 *              #define CSTDLIB_IO_UNIT_TESTS
 *              #include "io.h"
 * License: MIT (send end of the file for details)
 */

#ifndef CSTDLIB_IO_H
#define CSTDLIB_IO_H
    #include <stdio.h>
    #include <stdbool.h>

    typedef struct File {
        FILE* data;
    } File;

    /// @brief open/create a file
    /// @param path 
    /// @param path_len 
    /// @param mode this is the same as mode in `fopen`
    /// @return 
    File
    io_file_open(const char* path, size_t path_len, const char mode[]);


    /// @brief get file size
    /// @param self 
    /// @return 
    size_t
    io_file_size(File* self);


    /// @brief read file content
    /// @param self 
    /// @param buf 
    /// @param buf_size 
    /// @return return the bytes read
    size_t
    io_file_read(File* self, char buf[], size_t buf_size);


    /// @brief write to file
    /// @param self 
    /// @param buf 
    /// @param buf_size 
    /// @return return the bytes written
    size_t
    io_file_write(File* self, char buf[], size_t buf_size);


    /// @brief close an alreay opend file
    /// @param self 
    void
    io_file_close(File* self);


    /// @brief create a directory
    /// @param dir_path 
    /// @param path_len 
    void
    io_dir_create(const char* dir_path, size_t path_len);


    /// @brief check if path is a directory
    /// @param dir_path 
    /// @param path_len 
    /// @return 
    bool
    io_dir(const char* dir_path, size_t path_len);


    /// @brief check if the directory is empty
    /// @param dir_path 
    /// @param path_len 
    /// @return 
    bool
    io_dir_is_empty(const char* dir_path, size_t path_len);


    /// @brief check if file/directory exists
    /// @param path 
    /// @param path_len 
    /// @return 
    bool
    io_exists(const char* path, size_t path_len);


    /// @brief delete a file/directory (empty directory only)
    /// @param path 
    /// @param path_len 
    void
    io_delete(const char* path, size_t path_len);


    /// @brief delete a non-empty directory
    /// @param dir_path 
    /// @param path_len 
    void
    io_delete_recursively(const char* dir_path, size_t path_len);


    /// @brief this will run `handler` on every file/directory inside `path`
    ///        [BEWARE this will fail if path is not a dir]
    /// @param dir_path 
    /// @param path_len 
    /// @param handler 
    /// @param extra_data [optional] send/recieve extra data to the handler
    void
    io_foreach(
        const char* dir_path,
        size_t path_len,
        bool handler(const char* path, size_t path_len, void* extra_data),
        void* extra_data
    );
#endif // CSTDLIB_IO_H


#ifdef CSTDLIB_IO_IMPLEMENTATION
    #include <stdint.h>
    #include <errno.h>
    #include <string.h>
    #include <stdlib.h>
    #ifdef _WIN32
    #include <windows.h>
    #else
    #include <dirent.h>
    #include <sys/stat.h>
    #endif

    #ifdef NDEBUG
    #define assert(expr) ((void)(expr))
    #else
    #include <assert.h>
    #endif

    #define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
    #define __assert_always_print(expr) (fprintf(stderr, "%s:%d: %s: Assertion `(%s)' failed.\n", __FILE__, __LINE__, __func__, #expr))
    #define assert_always(expr) ((expr) ? ((void)0) : (__assert_always_print((expr)), abort()))
    #define assert_always_msg(expr, msg) ((expr) ? ((void)0) : (__assert_always_print(expr), puts((msg)), abort()))
    #define assert_always_perror(expr, msg) ((expr) ? ((void)0) : (__assert_always_print(expr), perror((msg)), abort()))

    #ifdef _WIN32
    static size_t internal_utf8_to_unicode(const char path[], size_t path_len, wchar_t** out_wide_path);
    static size_t internal_unicode_to_utf8(const wchar_t wide_path[], size_t wide_path_len, char** out_path);
    #endif // _WIN32
    static bool internal_io_delete_recursively_handler(const char* path, size_t path_len, void* extra_data);
    static bool internal_io_dir_is_empty_handler(const char* path, size_t path_len, void* extra_data);

    File
    io_file_open(const char* path, size_t path_len, const char mode[])
    {
        assert(mode);
        assert(path);
        assert(path_len > 0);
        assert(path[path_len] == '\0');

    #if defined(_WIN32)   // MSVC on windows
        wchar_t* wide_path = NULL;
        (void)internal_utf8_to_unicode(path, path_len, &wide_path);
        wchar_t* wide_mode = NULL;
        (void)internal_utf8_to_unicode(mode, strlen(mode), &wide_mode);

        File* opened_file = _wfsopen(wide_path, wide_mode, _SH_DENYNO);
        assert_always_perror(opened_file, path);
        free(wide_path);
        free(wide_mode);
    #else
        FILE* opened_file = fopen(path, mode);
        assert_always_perror(opened_file, path);
    #endif

        return (File){ .data = opened_file };
    }

    size_t
    io_file_size(File* self)
    {
        assert(self && self->data);

        assert_always_perror(fseek(self->data, 0, SEEK_END) == 0, "");
        size_t fsize = ftell(self->data);
        assert_always_perror(fseek(self->data, 0, SEEK_SET) == 0 , "");  /* same as rewind(f); */

        return fsize;
    }

    size_t
    io_file_read(File* self, char buf[], size_t buf_size)
    {
        assert(self && self->data);
        assert(buf);
        assert(buf_size > 0);

        size_t fsize = io_file_size(self);
        size_t read_amount = MIN(buf_size, fsize);

        size_t read_size = fread(buf, sizeof(buf[0]), read_amount, self->data);
        assert_always_perror(ferror(self->data) == 0, "");

        return read_size;
    }

    size_t
    io_file_write(File* self, char buf[], size_t buf_size)
    {
        assert(self && self->data);
        assert(buf);
        assert(buf_size > 0);

        size_t write_size = fwrite(buf, sizeof(buf[0]), buf_size, self->data);
        assert_always_perror(ferror(self->data) == 0, "");

        return write_size;
    }

    void
    io_file_close(File* self)
    {
        assert(self && self->data);
        assert(fclose(self->data) == 0);

        self->data = NULL;
    }

    void
    io_dir_create(const char* dir_path, size_t path_len)
    {
        assert(dir_path);
        assert(path_len > 0);
        assert(dir_path[path_len] == '\0');

        if(!io_exists(dir_path, path_len)) {
    #if defined(_WIN32)
            assert_always(path_len < MAX_PATH);
            assert_always(CreateDirectoryA(dir_path, NULL));
    #else
            assert_always(path_len < PATH_MAX);
            const mode_t mkdir_mode_mask = 0777;
            assert_always_perror(mkdir(dir_path, mkdir_mode_mask) == 0, dir_path);
    #endif
        }
    }

    bool
    io_dir(const char* dir_path, size_t path_len)
    {
        assert(dir_path);
        assert(path_len > 0);
        assert(dir_path[path_len] == '\0');

    #if defined(_WIN32)
        assert_always(path_len < MAX_PATH);

        size_t path_attributes = GetFileAttributes(dir_path);
        return (path_attributes != INVALID_FILE_ATTRIBUTES && 
                (path_attributes & FILE_ATTRIBUTE_DIRECTORY));
    #else
        assert_always(path_len < PATH_MAX);

        struct stat sb = {0};
        return ((stat(dir_path, &sb) == 0) && S_ISDIR(sb.st_mode));
    #endif
    }

    bool
    io_dir_is_empty(const char* dir_path, size_t path_len)
    {
        assert(dir_path);
        assert(path_len > 0);
        assert(dir_path[path_len] == '\0');
        assert_always(io_dir(dir_path, path_len));

    #if defined(_WIN32)
        assert_always(path_len < MAX_PATH);
    #else
        assert_always(path_len < PATH_MAX);
    #endif

        bool is_empty = true;
        io_foreach(dir_path, path_len, internal_io_dir_is_empty_handler, &is_empty);

        return is_empty;
    }

    bool
    io_exists(const char* path, size_t path_len)
    {
        assert(path);
        assert(path_len > 0);
        assert(path[path_len] == '\0');

    #if defined(_WIN32)
        assert_always(path_len < MAX_PATH);

        size_t path_attributes = GetFileAttributesA(path);
        if (path_attributes == INVALID_FILE_ATTRIBUTES)
        {
            return false;  //something is wrong with your path!
        }

        return true;
    #else
        assert_always(path_len < PATH_MAX);
        struct stat path_stats = {0};
        return (stat(path, &path_stats) != -1);
    #endif
    }

    void
    io_delete(const char* path, size_t path_len)
    {
        assert(path);
        assert(path_len > 0);
        assert(path[path_len] == '\0');

    #if defined(_WIN32)
        assert_always(path_len < MAX_PATH);
        size_t dwAttrib = GetFileAttributes(path);

        if(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
        {
            assert_always(RemoveDirectoryA(path));
            return;
        }
    #else
        assert_always(path_len < PATH_MAX);
    #endif
        assert_always_perror((remove(path) == 0), path);
    }

    void
    io_delete_recursively(const char* dir_path, size_t path_len)
    {
        assert(dir_path);
        assert(path_len > 0);
        assert(dir_path[path_len] == '\0');
        assert_always(io_dir(dir_path, path_len));

    #if defined(_WIN32)
        assert_always(path_len < MAX_PATH);
    #else
        assert_always(path_len < PATH_MAX);
    #endif

        io_foreach(dir_path, path_len, internal_io_delete_recursively_handler, NULL);
    }

    void
    io_foreach(
        const char* dir_path,
        size_t path_len,
        bool handler(const char* path, size_t path_len, void* extra_data),
        void* extra_data
    ) {
        assert(dir_path);
        assert(path_len > 0);
        assert(handler);
        assert(dir_path[path_len] == '\0');
        assert_always(io_dir(dir_path, path_len));

    #if defined(_WIN32)
        WIN32_FIND_DATAW cur_file;
        HANDLE find_handler;
        enum { buf_len = MAX_PATH * sizeof(wchar_t) };
        wchar_t buf[buf_len];

        wchar_t* wide_path = NULL;
        size_t wide_path_len = internal_utf8_to_unicode(dir_path, path_len, &wide_path);

        assert(memcpy_s(buf, buf_len, wide_path, wide_path_len * sizeof(wchar_t)) == 0);
        buf[wide_path_len] = L'/';
        buf[wide_path_len + 1] = L'*';
        buf[wide_path_len + 2] = L'\0';
        wide_path_len += 2;

        find_handler = FindFirstFileW(buf, &cur_file);
        do
        {
            assert_always(find_handler != INVALID_HANDLE_VALUE);
            // skip '.' and '..'
            if((wcscmp(cur_file.cFileName, L".") != 0) && (wcscmp(cur_file.cFileName, L"..") != 0))
            {
                size_t filename_len = wcsnlen(cur_file.cFileName, MAX_PATH);
                assert(memcpy_s(buf + wide_path_len - 1, buf_len, cur_file.cFileName, filename_len * sizeof(wchar_t)) == 0);
                buf[wide_path_len - 1 + filename_len] = L'\0';

                char* u8path = NULL;
                size_t u8path_len = internal_unicode_to_utf8(buf, wide_path_len - 1 + filename_len, &u8path);

                bool handler_status = handler(u8path, u8path_len, extra_data);
                free(u8path);

                if(!handler_status)
                {
                    break;
                }
            }
        } while(FindNextFileW(find_handler, &cur_file));
    
        assert(FindClose(find_handler));

        free(wide_path);
    #else
        DIR *cur_dir;
        struct dirent *cur_dir_properties;
        enum { buf_len = PATH_MAX };
        char buf[buf_len];

        assert(memcpy(buf, dir_path, path_len));

        cur_dir = opendir(dir_path);
        if (cur_dir)
        {
            while ((cur_dir_properties = readdir(cur_dir)) != NULL)
            {
                if((strcmp(cur_dir_properties->d_name, ".") != 0) && (strcmp(cur_dir_properties->d_name, "..") != 0))
                {
                    size_t filename_len = strlen(cur_dir_properties->d_name);
                    buf[path_len] = '/';
                    assert(memcpy(buf + path_len + 1, cur_dir_properties->d_name, filename_len));
                    buf[path_len + 1 + filename_len] = '\0';

                    if(!handler(buf, path_len + 1 + filename_len, extra_data))
                    {
                        break;
                    }
                }

            }

            assert_always(closedir(cur_dir) != -1);
        }
    #endif
    }

    // ------------------------- internal ------------------------- //
    #if defined(_WIN32)
    #include <wchar.h>

    size_t
    internal_utf8_to_unicode(const char path[], size_t path_len, wchar_t** out_wide_path)
    {
        assert(path);
        assert(out_wide_path);
        assert(path_len > 0);

        size_t wide_path_len = MultiByteToWideChar(CP_UTF8, 0, path, (int)path_len, NULL, 0);
        assert_always(wide_path_len);

        wchar_t* wide_path = (wchar_t*)malloc((wide_path_len + 1) * sizeof(wchar_t));
        assert(wide_path);

        wide_path_len = MultiByteToWideChar(CP_UTF8, 0, path, (int)path_len, wide_path, (int)(wide_path_len * sizeof(wchar_t)));
        wide_path[wide_path_len] = '\0';
        
        *out_wide_path = wide_path;
        return wide_path_len;
    }

    size_t
    internal_unicode_to_utf8(const wchar_t wide_path[], size_t wide_path_len, char** out_path) {
        assert(wide_path);
        assert(out_path);
        assert(wide_path_len > 0);

        size_t path_len = WideCharToMultiByte(CP_UTF8, 0, wide_path, (int)wide_path_len, NULL, 0, NULL, NULL);
        assert_always(path_len);

        char* path = (char*)malloc((path_len + 1) * sizeof(char));
        assert(wide_path);

        path_len = WideCharToMultiByte(CP_UTF8, 0, wide_path, (int)wide_path_len, path, (int)path_len, NULL, NULL);
        path[path_len] = '\0';
        
        *out_path = path;
        return path_len;
    }
    #endif // _WIN32

    bool
    internal_io_delete_recursively_handler(const char* path, size_t path_len, void* extra_data)
    {
        (void)extra_data;

        if(io_dir(path, path_len))
        {
            io_delete_recursively(path, path_len);
            io_delete(path, path_len);
        }
        else
        {
            io_delete(path, path_len);
        }

        return true;
    }

    bool
    internal_io_dir_is_empty_handler(const char* path, size_t path_len, void* extra_data)
    {
        (void)path;
        (void)path_len;

        *(bool*)extra_data = false; // This dir_path is not empty
        return false;
    }
#endif // CSTDLIB_IO_IMPLEMENTATION


#ifdef CSTDLIB_IO_UNIT_TESTS
    #ifdef NDEBUG
    #define NDEBUG_
    #undef NDEBUG
    #endif

    #include <assert.h>

    void
    io_unit_tests(void)
    {
        #include <string.h>

        #define STR(str) (str), (sizeof((str)) - 1)
        #define test_playground "test_playground"

        enum { buf_len = 100 };
        char buf[buf_len];

        io_dir_create(STR(test_playground));

        // test: io_file_write
        {
            File f = io_file_open(STR(test_playground "/file_ara.txt"), "w");
            assert(io_file_write(&f, STR("بسم الله الرحمن الرحيم\n")));
            io_file_close(&f);

            f = io_file_open(STR(test_playground "/file_eng.txt"), "w");
            assert(io_file_write(&f, STR("May peace be upon you\n")));
            io_file_close(&f);
        }

        // test: io_file_read
        {
            File f = io_file_open(STR(test_playground "/file_eng.txt"), "r");
            size_t amount_read = io_file_read(&f, buf, buf_len);
            assert(amount_read > 0);
            buf[amount_read] = '\0';
            assert(strncmp(buf, "May peace be upon you\n", amount_read) == 0);
            io_file_close(&f);
            io_delete(STR(test_playground "/file_eng.txt"));
        }

        // test: io_file_read - utf8
        {
            File f = io_file_open(STR(test_playground "/file_ara.txt"), "r");
            size_t amount_read = io_file_read(&f, buf, buf_len);
            assert(amount_read);
            buf[amount_read] = '\0';
            assert(strncmp(buf, "بسم الله الرحمن الرحيم\n", amount_read) == 0);
            io_file_close(&f);
            io_delete(STR(test_playground "/file_ara.txt"));
        }


        // test: io_delete_recursively
        {
            io_dir_create(STR(test_playground "/folder"));
            File file1 = io_file_open(STR(test_playground "/folder/1.txt"), "w");
            io_file_close(&file1);
            io_dir_create(STR(test_playground "/folder/folder2"));
            File file2 = io_file_open(STR(test_playground "/folder/folder2/.2.txt"), "w");
            io_file_close(&file2);
            io_delete_recursively(STR(test_playground "/folder"));
        }

        // test: io_dir_is_empty
        {
            io_dir_create(STR(test_playground "/folder"));
            assert(io_dir_is_empty(STR(test_playground "/folder")));
            io_delete(STR(test_playground "/folder"));
        }

        // test: io_foreach
        {
            File file1 = io_file_open(STR(test_playground "/1.txt"), "w");
            io_file_close(&file1);
            bool file_found = false;
            bool handler(const char* path, size_t path_len, void* extra_data);
            io_foreach(STR(test_playground), handler, &file_found);
            assert(&file_found);
            io_delete(STR(test_playground "/1.txt"));
        }

        // test: io_delete
        {
            io_dir_create(STR(test_playground "/folder2"));
            assert(io_exists(STR(test_playground "/folder2")));
            io_delete(STR(test_playground "/folder2"));
        }

        io_delete(STR(test_playground));
    }

    bool
    handler(const char* path, size_t path_len, void* extra_data)
    {
        (void)path_len;

        if(strstr(path, "1.txt"))
        {
            *(bool*)extra_data = true;
        }

        return true;
    }

    #ifdef NDEBUG_
    #define NDEBUG
    #undef NDEBUG_
    #endif
#endif // CSTDLIB_IO_UNIT_TESTS

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
