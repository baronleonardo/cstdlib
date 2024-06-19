/* How To : To use this module, do this in *ONE* C file:
 *              #define CSTDLIB_FS_IMPLEMENTATION
 *              #include "fs.h"
 * Tests  : To use run test, do this in *ONE* C file:
 *              #define CSTDLIB_FS_UNIT_TESTS
 *              #include "fs.h"
 * License: MIT (go to the end of this file for details)
 */

#ifndef CSTDLIB_FS_H
#define CSTDLIB_FS_H
#include <stdbool.h>
#include <stdio.h>

typedef struct CFile
{
  FILE* data;
} CFile;

typedef struct CPathBuffer
{
  char* buf;
  size_t len;
  size_t capacity;
} CPathBuffer;

/// @brief open/create a file
/// @param path
/// @param path_len
/// @param mode this is the same as mode in `fopen`
/// @param err return error (any value but zero is treated as an error)
/// @return
CFile c_fs_file_open (char const* path,
                      size_t path_len,
                      char const mode[],
                      int* err);

/// @brief get file size
/// @param self
/// @param err return error (any value but zero is treated as an error)
/// @return file size
size_t c_fs_file_size (CFile* self,
                       int* err);

/// @brief read file content
/// @param self
/// @param buf
/// @param buf_size
/// @param err return error (any value but zero is treated as an error)
/// @return return the bytes read
size_t c_fs_file_read (CFile* self,
                       char buf[],
                       size_t buf_size,
                       int* err);

/// @brief write to file
/// @param self
/// @param buf
/// @param buf_size
/// @param err return error (any value but zero is treated as an error)
/// @return return the bytes written
size_t c_fs_file_write (CFile* self,
                        char buf[],
                        size_t buf_size,
                        int* err);

/// @brief close an alreay opend file
/// @param self
void c_fs_file_close (CFile* self,
                      int* err);

/// @brief
/// @param path
/// @param path_len
/// @param path_capacity
/// @param err return error (any value but zero is treated as an error)
/// @return
CPathBuffer c_fs_path_buffer_create (char const path[],
                                     size_t path_len,
                                     size_t path_capacity,
                                     int* err);

/// @brief
/// @param path_capacity
/// @param err
/// @return
CPathBuffer c_fs_path_buffer_create_empty (size_t path_capacity,
                                           int* err);

/// @brief
/// @param self
/// @param new_path
/// @param new_path_len
/// @param could_realloc
/// @param err return error (any value but zero is treated as an error)
void c_fs_path_buffer_update (CPathBuffer* self,
                              char const new_path[],
                              size_t new_path_len,
                              bool could_realloc,
                              int* err);

/// @brief
/// @param path
void c_fs_path_buffer_destroy (CPathBuffer* self);

/// @brief create a directory
/// @param dir_path
/// @param path_len
/// @param err return error (any value but zero is treated as an error)
void c_fs_dir_create (char const* dir_path,
                      size_t path_len,
                      int* err);

/// @brief check if path is a directory
/// @param dir_path
/// @param path_len
/// @return
bool c_fs_dir_exists (char const* dir_path,
                      size_t path_len,
                      int* err);

/// @brief check if the directory is empty
/// @param dir_path
/// @param err return error (any value but zero is treated as an error)
/// @return
bool c_fs_dir_is_empty (CPathBuffer dir_path,
                        int* err);

/// @brief check if file/directory exists
/// @param path
/// @param path_len
/// @param err return error (any value but zero is treated as an error)
/// @return
bool c_fs_exists (char const* path,
                  size_t path_len,
                  int* err);

/// @brief delete a file/directory (empty directory only)
/// @param path
/// @param path_len
void c_fs_delete (char const* path,
                  size_t path_len,
                  int* err);

/// @brief delete a non-empty directory
/// @param dir_path
/// @param err return error (any value but zero is treated as an error)
void c_fs_delete_recursively (CPathBuffer dir_path,
                              int* err);

/// @brief this will run `handler` on every file/directory inside `path`
/// @param dir_path
/// @param handler
/// @param extra_data [optional] send/recieve extra data to the handler
/// @param err return error (any value but zero is treated as an error)
void c_fs_foreach (CPathBuffer dir_path,
                   bool handler (char* path, size_t path_len, void* extra_data, int* err),
                   void* extra_data,
                   int* err);
