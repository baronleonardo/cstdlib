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
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct CFile
{
  FILE* raw;
} CFile;

typedef struct CPath
{
  char* buf;
  size_t len;
  size_t capacity;
} CPath;

typedef struct cerror_t
{
  int code;
  char const* msg;
} cerror_t;

#define CERROR_NONE ((cerror_t){ 0, "" })
#define CERROR_OUT_IS_NULL ((cerror_t){ 256, "the out pointer is NULL" })
#define CERROR_WRONG_STR_LEN ((cerror_t){ 257, "invalid string (wrong length)" })
#define CERROR_MEM_ALLOCATION ((cerror_t){ 257, "memory allocation error" })
#define CERROR_PATH_BUFFER_FULL ((cerror_t){ 258, "CPath is full" })
#define CERROR_SMALL_BUFFER ((cerror_t){ 259, "buffer is small" })
#define CERROR_SMALL_PATH_CAPACITY ((cerror_t){ 260, "path capacity is small" })

/// @brief open/create a file
/// @param path
/// @param path_len
/// @param mode this is the same as mode in `fopen`
/// @param out_cfile return the new object
/// @return error
cerror_t c_fs_file_open (CPath const* path,
                         char const mode[],
                         CFile* out_cfile);

/// @brief
/// @param path
/// @param path_len
/// @param mode
/// @param out_cfile
/// @return
cerror_t c_fs_file_open_r (char const* path,
                           size_t path_len,
                           char const mode[],
                           CFile* out_cfile);

/// @brief get file size
/// @param self
/// @param out_size return the file size
/// @return error
cerror_t c_fs_file_size (CFile* self, size_t* out_size);

/// @brief read file content
/// @param self
/// @param buf
/// @param buf_size
/// @param err return error (any value but zero is treated as an error)
/// @return return the bytes read
cerror_t c_fs_file_read (CFile* self,
                         char buf[],
                         size_t buf_size,
                         size_t* out_size);

/// @brief write to file
/// @param self
/// @param buf
/// @param buf_size
/// @param err return error (any value but zero is treated as an error)
/// @return return the bytes written
cerror_t c_fs_file_write (CFile* self,
                          char buf[],
                          size_t buf_size,
                          size_t* out_size);

/// @brief close an alreay opend file
/// @param self
cerror_t c_fs_file_close (CFile* self);

/// @brief
/// @param path
/// @param path_len
/// @param path_capacity
/// @param err return error (any value but zero is treated as an error)
/// @return
cerror_t c_fs_path_create (char const path[],
                           size_t path_len,
                           size_t path_capacity,
                           CPath* out_path);

/// @brief
/// @param path_capacity
/// @param err
/// @return
cerror_t c_fs_path_create_empty (size_t path_capacity, CPath* out_path);

/// @brief this will append `path` to `CPath` and adding '/' in between
/// @param self
/// @param new_path
/// @param new_path_len
/// @param could_realloc
/// @param err return error (any value but zero is treated as an error)
cerror_t c_fs_path_append (CPath* self,
                           char const path[],
                           size_t path_len,
                           bool could_realloc);

/// @brief
/// @param self
/// @param could_realloc
/// @return
cerror_t c_fs_path_to_absolute (CPath* self, bool could_realloc);

/// @brief
/// @param path
void c_fs_path_destroy (CPath* self);

/// @brief create a directory
/// @param path
/// @return err return error (any value but zero is treated as an error)
cerror_t c_fs_dir_create (CPath const* path);

/// @brief create a directory with raw path
/// @param dir_path
/// @param path_len
/// @param err return error (any value but zero is treated as an error)
cerror_t c_fs_dir_create_r (char const dir_path[], size_t path_len);

/// @brief check if path is a directory
/// @param dir_path
/// @param out_exists
/// @return
cerror_t c_fs_dir_exists (CPath const* dir_path,
                          bool* out_exists);

/// @brief check if path is a directory
/// @param dir_path
/// @param path_len
/// @return
cerror_t c_fs_dir_exists_r (char const dir_path[],
                            size_t path_len,
                            bool* out_exists);

