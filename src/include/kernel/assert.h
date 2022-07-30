#ifndef ASSERT_H
        #define ASSERT_H

        #include <kernel/printk.h>

        #define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

        #ifdef DEBUG

                #define assert(expression) ((expression) ? (void) 0 : panic("Assertion failed: \"" #expression "\" file: %s function: %s line: %d\n", __FILENAME__, __func__, __LINE__))

        #else

                #define assert(expression)

        #endif

#endif /** ASSERT_H */