#endif // CSTDLIB_FS_H

#ifdef CSTDLIB_FS_IMPLEMENTATION
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

static bool internal_c_fs_delete_recursively_handler (char* path,
                                                      size_t path_len,
                                                      void* extra_data,
                                                      int* err);
static bool internal_c_fs_dir_is_empty_handler (char* path,
                                                size_t path_len,
                                                void* extra_data,
                                                int* err);

CFile
c_fs_file_open (char const* path,
                size_t path_len,
                char const mode[],
                int* err)
{
  assert (mode);
  assert (path);
  assert (path_len > 0);
  assert (path[path_len] == '\0');

  CFile cfile = { 0 };
  int error_;
  err = err ? err : &error_;
  *err = 0;

  enum
  {
    MAX_MODE_LEN = 3,
    MAX_FINAL_MODE_LEN = 15
  };

  size_t mode_len = strnlen (mode, MAX_MODE_LEN + 1);
  if (mode_len > MAX_MODE_LEN)
    {
      *err = -1;
      return cfile;
    }

#if defined(_WIN32)
  char mode_[MAX_FINAL_MODE_LEN] = { 0 };
  memcpy (mode_, mode, MAX_MODE_LEN);
  memcpy (mode_ + mode_len, "b, ccs=UTF-8", MAX_FINAL_MODE_LEN - MAX_MODE_LEN);

  FILE* opened_file = fopen (path, mode_);
#else
  FILE* opened_file = fopen (path, mode);
#endif

  if (!opened_file)
    {
#ifdef _WIN32
      *err = GetLastError ();
#else
      *err = errno;
#endif
      return cfile;
    }

  return (CFile){ .data = opened_file };
}

size_t
c_fs_file_size (CFile* self,
                int* err)
{
  assert (self && self->data);

  int error_;
  err = err ? err : &error_;
  *err = 0;

  int fseek_state = fseek (self->data, 0, SEEK_END);
  if (fseek_state != 0)
    {
      *err = ferror (self->data);
      return 0;
    }
  size_t fsize = ftell (self->data);
  fseek_state = fseek (self->data, 0, SEEK_SET); /* same as rewind(f); */
  if (fseek_state != 0)
    {
      *err = ferror (self->data);
      return 0;
    }

  return fsize;
}

size_t
c_fs_file_read (CFile* self,
                char buf[],
                size_t buf_size,
                int* err)
{
  assert (self && self->data);
  assert (buf);
  assert (buf_size > 0);

  int error_;
  err = err ? err : &error_;
  *err = 0;

  size_t fsize = c_fs_file_size (self, err);
  if (*err != 0)
    {
      return 0;
    }

  size_t read_amount = buf_size < fsize ? buf_size : fsize;

  size_t read_size = fread (buf, sizeof (buf[0]), read_amount, self->data);
  if (ferror (self->data) != 0)
    {
      *err = ferror (self->data);
      return 0;
    }

  return read_size;
}

size_t
c_fs_file_write (CFile* self,
                 char buf[],
                 size_t buf_size,
                 int* err)
{
  assert (self && self->data);
  assert (buf);
  assert (buf_size > 0);

  int error_;
  err = err ? err : &error_;
  *err = 0;

  size_t write_size = fwrite (buf, sizeof (buf[0]), buf_size, self->data);
  if (ferror (self->data) != 0)
    {
      *err = ferror (self->data);
      return 0;
    }

  return write_size;
}

void
c_fs_file_close (CFile* self, int* err)
{
  assert (self && self->data);

  int error_;
  err = err ? err : &error_;
  *err = 0;

  int close_state = fclose (self->data);
  if (close_state != 0)
    {
      *err = ferror (self->data);
      return;
    }

  *self = (CFile){ 0 };
}