/// @brief
/// @param dir_path
/// @param out_size
/// @return
cerror_t c_fs_dir_get_current (CPath* dir_path, size_t* out_size);

/// @brief
/// @param dir_path
/// @param dir_path_len
/// @param out_size
/// @return
cerror_t c_fs_dir_get_current_r (char dir_path[], size_t dir_path_len, size_t* out_size);

cerror_t c_fs_dir_change_current (CPath const* dir_path);

/// @brief
/// @param dir_path
/// @param dir_path_len
/// @return
cerror_t c_fs_dir_change_current_r (char const dir_path[], size_t dir_path_len);

/// @brief check if the directory is empty
/// @param dir_path
/// @param err return error (any value but zero is treated as an error)
/// @return
cerror_t c_fs_dir_is_empty (CPath const* dir_path, bool* out_is_empty);

/// @brief
/// @param dir_path
/// @param dir_path_len
/// @param out_is_empty
/// @return
cerror_t c_fs_dir_is_empty_r (char const dir_path[], size_t dir_path_len, bool* out_is_empty);

/// @brief
/// @param path
/// @param out_exists
/// @return
cerror_t c_fs_exists (CPath const* path, bool* out_exists);

/// @brief check if file/directory exists
/// @param path
/// @param path_len
/// @param err return error (any value but zero is treated as an error)
/// @return
cerror_t c_fs_exists_r (char const path[], size_t path_len, bool* out_exists);

/// @brief
/// @param path
/// @return
cerror_t c_fs_delete (CPath const* path);

/// @brief delete a file/directory (empty directory only) with raw path
/// @param path
/// @param path_len
cerror_t c_fs_delete_r (char const path[], size_t path_len);

/// @brief delete a non-empty directory
/// @param dir_path
/// @param err return error (any value but zero is treated as an error)
cerror_t c_fs_delete_recursively (CPath* dir_path);

/// @brief
/// @param path
/// @param path_len
/// @return
cerror_t c_fs_delete_recursively_r (char path[], size_t path_len, size_t path_capacity);

/// @brief this will run `handler` on every file/directory inside `path`
/// @param path
/// @param handler
/// @param extra_data [optional] send/recieve extra data to the handler
/// @param err return error (any value but zero is treated as an error)
cerror_t
c_fs_foreach (CPath* path,
              cerror_t handler (CPath* path, void* extra_data),
              void* extra_data);
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
#include <unistd.h>
#endif

#ifdef _WIN32
#define PATH_SEP '\\'
#define MAX_PATH_LEN INT16_MAX
#else
#define MAX_PATH_LEN FILENAME_MAX
#define PATH_SEP '/'
#endif

#if _WIN32 && (!_MSC_VER || !(_MSC_VER >= 1900))
#error "You need MSVC must be higher that or equal to 1900
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996) // disable warning about unsafe functions
#endif

static cerror_t internal_c_fs_delete_recursively_handler (CPath* path,
                                                          void* extra_data);
static cerror_t internal_c_fs_dir_is_empty_handler (CPath* path,
                                                    void* extra_data);

static inline cerror_t
errno_to_cerror (int errno_)
{
  return (cerror_t){ errno_, strerror (errno_) };
}

cerror_t
c_fs_file_open (CPath const* path,
                char const mode[],
                CFile* out_cfile)
{
  assert (path);

  return c_fs_file_open_r (path->buf, path->len, mode, out_cfile);
}

cerror_t
c_fs_file_open_r (char const* path,
                  size_t path_len,
                  char const mode[],
                  CFile* out_cfile)
{
  assert (mode);
  assert (path);
  assert (path_len > 0);
  assert (path[path_len] == '\0');

  if (out_cfile)
    {
      *out_cfile = (CFile){ 0 };

      enum
      {
        MAX_MODE_LEN = 3,
        MAX_FINAL_MODE_LEN = 15
      };

      size_t mode_len = strnlen (mode, MAX_MODE_LEN + 1);
      if (mode_len > MAX_MODE_LEN)
        {
          return CERROR_WRONG_STR_LEN;
        }

#if defined(_WIN32)
      char mode_[MAX_FINAL_MODE_LEN] = { 0 };
      memcpy (mode_, mode, MAX_MODE_LEN);
      memcpy (mode_ + mode_len,
              "b, ccs=UTF-8",
              MAX_FINAL_MODE_LEN - MAX_MODE_LEN);

      SetLastError (0);
      out_cfile->raw = fopen (path, mode_);
#else
      out_cfile->raw = fopen (path, mode);
#endif

      if (!out_cfile->raw)
        {
#ifdef _WIN32
          return (cerror_t){ GetLastError (), "" };
#else
          return errno_to_cerror (errno);
#endif
        }

      return CERROR_NONE;
    }

  return CERROR_OUT_IS_NULL;
}

