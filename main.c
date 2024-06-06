/***** implementation *****/
#define CSTDLIB_ARRAY_IMPLEMENTATION
#include "array.h"

#define CSTDLIB_FS_IMPLEMENTATION
#include "fs.h"

#define CSTDLIB_LOG_IMPLEMENTATION
#include "log.h"

#define CSTDLIB_MAP_IMPLEMENTATION
#include "map.h"

#define CSTDLIB_STR_IMPLEMENTATION
#include "str.h"

/* #define CSTDLIB_GUI_IMPLEMENTATION
#include "gui.h" */

/***** Tests *****/
#define CSTDLIB_ARRAY_UNIT_TESTS
#include "array.h"

#define CSTDLIB_FS_UNIT_TESTS
#include "fs.h"

#define CSTDLIB_LOG_UNIT_TESTS
#include "log.h"

#define CSTDLIB_MAP_UNIT_TESTS
#include "map.h"

#define CSTDLIB_STR_UNIT_TESTS
#include "str.h"

// #define CSTDLIB_GUI_UNIT_TESTS
// #include "gui.h"

int
main (void)
{
  c_array_unit_tests ();
  c_fs_unit_tests ();
  c_log_unit_tests ();
  // c_map_unit_tests ();
  c_str_unit_tests ();
  // gui_unit_tests ();

  return EXIT_SUCCESS;
}

#ifndef NDEBUG
/* @brief Address sanitizer flags
 * @return
 */
const char *
__lsan_default_options (void)
{
  return "suppressions=" CURRENT_DIR "/.leak-ignore"
         ":print_suppressions=0";
}
#endif /* NDEBUG */