CPathBuffer
c_fs_path_buffer_create (char const path[],
                         size_t path_len,
                         size_t path_capacity,
                         int* err)
{
  assert (path);
  assert (path_len > 0);
  assert (path[path_len] == '\0');

  CPathBuffer path_buf = c_fs_path_buffer_create_empty (path_capacity, err);

  if (err != 0)
    {
      path_buf.len = path_len;
      memcpy (path_buf.buf, path, path_len);
      path_buf.buf[path_buf.len] = '\0';
    }

  return path_buf;
}

CPathBuffer
c_fs_path_buffer_create_empty (size_t path_capacity,
                               int* err)
{
  assert (path_capacity > 0);

  int error_;
  err = err ? err : &error_;
  *err = 0;

  CPathBuffer path_buf = {
    .buf = malloc (path_capacity),
    .capacity = path_capacity
  };

  if (!path_buf.buf)
    {
      *err = -1;
      return (CPathBuffer){ 0 };
    }
  else
    {
      return path_buf;
    }
}

void
c_fs_path_buffer_update (CPathBuffer* self,
                         char const new_path[],
                         size_t new_path_len,
                         bool could_realloc,
                         int* err)
{
  assert (self && self->buf);
  assert (new_path);
  assert (new_path_len > 0);

  int error_;
  err = err ? err : &error_;
  *err = 0;

  if (self->capacity < new_path_len + 1)
    {
      if (could_realloc)
        {
          self->buf = realloc (self->buf, new_path_len + 1);
          if (!self->buf)
            {
              *err = -1;
              return;
            }

          self->capacity = new_path_len + 1;
        }
      else
        {
          *err = -1;
          return;
        }
    }

  memcpy (self->buf, new_path, new_path_len + 1);
  self->len = new_path_len;
}

void
c_fs_path_buffer_destroy (CPathBuffer* self)
{
  assert (self && self->buf);

  free (self->buf);
  *self = (CPathBuffer){ 0 };
}

void
c_fs_dir_create (char const* dir_path, size_t path_len, int* err)
{
  assert (dir_path);
  assert (path_len > 0);
  assert (dir_path[path_len] == '\0');

  int error_;
  err = err ? err : &error_;
  *err = 0;

#if defined(_WIN32)
  BOOL dir_created = CreateDirectoryA (dir_path, NULL);
  if (!dir_created)
    {
      *err = GetLastError ();
      return;
    }
#else
  unsigned umask_default = umask (0);
  unsigned const full_permission = 0777;
  unsigned default_permission = full_permission & ~umask_default;

  int dir_creation_status = mkdir (dir_path, default_permission);
  if (dir_creation_status != 0)
    {
      *err = errno;
      return;
    }
#endif
}

