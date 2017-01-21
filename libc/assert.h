#ifndef __ASSERT_H__
#define __ASSERT_H__

#include <common.h>

#define assert_break *(uint32*)(0) = 0

#define assert1(cond) cond != TRUE ? assert_break:(void)(0)


#define assertS(cond, state, format) \
        cond != TRUE ? printk(state, format), assert_break : (void)(0)

#define assertK(cond, state, format, ...) \
        cond != TRUE ? printk(state, format, __VA_ARGS__), assert_break : (void)(0)


#endif