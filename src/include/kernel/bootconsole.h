#ifndef BOOTCONSOLE_H
        #define BOOTCONSOLE_H

        #include <stdbool.h>
        #include <stddef.h>
        #include <arch/types.h>

        typedef enum bootconsole_type {
                BOOTCONSOLE_MEM = 1,
                BOOTCONSOLE_SERIAL = 2,
                BOOTCONSOLE_VGA_TEXT_MODE = 3,
        } bootconsole_type_t;

        typedef struct bootconsole {
                int (*console_init) (void);
                void (*console_put_char) (char);
                void (*console_put_string) (const char*, size_t);
        } bootconsole_t;

        void bootconsole_disable();
        bool bootconsole_is_enabled();
        int bootconsole_init(bootconsole_type_t);
        void bootconsole_put_char(char);
        void bootconsole_put_string(const char*, size_t);

#endif /** BOOTCONSOLE_H */