cerror_t
c_fs_file_size (CFile* self, size_t* out_size)
{
  assert (self && self->raw);

  if (out_size)
    {
      clearerr (self->raw);
      int fseek_state = fseek (self->raw, 0, SEEK_END);
      if (fseek_state != 0)
        {
          *out_size = 0;
          return errno_to_cerror (ferror (self->raw));
        }
      *out_size = ftell (self->raw);
      fseek_state = fseek (self->raw, 0, SEEK_SET); /* same as rewind(f); */
      if (fseek_state != 0)
        {
          *out_size = 0;
          return errno_to_cerror (ferror (self->raw));
        }

      return CERROR_NONE;
    }

  return CERROR_OUT_IS_NULL;
}

cerror_t
c_fs_file_read (CFile* self,
                char buf[],
                size_t buf_size,
                size_t* out_size)
{
  assert (self && self->raw);
  assert (buf);
  assert (buf_size > 0);

  clearerr (self->raw);
  size_t read_size = fread (buf, sizeof (buf[0]), buf_size, self->raw);

  if (out_size)
    *out_size = read_size;

  return read_size > 0 ? CERROR_NONE : errno_to_cerror (ferror (self->raw));
}

cerror_t
c_fs_file_write (CFile* self,
                 char buf[],
                 size_t buf_size,
                 size_t* out_size)
{
  assert (self && self->raw);
  assert (buf);
  assert (buf_size > 0);

  clearerr (self->raw);
  size_t write_size = fwrite (buf, sizeof (buf[0]), buf_size, self->raw);

  if (out_size)
    *out_size = write_size;

  return write_size > 0 ? CERROR_NONE : errno_to_cerror (ferror (self->raw));
}

cerror_t
c_fs_file_close (CFile* self)
{
  assert (self && self->raw);

  clearerr (self->raw);
  fclose (self->raw);
  cerror_t err = errno_to_cerror (ferror (self->raw));

  *self = (CFile){ 0 };
  return err;
}

cerror_t
c_fs_path_create (char const path[],
                  size_t path_len,
                  size_t path_capacity,
                  CPath* out_cpath)
{
  assert (path);
  assert (path_len > 0);
  assert (path[path_len] == '\0');

  cerror_t err = c_fs_path_create_empty (path_capacity, out_cpath);

  if (err.code == CERROR_NONE.code)
    {
      out_cpath->len = path_len;
      memcpy (out_cpath->buf, path, path_len);
      out_cpath->buf[out_cpath->len] = '\0';
    }

  return err;
}

cerror_t
c_fs_path_create_empty (size_t path_capacity, CPath* out_cpath)
{
  assert (path_capacity > 0);

  if (out_cpath)
    {
      *out_cpath =
          (CPath){ .buf = malloc (path_capacity), .capacity = path_capacity };

      if (!out_cpath->buf)
        {
          return CERROR_MEM_ALLOCATION;
        }

      return CERROR_NONE;
    }

  return CERROR_OUT_IS_NULL;
}

cerror_t
c_fs_path_append (CPath* self,
                  char const path[],
                  size_t path_len,
                  bool could_realloc)
{
  assert (self && self->buf);
  assert (path);
  assert (path_len > 0);

  if (self->capacity < path_len + 1)
    {
      if (could_realloc)
        {
          char* reallocated_buf =
              realloc (self->buf, self->len + path_len + 2);
          if (!reallocated_buf)
            {
              return CERROR_MEM_ALLOCATION;
            }

          self->buf = reallocated_buf;
          self->capacity = self->len + path_len + 2;
        }
      else
        {
          return CERROR_PATH_BUFFER_FULL;
        }
    }

  self->buf[self->len++] = PATH_SEP;
  memcpy (self->buf + self->len, path, path_len + 1);
  self->len += path_len;
  self->buf[self->len] = '\0';

  return CERROR_NONE;
}

