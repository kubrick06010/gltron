#ifndef _NEBU_ASSERT
#define _NEBU_ASSERT

#include <unistd.h>

#define nebu_assert(x) nebu_assert_int((ssize_t)x)

enum {
	NEBU_ASSERT_LIBC = 1,
	NEBU_ASSERT_PRINT_STDERR = 2,
	NEBU_ASSERT_DEFAULT_CONFIG = 1
};

#ifdef __cplusplus
extern "C" {
#endif

extern void nebu_assert_config(int flags);
extern void nebu_assert_int(ssize_t value);

#ifdef __cplusplus
}
#endif

#endif
