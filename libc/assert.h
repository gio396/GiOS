#ifndef __ASSERT_H__
#define __ASSERT_H__

#include <common.h>

#define assert_break *(u32*)(0) = 0

#define assert1(cond) !(cond) ? assert_break:(void)(0)

#define assertS(cond, state, format) \
        cond ? printk(state, format), assert_break : (void)(0)

#define assertK(cond, state, format, ...) \
        cond ? printk(state, format, __VA_ARGS__), assert_break : (void)(0)

#define static_assert(cond, msg) _Static_assert(cond, msg)


#endif