cerror_t
c_fs_path_replace (CPath* self,
                   char const new_path[],
                   size_t new_path_len,
                   bool could_realloc)
{
  assert (self && self->buf);
  assert (new_path);
  assert (new_path_len > 0);

  if (self->capacity < new_path_len + 1)
    {
      if (could_realloc)
        {
          char* reallocated_buf = realloc (self->buf, new_path_len + 1);
          if (!reallocated_buf)
            {
              return CERROR_MEM_ALLOCATION;
            }

          self->buf = reallocated_buf;
          self->capacity = new_path_len + 1;
        }
      else
        {
          return CERROR_PATH_BUFFER_FULL;
        }
    }

  memcpy (self->buf, new_path, new_path_len);
  self->len = new_path_len;
  self->buf[self->len] = '\0';

  return CERROR_NONE;
}

cerror_t
c_fs_path_to_absolute (CPath* self, bool could_realloc)
{
  (void) self;
  (void) could_realloc;

  if (could_realloc)
    {
      char* reallocated_buf = realloc (self->buf, MAX_PATH_LEN);
      if (!reallocated_buf)
        {
          return CERROR_MEM_ALLOCATION;
        }
      self->buf = reallocated_buf;
      self->capacity = MAX_PATH_LEN;
    }

#ifdef _WIN32
  SetLastError (0);
  self->len = GetLongPathNameA (self->buf, self->buf, (DWORD) self->capacity);
  return self->len > 0 ? CERROR_NONE : (cerror_t){ GetLastError (), "" };
#else
  errno = 0;
  char* path_status = realpath (self->buf, self->buf);
  self->len = strlen (self->buf);
  return path_status ? CERROR_NONE : errno_to_cerror (errno);
#endif
}

void
c_fs_path_destroy (CPath* self)
{
  assert (self && self->buf);

  free (self->buf);
  *self = (CPath){ 0 };
}

cerror_t
c_fs_dir_create (CPath const* dir_path)
{
  assert (dir_path && dir_path->buf);

  return c_fs_dir_create_r (dir_path->buf, dir_path->len);
}

cerror_t
c_fs_dir_create_r (char const dir_path[], size_t path_len)
{
  assert (dir_path);
  assert (path_len > 0);
  assert (dir_path[path_len] == '\0');

#if defined(_WIN32)
  SetLastError (0);
  BOOL dir_created = CreateDirectoryA (dir_path, NULL);
  return dir_created ? CERROR_NONE : (cerror_t){ GetLastError (), "" };
#else
  errno = 0;
  int dir_status = mkdir (dir_path, ~umask (0));
  return dir_status == 0 ? CERROR_NONE : errno_to_cerror (errno);
#endif
}

cerror_t
c_fs_dir_exists (CPath const* dir_path,
                 bool* out_exists)
{
  assert (dir_path && dir_path->buf);

  return c_fs_dir_exists_r (dir_path->buf, dir_path->len, out_exists);
}

cerror_t
c_fs_dir_exists_r (char const dir_path[], size_t path_len, bool* out_exists)
{
  assert (dir_path);
  assert (path_len > 0);
  assert (dir_path[path_len] == '\0');

  if (out_exists)
    {
#if defined(_WIN32)
      SetLastError (0);
      size_t path_attributes = GetFileAttributesA (dir_path);
      if (path_attributes == INVALID_FILE_ATTRIBUTES)
        {
          *out_exists = false;
          return (cerror_t){ GetLastError (), "" };
        }

      else if ((path_attributes & FILE_ATTRIBUTE_DIRECTORY) ==
               FILE_ATTRIBUTE_DIRECTORY)
        {
          *out_exists = true;
          return CERROR_NONE;
        }
      else
        {
          *out_exists = false;
          return (cerror_t){ ERROR_DIRECTORY, "" };
        }
#else
      struct stat sb = { 0 };
      int path_attributes = stat (dir_path, &sb);
      if (path_attributes != 0)
        {
          *out_exists = false;
          return errno_to_cerror (errno);
        }

      if (S_ISDIR (sb.st_mode) == 0)
        {
          *out_exists = false;
          return (cerror_t){ ENOTDIR, "" };
        }

      *out_exists = true;
      return CERROR_NONE;
#endif
    }

  return CERROR_OUT_IS_NULL;
}

