/* How To : To use this module, do this in *ONE* C file:
 *              #define CSTDLIB_LOG_IMPLEMENTATION
 *              #include "log.h"
 * Tests  : To use run test, do this in *ONE* C file:
 *              #define CSTDLIB_LOG_UNIT_TESTS
 *              #include "log.h"
 * License: MIT (go to the end of this file for details)
 */

#ifndef CSTDLIB_LOG_H
#define CSTDLIB_LOG_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef enum
{
  LOG_INFO,
  LOG_WARN,
  LOG_ERROR,
  LOG_FATAL
} CLogType;

void c_log_impl (CLogType type, char const* file, size_t line, FILE* out, bool use_color, char const* format, ...);

#ifdef _WIN32
#define __FILENAME__ \
  (strrchr (__FILE__, '\\') ? strrchr (__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ \
  (strrchr (__FILE__, '/') ? strrchr (__FILE__, '/') + 1 : __FILE__)
#endif

// log to stdout/stderr
#define c_log_info(fmt, ...)                                           \
  (c_log_impl (LOG_INFO, __FILENAME__, __LINE__, stdout, false, (fmt), \
               __VA_ARGS__))
#define c_log_warn(fmt, ...)                                           \
  (c_log_impl (LOG_WARN, __FILENAME__, __LINE__, stderr, false, (fmt), \
               __VA_ARGS__))
#define c_log_error(fmt, ...)                                           \
  (c_log_impl (LOG_ERROR, __FILENAME__, __LINE__, stderr, false, (fmt), \
               __VA_ARGS__))
#define c_log_fatal(fmt, ...)                                           \
  (c_log_impl (LOG_FATAL, __FILENAME__, __LINE__, stderr, false, (fmt), \
               __VA_ARGS__))

// colorful logging to stdout/stderr
#define c_clog_info(fmt, ...)                                         \
  (c_log_impl (LOG_INFO, __FILENAME__, __LINE__, stdout, true, (fmt), \
               __VA_ARGS__))
#define c_clog_warn(fmt, ...)                                         \
  (c_log_impl (LOG_WARN, __FILENAME__, __LINE__, stderr, true, (fmt), \
               __VA_ARGS__))
#define c_clog_error(fmt, ...)                                         \
  (c_log_impl (LOG_ERROR, __FILENAME__, __LINE__, stderr, true, (fmt), \
               __VA_ARGS__))
#define c_clog_fatal(fmt, ...)                                         \
  (c_log_impl (LOG_FATAL, __FILENAME__, __LINE__, stderr, true, (fmt), \
               __VA_ARGS__))

// log to file
#define c_flog_info(out, fmt, ...)                                    \
  (c_log_impl (LOG_INFO, __FILENAME__, __LINE__, (out), false, (fmt), \
               __VA_ARGS__))
#define c_flog_warn(out, fmt, ...)                                    \
  (c_log_impl (LOG_WARN, __FILENAME__, __LINE__, (out), false, (fmt), \
               __VA_ARGS__))
#define c_flog_error(out, fmt, ...)                                    \
  (c_log_impl (LOG_ERROR, __FILENAME__, __LINE__, (out), false, (fmt), \
               __VA_ARGS__))
#define c_flog_fatal(out, fmt, ...)                                    \
  (c_log_impl (LOG_FATAL, __FILENAME__, __LINE__, (out), false, (fmt), \
               __VA_ARGS__))

#endif // CSTDLIB_LOG_H

#ifdef CSTDLIB_LOG_IMPLEMENTATION
#include <assert.h>

static void internal_c_log_get_cur_time (char* time_buf, size_t time_buf_size);

void
c_log_impl (CLogType type, char const* file, size_t line, FILE* out, bool use_color, char const* format, ...)
{
  assert (type < 4);
  assert (out);

  char const* log_types[] = { "INFO", "WARN", "ERROR", "FATAL" };
  char const* colors[] = { "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m" };
  char const* color_reset = "\x1b[0m";

  enum
  {
    time_buf_size = 9
  };
  char time_buf[time_buf_size];
  internal_c_log_get_cur_time (time_buf, time_buf_size);

  if (use_color)
    {
      fprintf (out, "%s %s%-5s%s %s:%zu: ", time_buf, colors[type],
               log_types[type], color_reset, file, line);
    }
  else
    {
      fprintf (out, "%s %-5s %s:%zu: ", time_buf, log_types[type],
               file, line);
    }

  va_list args;
  va_start (args, format);
  vfprintf (out, format, args);
  fprintf (out, "\n");
  fflush (out);
  va_end (args);
}

#include <time.h>

void
internal_c_log_get_cur_time (char* time_buf, size_t time_buf_size)
{
  time_t rawtime;
  struct tm timeinfo;

  time (&rawtime);

#if defined(__unix__)
  localtime_r (&rawtime, &timeinfo);
#elif defined(_WIN32)
  localtime_s (&timeinfo, &rawtime);
#else
  timeinfo = localtime (&rawtime);
  assert (timeinfo);
#endif

  strftime (time_buf, time_buf_size, "%H:%M:%S", &timeinfo);
  time_buf[time_buf_size - 1] = '\0';
}

#undef CSTDLIB_LOG_IMPLEMENTATION
#endif // CSTDLIB_LOG_IMPLEMENTATION

#ifdef CSTDLIB_LOG_UNIT_TESTS
#ifdef NDEBUG
#define NDEBUG_
#undef NDEBUG
#endif

#include <assert.h>

void
c_log_unit_tests (void)
{
  c_clog_info ("%s", "This is an info");
  c_clog_warn ("%s", "This is a warning");
  c_clog_error ("%s", "This is an error");
  c_clog_fatal ("%s", "This is fatal");
}

#ifdef NDEBUG_
#define NDEBUG
#undef NDEBUG_
#endif

#undef CSTDLIB_LOG_UNIT_TESTS
#endif // CSTDLIB_LOG_UNIT_TESTS

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
