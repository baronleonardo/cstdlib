#define CSTDLIB_ARRAY_IMPLEMENTATION
#define CSTDLIB_ARRAY_UNIT_TESTS
#include "array.h"

#define CSTDLIB_IO_IMPLEMENTATION
#define CSTDLIB_IO_UNIT_TESTS
#include "io.h"

#define CSTDLIB_LOG_IMPLEMENTATION
#define CSTDLIB_LOG_UNIT_TESTS
#include "log.h"

#define CSTDLIB_MAP_IMPLEMENTATION
#define CSTDLIB_MAP_UNIT_TESTS
#include "map.h"

#define CSTDLIB_STR_IMPLEMENTATION
#define CSTDLIB_STR_UNIT_TESTS
#include "str.h"

int
main(void)
{
    array_unit_tests();
    io_unit_tests();
    log_unit_tests();
    map_unit_tests();
    str_unit_tests();
}
