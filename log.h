/* How To : To use this module, do this in *ONE* C file:
 *              #define CSTDLIB_LOG_IMPLEMENTATION
 *              #include "log.h"
 * Tests  : To use run test, do this in *ONE* C file:
 *              #define CSTDLIB_LOG_UNIT_TESTS
 *              #include "log.h"
 * License: MIT (send end of the file for details)
 */

#ifndef CSTDLIB_LOG_H
#define CSTDLIB_LOG_H

    #include <stdarg.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <string.h>
    #include <stdio.h>

    typedef enum { LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL } LogType;

    void
    log_impl(LogType type, const char* file, uint32_t line, FILE* out, bool use_color, const char* format, ...);

    #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

    // log to stdout/stderr
    #define log_info(fmt, ...)  (log_impl(LOG_INFO,  __FILENAME__, __LINE__, stdout, false, (fmt), __VA_ARGS__))
    #define log_warn(fmt, ...)  (log_impl(LOG_WARN,  __FILENAME__, __LINE__, stderr, false, (fmt), __VA_ARGS__))
    #define log_error(fmt, ...) (log_impl(LOG_ERROR, __FILENAME__, __LINE__, stderr, false, (fmt), __VA_ARGS__))
    #define log_fatal(fmt, ...) (log_impl(LOG_FATAL, __FILENAME__, __LINE__, stderr, false, (fmt), __VA_ARGS__))

    // colorful logging to stdout/stderr
    #define clog_info(fmt, ...)  (log_impl(LOG_INFO,  __FILENAME__, __LINE__, stdout, true, (fmt), __VA_ARGS__))
    #define clog_warn(fmt, ...)  (log_impl(LOG_WARN,  __FILENAME__, __LINE__, stderr, true, (fmt), __VA_ARGS__))
    #define clog_error(fmt, ...) (log_impl(LOG_ERROR, __FILENAME__, __LINE__, stderr, true, (fmt), __VA_ARGS__))
    #define clog_fatal(fmt, ...) (log_impl(LOG_FATAL, __FILENAME__, __LINE__, stderr, true, (fmt), __VA_ARGS__))

    // log to file
    #define flog_info(out, fmt, ...)  (log_impl(LOG_INFO,  __FILENAME__, __LINE__, (out), false, (fmt), __VA_ARGS__))
    #define flog_warn(out, fmt, ...)  (log_impl(LOG_WARN,  __FILENAME__, __LINE__, (out), false, (fmt), __VA_ARGS__))
    #define flog_error(out, fmt, ...) (log_impl(LOG_ERROR, __FILENAME__, __LINE__, (out), false, (fmt), __VA_ARGS__))
    #define flog_fatal(out, fmt, ...) (log_impl(LOG_FATAL, __FILENAME__, __LINE__, (out), false, (fmt), __VA_ARGS__))

#endif // CSTDLIB_LOG_H


#ifdef CSTDLIB_LOG_IMPLEMENTATION
    #ifdef NDEBUG
    #define assert(expr) ((void)(expr))
    #else
    #include <assert.h>
    #endif

    static void log_internal_get_cur_time(char* time_buf, size_t time_buf_size);

    void
    log_impl(LogType type, const char* file, uint32_t line, FILE* out, bool use_color, const char* format, ...) {
        assert(type < 4);
        assert(out);

        const char* log_types[] = { "INFO", "WARN", "ERROR", "FATAL" };
        const char* colors[]    = { "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m" };
        const char* color_reset = "\x1b[0m";

        enum { time_buf_size = 9 };
        char time_buf[time_buf_size];
        log_internal_get_cur_time(time_buf, time_buf_size);

        if(use_color)
        {
            assert(fprintf(out, "%s %s%-5s%s %s:%d: ", time_buf, colors[type], log_types[type], color_reset, file, line));
        }
        else
        {
            assert(fprintf(out, "%s %-5s %s:%d: ", time_buf, log_types[type], file, line));
        }

        va_list args;
        va_start(args, format);
        assert(vfprintf(out, format, args));
        assert(fprintf(out, "\n"));
        assert(fflush(out) == 0);
        va_end(args);
    }

    #include <time.h>

    void
    log_internal_get_cur_time(char* time_buf, size_t time_buf_size)
    {
        time_t rawtime;
        struct tm timeinfo;

        time(&rawtime);

    #if defined(__unix__)
        localtime_r(&rawtime, &timeinfo);
    #elif defined(_WIN32)
        localtime_s(&timeinfo, &rawtime);
    #else
        timeinfo = localtime(&rawtime);
        assert(timeinfo);
    #endif

        assert(strftime (time_buf, time_buf_size, "%H:%M:%S", &timeinfo));
        time_buf[time_buf_size - 1] = '\0';
    }
#endif // CSTDLIB_LOG_IMPLEMENTATION


#ifdef CSTDLIB_LOG_UNIT_TESTS
    #ifdef NDEBUG
    #define NDEBUG_
    #undef NDEBUG
    #endif

    #include <assert.h>

    void log_unit_tests(void)
    {
        clog_info("%s", "This is an info");
        clog_warn("%s", "This is a warning");
        clog_error("%s", "This is an error");
        clog_fatal("%s", "This is fatal");
    }

    #ifdef NDEBUG_
    #define NDEBUG
    #undef NDEBUG_
    #endif
#endif // CSTDLIB_LOG_UNIT_TESTS

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