cerror_t
c_fs_dir_get_current (CPath* dir_path, size_t* out_size)
{
  assert (dir_path);

  return c_fs_dir_get_current_r (dir_path->buf, dir_path->len, out_size);
}

cerror_t
c_fs_dir_get_current_r (char dir_path[], size_t dir_path_len, size_t* out_size)
{
  assert (dir_path && dir_path_len > 0);

#ifdef _WIN32
  SetLastError (0);
  DWORD path_len = GetCurrentDirectoryA ((DWORD) dir_path_len, dir_path);
  if (out_size)
    *out_size = path_len;
  return path_len > 0 ? CERROR_NONE : (cerror_t){ GetLastError (), "" };
#else
  errno = 0;
  char* state = getcwd (dir_path, dir_path_len);
  if (out_size)
    *out_size = strlen (dir_path);
  return state ? CERROR_NONE : CERROR_SMALL_BUFFER;
#endif
}

cerror_t
c_fs_dir_change_current (CPath const* dir_path)
{
  assert (dir_path);

  return c_fs_dir_change_current_r (dir_path->buf, dir_path->len);
}

cerror_t
c_fs_dir_change_current_r (char const dir_path[], size_t dir_path_len)
{
  assert (dir_path && dir_path_len > 0);
  assert (dir_path[dir_path_len] == '\0');
  (void) dir_path_len;

#ifdef _WIN32
  SetLastError (0);
  BOOL status = SetCurrentDirectory (dir_path);
  return status ? CERROR_NONE : (cerror_t){ GetLastError (), "" };
#else
  errno = 0;
  int status = chdir (dir_path);
  return status == 0 ? CERROR_NONE : errno_to_cerror (errno);
#endif
}

cerror_t
c_fs_dir_is_empty (CPath const* dir_path, bool* out_is_empty)
{
  assert (dir_path);

  return c_fs_dir_is_empty_r (dir_path->buf, dir_path->len, out_is_empty);
}

cerror_t
c_fs_dir_is_empty_r (char const dir_path[], size_t dir_path_len, bool* out_is_empty)
{
  assert (dir_path && dir_path_len > 0);
  assert (dir_path[dir_path_len] == '\0');

  CPath path;
  cerror_t err = c_fs_path_create (dir_path, dir_path_len, MAX_PATH_LEN, &path);
  if (err.code != 0)
    {
      return err;
    }

  err = c_fs_foreach (
      &path,
      internal_c_fs_dir_is_empty_handler,
      out_is_empty);

  c_fs_path_destroy (&path);

  return err;
}

cerror_t
c_fs_exists (CPath const* path, bool* out_exists)
{
  assert (path);

  return c_fs_exists_r (path->buf, path->len, out_exists);
}

cerror_t
c_fs_exists_r (char const path[],
               size_t path_len,
               bool* out_exists)
{
  assert (path);
  assert (path_len > 0);
  assert (path[path_len] == '\0');

  if (out_exists)
    {
#if defined(_WIN32)
      SetLastError (0);
      DWORD stat_status = GetFileAttributesA (path);
      if (stat_status == INVALID_FILE_ATTRIBUTES)
        {
          DWORD last_error = GetLastError ();
          if (last_error == ERROR_FILE_NOT_FOUND ||
              last_error == ERROR_PATH_NOT_FOUND)
            {
              *out_exists = false;
              return CERROR_NONE;
            }
          else
            {
              *out_exists = false;
              return (cerror_t){ last_error, "" };
            }
        }

      *out_exists = true;
      return CERROR_NONE;
#else
      struct stat path_stats = { 0 };
      int stat_status = stat (path, &path_stats);
      if (stat_status == 0)
        {
          *out_exists = true;
          return CERROR_NONE;
        }
      else if (errno == ENOENT)
        {
          /// WARN: this could also means dangling pointer
          *out_exists = false;
          return CERROR_NONE;
        }
      else
        {
          *out_exists = false;
          return errno_to_cerror (errno);
        }
#endif
    }

  return CERROR_OUT_IS_NULL;
}