bool
c_fs_dir_exists (char const* dir_path, size_t path_len, int* err)
{
  assert (dir_path);
  assert (path_len > 0);
  assert (dir_path[path_len] == '\0');

  int error_ = -1;
  err = err ? err : &error_;
  *err = 0;

#if defined(_WIN32)
  size_t path_attributes = GetFileAttributesA (dir_path);
  if (path_attributes == INVALID_FILE_ATTRIBUTES)
    {
      *err = GetLastError ();
      return false;
    }

  else if ((path_attributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
    {
      return true;
    }
  else
    {
      *err = ERROR_DIRECTORY;
      return false;
    }
#else
  struct stat sb = { 0 };
  int path_attributes = stat (dir_path, &sb);
  if (path_attributes != 0)
    {
      *err = errno;
      return false;
    }

  if (S_ISDIR (sb.st_mode) == 0)
    {
      *err = ENOTDIR;
      return false;
    }

  return true;
#endif
}

bool
c_fs_dir_is_empty (CPathBuffer dir_path,
                   int* err)
{
  assert (dir_path.buf && dir_path.len > 0 && dir_path.capacity > 0);
  assert (dir_path.buf[dir_path.len] == '\0');

  int error_;
  err = err ? err : &error_;
  *err = 0;

  bool is_empty = true;
  c_fs_foreach (dir_path,
                internal_c_fs_dir_is_empty_handler,
                &is_empty,
                err);

  return is_empty;
}

bool
c_fs_exists (char const* path, size_t path_len, int* err)
{
  assert (path);
  assert (path_len > 0);
  assert (path[path_len] == '\0');

  int error_;
  err = err ? err : &error_;
  *err = 0;

#if defined(_WIN32)
  DWORD stat_status = GetFileAttributesA (path);
  if (stat_status == INVALID_FILE_ATTRIBUTES)
    {
      DWORD last_error = GetLastError ();
      if (last_error == ERROR_FILE_NOT_FOUND ||
          last_error == ERROR_PATH_NOT_FOUND)
        {
          return false;
        }
      else
        {
          *err = last_error;
          return false;
        }
    }

  return true;
#else
  struct stat path_stats = { 0 };
  int stat_status = stat (path, &path_stats);
  if (stat_status == 0)
    {
      return true;
    }
  else if (errno == ENOENT)
    {
      /// WARN: this could also means dangling pointer
      return false;
    }
  else
    {
      *err = errno;
      return false;
    }
#endif
}

void
c_fs_delete (char const* path, size_t path_len, int* err)
{
  assert (path);
  assert (path_len > 0);
  assert (path[path_len] == '\0');

  int error_ = -1;
  err = err ? err : &error_;
  *err = 0;

#if defined(_WIN32)
  DWORD dwAttrib = GetFileAttributesA (path);

  if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
    {
      BOOL remove_dir_status = RemoveDirectoryA (path);
      if (!remove_dir_status)
        {
          *err = remove_dir_status;
        }

      return;
    }
#endif

  int remove_status = remove (path);
  if (remove_status != 0)
    {
      *err = remove_status;
      return;
    }
}

void
c_fs_delete_recursively (CPathBuffer dir_path, int* err)
{
  assert (dir_path.buf && dir_path.len > 0 && dir_path.capacity > 0);
  assert (dir_path.buf[dir_path.len] == '\0');

  int error_;
  err = err ? err : &error_;
  *err = 0;

  c_fs_foreach (dir_path, internal_c_fs_delete_recursively_handler,
                &dir_path.capacity, err);

  if (*err != 0)
    {
      return;
    }

  // delete the directory itself after deleting all his children
  c_fs_delete (dir_path.buf, dir_path.len, err);
}

void
c_fs_foreach (CPathBuffer dir_path,
              bool handler (char* path, size_t path_len, void* extra_data, int* err),
              void* extra_data,
              int* err)
{
  assert (dir_path.buf && dir_path.len > 0 && dir_path.capacity > 0);
  assert (dir_path.buf[dir_path.len] == '\0');
  assert (handler);

  int error_;
  err = err ? err : &error_;
  *err = 0;

  bool dir_exists = c_fs_dir_exists (dir_path.buf, dir_path.len, err);
  if (!dir_exists)
    {
      return;
    }

  size_t orig_path_len = dir_path.len;

#if defined(_WIN32)
  dir_path.buf[dir_path.len] = L'/';
  dir_path.buf[dir_path.len + 1] = L'*';
  dir_path.buf[dir_path.len + 2] = L'\0';
  dir_path.len += 2;

  WIN32_FIND_DATAA cur_file;
  HANDLE find_handler = FindFirstFileA (dir_path.buf, &cur_file);
  do
    {
      if (find_handler == INVALID_HANDLE_VALUE)
        {
          *err = GetLastError ();
          dir_path.buf[dir_path.len - 2] = L'\0';
          return;
        }

      // skip '.' and '..'
      if ((strcmp (cur_file.cFileName, ".") != 0) &&
          (strcmp (cur_file.cFileName, "..") != 0))
        {
          size_t filename_len = strnlen (cur_file.cFileName,
#ifdef _WIN32
                                         INT_MAX
#else
                                         FILENAME_MAX
#endif
          );
          memcpy (dir_path.buf + dir_path.len - 1, cur_file.cFileName, filename_len);
          dir_path.buf[dir_path.len - 1 + filename_len] = L'\0';

          bool handler_status = handler (dir_path.buf,
                                         dir_path.len - 1 + filename_len,
                                         extra_data,
                                         err);

          if (!handler_status)
            {
              break;
            }
        }
    }
  while (FindNextFileA (find_handler, &cur_file));

  if (!FindClose (find_handler))
    {
      dir_path.buf[orig_path_len] = '\0';
      *err = GetLastError ();
      return;
    }
#else

  DIR* cur_dir = opendir (dir_path.buf);
  if (cur_dir)
    {
      errno = 0;

      struct dirent* cur_dir_properties;
      while ((cur_dir_properties = readdir (cur_dir)) != NULL)
        {
          if ((strcmp (cur_dir_properties->d_name, ".") != 0) &&
              (strcmp (cur_dir_properties->d_name, "..") != 0))
            {
              size_t filename_len = strlen (cur_dir_properties->d_name);
              dir_path.buf[dir_path.len] = '/';
              assert (memcpy (dir_path.buf + dir_path.len + 1,
                              cur_dir_properties->d_name,
                              filename_len));
              dir_path.buf[dir_path.len + 1 + filename_len] = '\0';

              bool handler_return_status = handler (
                  dir_path.buf,
                  dir_path.len + 1 + filename_len,
                  extra_data,
                  err);
              if (!handler_return_status)
                {
                  break;
                }
            }

          errno = 0;
        }

      if (closedir (cur_dir) != 0)
        {
          dir_path.buf[orig_path_len] = '\0';
          *err = errno;
          return;
        }
    }
#endif

  dir_path.buf[orig_path_len] = '\0';
}

// ------------------------- internal ------------------------- //
bool
internal_c_fs_delete_recursively_handler (char* path,
                                          size_t path_len,
                                          void* extra_data,
                                          int* err)
{
  (void) extra_data;

  bool is_dir = c_fs_dir_exists (path, path_len, err);

  if (is_dir && *err == 0)
    {
      c_fs_delete_recursively (
          (CPathBuffer){
              .buf = path,
              .len = path_len,
              .capacity = *(size_t*) extra_data },
          err);
      if (*err != 0)
        {
          return false;
        }
    }
  else
    {
      c_fs_delete (path, path_len, err);
      if (*err != 0)
        {
          return false;
        }
    }

  return true;
}

bool
internal_c_fs_dir_is_empty_handler (char* path,
                                    size_t path_len,
                                    void* extra_data,
                                    int* err)
{
  (void) path;
  (void) path_len;

  *(bool*) extra_data = false; // This dir_path is not empty
  if (err)
    {
      *err = 0;
    }
  return false;
}

#undef CSTDLIB_FS_IMPLEMENTATION
#endif // CSTDLIB_FS_IMPLEMENTATION

#ifdef CSTDLIB_FS_UNIT_TESTS
#ifdef NDEBUG
#define NDEBUG_
#undef NDEBUG
#endif

#include <assert.h>
#include <stdbool.h>
#include <string.h>

void
c_fs_unit_tests (void)
{

#define STR(str) (str), (sizeof ((str)) - 1)
#define test_playground "test_playground"

  enum
  {
    buf_len = 100
  };
  char buf[buf_len];
  int err = 0;

  CPathBuffer path_buf = c_fs_path_buffer_create_empty (
#ifdef WIN32
      INT16_MAX,
#else
      FILENAME_MAX,
#endif
      &err);
  assert (err == 0);

  c_fs_dir_create (STR (test_playground), &err);
  assert (err == 0);

  // test: c_fs_file_write
  {
    CFile f = c_fs_file_open (STR (test_playground "/file_ara.txt"), "w", &err);
    assert (err == 0);
    assert (c_fs_file_write (&f, STR ("بسم الله الرحمن الرحيم\n"), &err));
    assert (err == 0);
    c_fs_file_close (&f, &err);
    assert (err == 0);

    f = c_fs_file_open (STR (test_playground "/file_eng.txt"), "w", &err);
    assert (err == 0);
    assert (c_fs_file_write (&f, STR ("May peace be upon you\n"), &err));
    assert (err == 0);
    c_fs_file_close (&f, &err);
    assert (err == 0);
  }

  // test: c_fs_file_read
  {
    CFile f = c_fs_file_open (STR (test_playground "/file_eng.txt"), "r", &err);
    assert (err == 0);

    size_t amount_read = c_fs_file_read (&f, buf, buf_len, &err);
    assert (err == 0);
    assert (amount_read > 0);
    buf[amount_read] = '\0';
    assert (strncmp (buf, "May peace be upon you\n", amount_read) == 0);

    c_fs_file_close (&f, &err);
    assert (err == 0);
    c_fs_delete (STR (test_playground "/file_eng.txt"), &err);
    assert (err == 0);
  }

  // test: c_fs_file_read - utf8
  {
    CFile f = c_fs_file_open (STR (test_playground "/file_ara.txt"), "r", &err);
    assert (err == 0);

    size_t amount_read = c_fs_file_read (&f, buf, buf_len, &err);
    assert (err == 0);
    assert (amount_read);
    buf[amount_read] = '\0';
    assert (strncmp (buf, "بسم الله الرحمن الرحيم\n", amount_read) == 0);

    c_fs_file_close (&f, &err);
    assert (err == 0);
    c_fs_delete (STR (test_playground "/file_ara.txt"), &err);
    assert (err == 0);
  }

  // test: c_fs_delete_recursively
  {
    c_fs_dir_create (STR (test_playground "/folder"), &err);
    assert (err == 0);
    CFile file1 = c_fs_file_open (STR (test_playground "/folder/1.txt"), "w", &err);
    assert (err == 0);
    c_fs_file_close (&file1, &err);
    assert (err == 0);

    c_fs_dir_create (STR (test_playground "/folder/folder2"), &err);
    assert (err == 0);
    CFile file2 = c_fs_file_open (
        STR (test_playground "/folder/folder2/.2.txt"), "w", &err);
    assert (err == 0);
    c_fs_file_close (&file2, &err);
    assert (err == 0);

    memcpy (path_buf.buf, STR (test_playground "/folder") + 1);
    path_buf.len = sizeof (test_playground "/folder") - 1;
    c_fs_delete_recursively (path_buf, &err);
    assert (err == 0);
  }

  // test: c_fs_dir_is_empty
  {
    c_fs_dir_create (STR (test_playground "/folder"), &err);
    assert (err == 0);
    assert (c_fs_dir_is_empty (path_buf, &err));
    assert (err == 0);

    c_fs_delete (STR (test_playground "/folder"), &err);
    assert (err == 0);
  }

  // test: c_fs_foreach
  {
    // this will create a file
    CFile file1 = c_fs_file_open (STR (test_playground "/1.txt"), "w", &err);
    assert (err == 0);
    c_fs_file_close (&file1, &err);
    assert (err == 0);

    bool file_found = false;
    bool c_fs_handler (char* path, size_t path_len, void* extra_data, int* err);
    assert (err == 0);

    c_fs_path_buffer_update (&path_buf, STR (test_playground), true, &err);
    assert (err == 0);
    c_fs_foreach (path_buf, c_fs_handler, &file_found, &err);
    assert (err == 0);
    assert (file_found);

    c_fs_delete (STR (test_playground "/1.txt"), &err);
    assert (err == 0);
  }

  // test: c_fs_delete
  {
    c_fs_dir_create (STR (test_playground "/folder2"), &err);
    assert (err == 0);
    assert (c_fs_exists (STR (test_playground "/folder2"), &err));
    assert (err == 0);
    c_fs_delete (STR (test_playground "/folder2"), &err);
    assert (err == 0);
  }

  // test: negative results
  {
    c_fs_file_open (STR ("/ymp/file1"), "r", &err);
    assert (err != 0);

    c_fs_dir_create (STR ("/ymp/file1"), &err);
    assert (err != 0);

    c_fs_dir_exists (STR ("/ymp/file1"), &err);
    assert (err != 0);

    c_fs_exists (STR ("/ymp/file1"), &err);
    assert (err == 0);
  }

  c_fs_delete (STR (test_playground), &err);
  assert (err == 0);

  c_fs_path_buffer_destroy (&path_buf);
}

bool
c_fs_handler (char* path, size_t path_len, void* extra_data, int* err)
{
  (void) path_len;

  if (strstr (path, "1.txt"))
    {
      *(bool*) extra_data = true;
    }

  *err = 0;
  return true;
}

#ifdef NDEBUG_
#define NDEBUG
#undef NDEBUG_
#endif

#undef CSTDLIB_FS_UNIT_TESTS
#endif // CSTDLIB_FS_UNIT_TESTS

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
