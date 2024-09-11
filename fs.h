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

/// @brief
/// @param path
/// @param path_len
/// @param mode
/// @param out_cfile
/// @return
cerror_t c_fs_file_open (char const path[],
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

/// @briefthis will append `path` to `base_path` and adding '/' in between
/// @param base_path
/// @param base_path_len
/// @param base_path_capacity
/// @param path
/// @param path_len
/// @param out_new_path_len
/// @return error (any value but zero is treated as an error)
cerror_t c_fs_path_append (char base_path[],
                           size_t base_path_len,
                           size_t base_path_capacity,
                           char const path[],
                           size_t path_len,
                           size_t* out_new_path_len);

/// @brief
/// @param path
/// @param path_len
/// @param out_absolute_path
/// @param out_absolute_path_capacity
/// @param out_absolute_path_len
/// @return
cerror_t c_fs_path_to_absolute (char const path[],
                                size_t path_len,
                                char out_absolute_path[],
                                size_t out_absolute_path_capacity,
                                size_t* out_absolute_path_len);

/// @brief create a directory with raw path
/// @param dir_path
/// @param path_len
/// @param err return error (any value but zero is treated as an error)
cerror_t c_fs_dir_create (char const dir_path[], size_t path_len);

/// @brief check if path is a directory
/// @param path
/// @param path_len
/// @return
cerror_t c_fs_dir_exists (char const path[],
                          size_t path_len,
                          bool* out_exists);

/// @brief
/// @param path
/// @param path_len
/// @return
cerror_t c_fs_dir_change_current (char const path[], size_t path_len);

/// @brief
/// @param path
/// @param path_len
/// @param out_is_empty
/// @return
cerror_t c_fs_dir_is_empty (char const path[],
                            size_t path_len,
                            bool* out_is_empty);

/// @brief check if file/directory exists
/// @param path
/// @param path_len
/// @param err return error (any value but zero is treated as an error)
/// @return
cerror_t c_fs_exists (char const path[], size_t path_len, bool* out_exists);

/// @brief delete a file/directory (empty directory only) with raw path
/// @param path
/// @param path_len
cerror_t c_fs_delete (char const path[], size_t path_len);

/// @brief
/// @param path
/// @param path_len
/// @return
cerror_t c_fs_delete_recursively (char path[], size_t path_len);

/// @brief
/// @param path_buf
/// @param path_buf_len
/// @param path_buf_capacity
/// @param handler
/// @param extra_data
/// @return
cerror_t
c_fs_foreach (char path_buf[],
              size_t path_buf_len,
              size_t path_buf_capacity,
              cerror_t handler (char* path, size_t path_len, void* extra_data),
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

static cerror_t internal_c_fs_delete_recursively_handler (char path[],
                                                          size_t path_len,
                                                          void* extra_data);
static cerror_t internal_c_fs_dir_is_empty_handler (char path[],
                                                    size_t path_len,
                                                    void* extra_data);

static cerror_t internal_c_fs_delete_recursively (char path_buf[],
                                                  size_t path_buf_len);

static inline cerror_t
errno_to_cerror (int errno_)
{
  return (cerror_t){ errno_, strerror (errno_) };
}

cerror_t
c_fs_file_open (char const* path,
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
c_fs_path_append (char base_path[],
                  size_t base_path_len,
                  size_t base_path_capacity,
                  char const path[],
                  size_t path_len,
                  size_t* out_new_path_len)
{
  assert (base_path && base_path_len > 0);
  assert (path && path_len > 0);

  if (base_path_capacity < (base_path_len + path_len + 2))
    {
      return CERROR_PATH_BUFFER_FULL;
    }

  base_path[base_path_len++] = PATH_SEP;
  base_path_len += path_len;
  strncpy (base_path + base_path_len, path, base_path_len);

  if (out_new_path_len)
    *out_new_path_len = base_path_len;

  return CERROR_NONE;
}

cerror_t
c_fs_path_to_absolute (char const path[],
                       size_t path_len,
                       char out_absolute_path[],
                       size_t out_absolute_path_capacity,
                       size_t* out_absolute_path_len)
{
  assert (path && path_len > 0);
  assert (out_absolute_path && out_absolute_path_capacity > 0);

#ifdef _WIN32
  SetLastError (0);
  DWORD abs_path_len = GetLongPathNameA (path, out_absolute_path, (DWORD) out_absolute_path_capacity);
  if (out_absolute_path_len)
    *out_absolute_path_len = abs_path_len;
  return abs_path_len > 0 ? CERROR_NONE : (cerror_t){ GetLastError (), "" };
#else
  errno = 0;
  char* path_status = realpath (path, out_absolute_path);
  size_t abs_path_len = path_status ? strlen (out_absolute_path) : 0;
  if (out_absolute_path_len)
    *out_absolute_path_len = abs_path_len;
  return path_status ? CERROR_NONE : errno_to_cerror (errno);
#endif
}

cerror_t
c_fs_dir_create (char const dir_path[], size_t path_len)
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
c_fs_dir_exists (char const dir_path[], size_t path_len, bool* out_exists)
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
c_fs_dir_get_current (char dir_path[], size_t dir_path_len, size_t* out_size)
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
c_fs_dir_change_current (char const dir_path[], size_t dir_path_len)
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
c_fs_dir_is_empty (char const path[],
                   size_t path_len,
                   bool* out_is_empty)
{
  assert (path && path_len > 0);
  assert (path[path_len] == '\0');

  char* path_buf = malloc (MAX_PATH_LEN);
  if (!path_buf)
    {
      return CERROR_MEM_ALLOCATION;
    }
  strncpy (path_buf, path, MAX_PATH_LEN);

  /// FIXME: need optimization (fail on first path that is not empty)
  cerror_t err = c_fs_foreach (
      path_buf, path_len, MAX_PATH_LEN,
      internal_c_fs_dir_is_empty_handler,
      out_is_empty);

  free (path_buf);

  return err;
}

cerror_t
c_fs_exists (char const path[],
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
c_fs_delete (char const path[], size_t path_len)
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
c_fs_delete_recursively (char path[], size_t path_len)
{
  assert (path && path_len > 0);
  assert (path[path_len] == '\0');

  char* path_buf = malloc (MAX_PATH_LEN);
  if (!path_buf)
    {
      return CERROR_MEM_ALLOCATION;
    }
  strncpy (path_buf, path, MAX_PATH_LEN);

  cerror_t err = internal_c_fs_delete_recursively (path_buf, path_len);

  free (path_buf);
  return err;
}

cerror_t
c_fs_foreach (char path_buf[],
              size_t path_buf_len,
              size_t path_buf_capacity,
              cerror_t handler (char* path, size_t path_len, void* extra_data),
              void* extra_data)
{
  assert (path_buf && path_buf_len > 0 && path_buf_capacity > 0);
  assert (path_buf[path_buf_len] == '\0');
  assert (handler);

  bool dir_exists;
  cerror_t err = c_fs_dir_exists (path_buf, path_buf_len, &dir_exists);
  if (!dir_exists)
    {
      return err;
    }

  size_t orig_path_len = path_buf_len;

#if defined(_WIN32)
  path_buf[path_buf_len] = '/';
  path_buf[path_buf_len + 1] = '*';
  path_buf[path_buf_len + 2] = '\0';
  path_buf_len += 2;

  SetLastError (0);
  WIN32_FIND_DATAA cur_file;
  HANDLE find_handler = FindFirstFileA (path_buf, &cur_file);
  do
    {
      if (find_handler == INVALID_HANDLE_VALUE)
        {
          path_buf[path_buf_len - 2] = '\0';
          err = (cerror_t){ GetLastError (), "" };
          goto Error;
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
          strcpy (path_buf + path_buf_len - 1,
                  cur_file.cFileName,
                  filename_len);

          size_t old_len = path_buf_len;
          path_buf_len = path_buf_len - 1 + filename_len;
          cerror_t err = handler (path_buf, extra_data);
          path_buf_len = old_len;
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
      path_buf[orig_path_buf_len] = '\0';
      err = (cerror_t){ GetLastError (), "" };
      goto Error;
    }
#else

  DIR* cur_dir = opendir (path_buf);
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
              path_buf[path_buf_len] = '/';
              strncpy (path_buf + path_buf_len + 1,
                       cur_dir_properties->d_name,
                       filename_len);

              size_t old_len = path_buf_len;
              path_buf_len = path_buf_len + 1 + filename_len;
              path_buf[path_buf_len] = '\0';
              cerror_t handler_err = handler (path_buf, path_buf_len, extra_data);
              path_buf_len = old_len;
              if (handler_err.code != CERROR_NONE.code)
                {
                  break;
                }
            }

          errno = 0;
        }

      if (closedir (cur_dir) != 0)
        {
          path_buf[orig_path_len] = '\0';
          err = errno_to_cerror (errno);
          goto Error;
        }
    }
#endif

  path_buf[orig_path_len] = '\0';

Error:
  return err;
}

// ------------------------- internal ------------------------- //
cerror_t
internal_c_fs_delete_recursively_handler (char path[],
                                          size_t path_len,
                                          void* extra_data)
{
  (void) extra_data;

  bool is_dir;
  c_fs_dir_exists (path, path_len, &is_dir);

  if (is_dir)
    {
      cerror_t err = internal_c_fs_delete_recursively (path, path_len);
      if (err.code != CERROR_NONE.code)
        {
          return err;
        }
    }
  else
    {
      cerror_t delete_err = c_fs_delete (path, path_len);
      if (delete_err.code != CERROR_NONE.code)
        {
          return delete_err;
        }
    }

  return CERROR_NONE;
}

cerror_t
internal_c_fs_dir_is_empty_handler (char path[],
                                    size_t path_len,
                                    void* extra_data)
{
  (void) path;
  (void) path_len;

  *(bool*) extra_data = false; // This dir_path is not empty

  return CERROR_NONE;
}

cerror_t
internal_c_fs_delete_recursively (char path_buf[],
                                  size_t path_buf_len)
{
  cerror_t err = c_fs_foreach (path_buf, path_buf_len, MAX_PATH_LEN,
                               internal_c_fs_delete_recursively_handler,
                               NULL);

  if (err.code != CERROR_NONE.code)
    {
      return err;
    }

  // delete the directory itself after deleting all his children
  err = c_fs_delete (path_buf, path_buf_len);

  return err;
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

#define STR(str) str, (sizeof (str) - 1)
#define test_playground "test_playground"

  enum
  {
    buf_len = 100
  };
  char buf[buf_len];

  cerror_t err = CERROR_NONE;

  char* path_buf = malloc (MAX_PATH_LEN);
  assert (path_buf);
  size_t path_buf_len = 0;

  err = c_fs_dir_create (STR (test_playground));
  assert (err.code == 0);

  // test: c_fs_file_write
  {
    CFile f;
    size_t file_size = 0;

    err = c_fs_file_open (STR (test_playground "/file_ara.txt"), "w", &f);
    assert (err.code == 0);
    err = c_fs_file_write (&f, STR ("بسم الله الرحمن الرحيم\n"), &file_size);
    assert (file_size > 0);
    assert (err.code == 0);
    err = c_fs_file_close (&f);
    assert (err.code == 0);

    err = c_fs_file_open (STR (test_playground "/file_eng.txt"), "w", &f);
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

    err = c_fs_file_open (STR (test_playground "/file_eng.txt"), "r", &f);
    assert (err.code == 0);

    err = c_fs_file_read (&f, buf, buf_len, &read_size);
    assert (read_size > 0);
    assert (err.code == 0);
    buf[read_size] = '\0';
    assert (strncmp (buf, "May peace be upon you\n", read_size) == 0);

    err = c_fs_file_close (&f);
    assert (err.code == 0);
    err = c_fs_delete (STR (test_playground "/file_eng.txt"));
    assert (err.code == 0);
  }

  // test: c_fs_file_read - utf8
  {
    CFile f;
    size_t read_size = 0;

    err = c_fs_file_open (STR (test_playground "/file_ara.txt"), "r", &f);
    assert (err.code == 0);

    err = c_fs_file_read (&f, buf, buf_len, &read_size);
    assert (read_size > 0);
    assert (err.code == 0);
    buf[read_size] = '\0';
    assert (strncmp (buf, "بسم الله الرحمن الرحيم\n", read_size) == 0);

    err = c_fs_file_close (&f);
    assert (err.code == 0);
    err = c_fs_delete (STR (test_playground "/file_ara.txt"));
    assert (err.code == 0);
  }

  // test: c_fs_delete_recursively
  {
    err = c_fs_dir_create (STR (test_playground "/folder"));
    assert (err.code == 0);
    CFile file1;
    err = c_fs_file_open (STR (test_playground "/folder/1.txt"), "w", &file1);
    assert (err.code == 0);
    err = c_fs_file_close (&file1);
    assert (err.code == 0);

    err = c_fs_dir_create (STR (test_playground "/folder/folder2"));
    assert (err.code == 0);
    CFile file2;
    err = c_fs_file_open (
        STR (test_playground "/folder/folder2/.2.txt"), "w", &file2);
    assert (err.code == 0);
    err = c_fs_file_close (&file2);
    assert (err.code == 0);

    memcpy (path_buf, STR (test_playground "/folder") + 1);
    path_buf_len = sizeof (test_playground "/folder") - 1;
    err = c_fs_delete_recursively (path_buf, path_buf_len);
    assert (err.code == 0);
  }

  // test: c_fs_dir_is_empty
  {
    err = c_fs_dir_create (STR (test_playground "/folder"));
    assert (err.code == 0);

    bool is_empty = false;
    err = c_fs_dir_is_empty (path_buf, path_buf_len, &is_empty);
    assert (!is_empty);
    assert (err.code == 0);

    err = c_fs_delete (STR (test_playground "/folder"));
    assert (err.code == 0);
  }

  // test: c_fs_foreach
  {
    // this will create a file
    CFile file1;
    err = c_fs_file_open (STR (test_playground "/1.txt"), "w", &file1);
    assert (err.code == 0);
    err = c_fs_file_close (&file1);
    assert (err.code == 0);

    bool file_found = false;
    assert (err.code == 0);

    strcpy (path_buf, test_playground);
    path_buf_len = strlen (path_buf);
    // assert (err.code == 0);
    // assert (out_path_len > 0);

    cerror_t c_fs_handler (char path[], size_t path_len, void* extra_data);
    err = c_fs_foreach (path_buf,
                        path_buf_len,
                        MAX_PATH_LEN,
                        c_fs_handler,
                        &file_found);
    assert (err.code == 0);
    assert (file_found);

    err = c_fs_delete (STR (test_playground "/1.txt"));
    assert (err.code == 0);
  }

  // test: c_fs_delete
  {
    err = c_fs_dir_create (STR (test_playground "/folder2"));
    assert (err.code == 0);

    bool is_exists = false;
    err = c_fs_exists (STR (test_playground "/folder2"), &is_exists);
    assert (is_exists);
    assert (err.code == 0);
    err = c_fs_delete (STR (test_playground "/folder2"));
    assert (err.code == 0);
  }

  // test: negative results
  {
    CFile file;
    bool is_exists;

    err = c_fs_file_open (STR ("/ymp/file1"), "r", &file);
    assert (err.code != 0);

    err = c_fs_dir_create (STR ("/ymp/file1"));
    assert (err.code != 0);

    err = c_fs_dir_exists (STR ("/ymp/file1"), &is_exists);
    assert (err.code != 0);
    assert (!is_exists);

    err = c_fs_exists (STR ("/ymp/file1"), &is_exists);
    assert (err.code == 0);
    assert (!is_exists);
  }

  // test: absolute path
  {
    strcpy (path_buf, "./test_playground");
    path_buf_len = strlen (path_buf);

    size_t out_path_len = 0;
    err = c_fs_path_to_absolute (path_buf, path_buf_len, path_buf, MAX_PATH_LEN, &out_path_len);
    assert (err.code == 0);
    assert (path_buf);
    assert (out_path_len > 0);
  }

  // test: change current dir
  {
    char tmp_buf[MAX_PATH_LEN] = { 0 };
    size_t tmp_buf_len = 0;

    err = c_fs_dir_get_current (tmp_buf, MAX_PATH_LEN, &tmp_buf_len);
    assert (err.code == 0);

    err = c_fs_path_append (path_buf, path_buf_len, MAX_PATH_LEN, STR (".."), &path_buf_len);
    assert (err.code == 0);
    assert (path_buf_len > 0);

    err = c_fs_dir_change_current (tmp_buf, tmp_buf_len);
    assert (err.code == 0);
  }

  err = c_fs_delete (STR (test_playground));
  assert (err.code == 0);
}

cerror_t
c_fs_handler (char path[],
              size_t path_len,
              void* extra_data)
{
  (void) path_len;

  if (strstr (path, "1.txt"))
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