cerror_t
c_fs_delete (CPath const* path)
{
  assert (path);

  return c_fs_delete_r (path->buf, path->len);
}

cerror_t
c_fs_delete_r (char const path[], size_t path_len)
{
  assert (path);
  assert (path_len > 0);
  assert (path[path_len] == '\0');

#if defined(_WIN32)
  DWORD dwAttrib = GetFileAttributesA (path);

  if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
    {
      BOOL remove_dir_status = RemoveDirectoryA (path);
      if (!remove_dir_status)
        {
          return (cerror_t){ remove_dir_status, "" };
        }

      return CERROR_NONE;
    }
#endif

  errno = 0;
  int remove_status = remove (path);
  if (remove_status != 0)
    {
      return errno_to_cerror (errno);
    }

  return CERROR_NONE;
}

cerror_t
c_fs_delete_recursively (CPath* path)
{
  assert (path);

  return c_fs_delete_recursively_r (path->buf, path->len, path->capacity);
}

cerror_t
c_fs_delete_recursively_r (char path[], size_t path_len, size_t path_capacity)
{
  assert (path && path_len > 0 && path_capacity > path_len);
  assert (path[path_len] == '\0');

  if (path_capacity > path_len)
    {
      cerror_t foreach_err = c_fs_foreach (
          &(CPath){ .buf = path, .len = path_len, .capacity = path_capacity },
          internal_c_fs_delete_recursively_handler,
          NULL);

      if (foreach_err.code != CERROR_NONE.code)
        {
          return foreach_err;
        }

      // delete the directory itself after deleting all his children
      cerror_t delete_err = c_fs_delete_r (path, path_len);

      return delete_err;
    }

  return CERROR_SMALL_PATH_CAPACITY;
}

cerror_t
c_fs_foreach (CPath* path,
              cerror_t handler (CPath path[], void* extra_data),
              void* extra_data)
{
  assert (path);
  assert (path->buf && path->len > 0);
  assert (path->buf[path->len] == '\0');
  assert (handler);

  bool dir_exists;
  cerror_t err = c_fs_dir_exists_r (path->buf, path->len, &dir_exists);
  if (!dir_exists)
    {
      return err;
    }

  size_t orig_path_len = path->len;

#if defined(_WIN32)
  path->buf[path->len] = '/';
  path->buf[path->len + 1] = '*';
  path->buf[path->len + 2] = '\0';
  path->len += 2;

  SetLastError (0);
  WIN32_FIND_DATAA cur_file;
  HANDLE find_handler = FindFirstFileA (path->buf, &cur_file);
  do
    {
      if (find_handler == INVALID_HANDLE_VALUE)
        {
          path->buf[path->len - 2] = '\0';
          return (cerror_t){ GetLastError (), "" };
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
          memcpy (path->buf + path->len - 1,
                  cur_file.cFileName,
                  filename_len);
          path->buf[path->len - 1 + filename_len] = '\0';

          size_t old_len = path->len;
          path->len = path->len - 1 + filename_len;
          cerror_t err = handler (path, extra_data);
          path->len = old_len;
          if (err.code != CERROR_NONE.code)
            {
              break;
            }
        }
    }
  while (FindNextFileA (find_handler, &cur_file));

  SetLastError (0);
  if (!FindClose (find_handler))
    {
      path->buf[orig_path_len] = '\0';
      return (cerror_t){ GetLastError (), "" };
    }
#else

  DIR* cur_dir = opendir (path->buf);
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
              path->buf[path->len] = '/';
              assert (memcpy (path->buf + path->len + 1,
                              cur_dir_properties->d_name,
                              filename_len));
              path->buf[path->len + 1 + filename_len] = '\0';

              size_t old_len = path->len;
              path->len = path->len + 1 + filename_len;
              cerror_t handler_err = handler (path, extra_data);
              path->len = old_len;
              if (handler_err.code != CERROR_NONE.code)
                {
                  break;
                }
            }

          errno = 0;
        }

      if (closedir (cur_dir) != 0)
        {
          path->buf[orig_path_len] = '\0';
          return errno_to_cerror (errno);
        }
    }
#endif

  path->buf[orig_path_len] = '\0';
  return CERROR_NONE;
}

// ------------------------- internal ------------------------- //
cerror_t
internal_c_fs_delete_recursively_handler (CPath* path,
                                          void* extra_data)
{
  (void) extra_data;

  bool is_dir;
  c_fs_dir_exists (path, &is_dir);

  if (is_dir)
    {
      cerror_t err = c_fs_delete_recursively (path);
      if (err.code != CERROR_NONE.code)
        {
          return err;
        }
    }
  else
    {
      cerror_t delete_err = c_fs_delete (path);
      if (delete_err.code != CERROR_NONE.code)
        {
          return delete_err;
        }
    }

  return CERROR_NONE;
}

cerror_t
internal_c_fs_dir_is_empty_handler (CPath* path,
                                    void* extra_data)
{
  (void) path;

  *(bool*) extra_data = false; // This dir_path is not empty

  return CERROR_NONE;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

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

  cerror_t err = CERROR_NONE;

  CPath path_buf;
  cerror_t path_buf_err = c_fs_path_create_empty (MAX_PATH_LEN, &path_buf);
  assert (path_buf_err.code == 0);

  err = c_fs_dir_create_r (STR (test_playground));
  assert (err.code == 0);

  // test: c_fs_file_write
  {
    CFile f;
    size_t file_size = 0;

    err = c_fs_file_open_r (STR (test_playground "/file_ara.txt"), "w", &f);
    assert (err.code == 0);
    err = c_fs_file_write (&f, STR ("بسم الله الرحمن الرحيم\n"), &file_size);
    assert (file_size > 0);
    assert (err.code == 0);
    err = c_fs_file_close (&f);
    assert (err.code == 0);

    err = c_fs_file_open_r (STR (test_playground "/file_eng.txt"), "w", &f);
    assert (err.code == 0);
    err = c_fs_file_write (&f, STR ("May peace be upon you\n"), &file_size);
    assert (file_size > 0);
    assert (err.code == 0);
    err = c_fs_file_close (&f);
    assert (err.code == 0);
  }

  // test: c_fs_file_read
  {
    CFile f;
    size_t read_size = 0;

    err = c_fs_file_open_r (STR (test_playground "/file_eng.txt"), "r", &f);
    assert (err.code == 0);

    err = c_fs_file_read (&f, buf, buf_len, &read_size);
    assert (read_size > 0);
    assert (err.code == 0);
    buf[read_size] = '\0';
    assert (strncmp (buf, "May peace be upon you\n", read_size) == 0);

    err = c_fs_file_close (&f);
    assert (err.code == 0);
    err = c_fs_delete_r (STR (test_playground "/file_eng.txt"));
    assert (err.code == 0);
  }

  // test: c_fs_file_read - utf8
  {
    CFile f;
    size_t read_size = 0;

    err = c_fs_file_open_r (STR (test_playground "/file_ara.txt"), "r", &f);
    assert (err.code == 0);

    err = c_fs_file_read (&f, buf, buf_len, &read_size);
    assert (read_size > 0);
    assert (err.code == 0);
    buf[read_size] = '\0';
    assert (strncmp (buf, "بسم الله الرحمن الرحيم\n", read_size) == 0);

    err = c_fs_file_close (&f);
    assert (err.code == 0);
    err = c_fs_delete_r (STR (test_playground "/file_ara.txt"));
    assert (err.code == 0);
  }

  // test: c_fs_delete_recursively
  {
    err = c_fs_dir_create_r (STR (test_playground "/folder"));
    assert (err.code == 0);
    CFile file1;
    err = c_fs_file_open_r (STR (test_playground "/folder/1.txt"), "w", &file1);
    assert (err.code == 0);
    err = c_fs_file_close (&file1);
    assert (err.code == 0);

    err = c_fs_dir_create_r (STR (test_playground "/folder/folder2"));
    assert (err.code == 0);
    CFile file2;
    err = c_fs_file_open_r (
        STR (test_playground "/folder/folder2/.2.txt"), "w", &file2);
    assert (err.code == 0);
    err = c_fs_file_close (&file2);
    assert (err.code == 0);

    memcpy (path_buf.buf, STR (test_playground "/folder") + 1);
    path_buf.len = sizeof (test_playground "/folder") - 1;
    err = c_fs_delete_recursively (&path_buf);
    assert (err.code == 0);
  }

  // test: c_fs_dir_is_empty
  {
    err = c_fs_dir_create_r (STR (test_playground "/folder"));
    assert (err.code == 0);

    bool is_empty = false;
    err = c_fs_dir_is_empty (&path_buf, &is_empty);
    assert (!is_empty);
    assert (err.code == 0);

    err = c_fs_delete_r (STR (test_playground "/folder"));
    assert (err.code == 0);
  }

  // test: c_fs_foreach
  {
    // this will create a file
    CFile file1;
    err = c_fs_file_open_r (STR (test_playground "/1.txt"), "w", &file1);
    assert (err.code == 0);
    err = c_fs_file_close (&file1);
    assert (err.code == 0);

    bool file_found = false;
    cerror_t c_fs_handler (CPath * path, void* extra_data);
    assert (err.code == 0);

    err = c_fs_path_replace (&path_buf, STR (test_playground), true);
    assert (err.code == 0);
    err = c_fs_foreach (&path_buf, c_fs_handler, &file_found);
    assert (err.code == 0);
    assert (file_found);

    err = c_fs_delete_r (STR (test_playground "/1.txt"));
    assert (err.code == 0);
  }

  // test: c_fs_delete
  {
    err = c_fs_dir_create_r (STR (test_playground "/folder2"));
    assert (err.code == 0);

    bool is_exists = false;
    err = c_fs_exists_r (STR (test_playground "/folder2"), &is_exists);
    assert (is_exists);
    assert (err.code == 0);
    err = c_fs_delete_r (STR (test_playground "/folder2"));
    assert (err.code == 0);
  }

  // test: negative results
  {
    CFile file;
    bool is_exists;

    err = c_fs_file_open_r (STR ("/ymp/file1"), "r", &file);
    assert (err.code != 0);

    err = c_fs_dir_create_r (STR ("/ymp/file1"));
    assert (err.code != 0);

    err = c_fs_dir_exists_r (STR ("/ymp/file1"), &is_exists);
    assert (err.code != 0);
    assert (!is_exists);

    err = c_fs_exists_r (STR ("/ymp/file1"), &is_exists);
    assert (err.code == 0);
    assert (!is_exists);
  }

  // test: absolute path
  {
    err = c_fs_path_replace (&path_buf, STR ("./test_playground"), true);
    assert (err.code == 0);
    err = c_fs_path_to_absolute (&path_buf, true);
    assert (err.code == 0);
    assert (path_buf.buf);
  }

  // test: change current dir
  {
    size_t path_len = 0;
    char path[INT16_MAX];
    err = c_fs_dir_get_current_r (path, INT16_MAX, &path_len);
    assert (err.code == 0);

    err = c_fs_path_replace (&path_buf, path, path_len, true);
    assert (err.code == 0);

    err = c_fs_path_append (&path_buf, STR (".."), true);
    assert (err.code == 0);

    err = c_fs_dir_change_current_r (path_buf.buf, path_buf.len);
    assert (err.code == 0);

    assert (strcmp (path, path_buf.buf) != 0);

    err = c_fs_dir_change_current_r (path, path_len);
    assert (err.code == 0);
  }

  err = c_fs_delete_r (STR (test_playground));
  assert (err.code == 0);

  c_fs_path_destroy (&path_buf);
}

cerror_t
c_fs_handler (CPath* path, void* extra_data)
{
  if (strstr (path->buf, "1.txt"))
    {
      *(bool*) extra_data = true;
    }

  return (cerror_t){ 0 };
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
 * deal in the Software without ion, including without limitation the
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
