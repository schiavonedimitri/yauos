#include <stdbool.h>
#include <stddef.h>
#include <arch/bootconsole/bootconsole.h>

extern bootconsole_t bootconsole_vga_text_mode;
bootconsole_t *bootconsole;
static bool bootconsole_enabled = 1;

void bootconsole_disable() {
	bootconsole_enabled = 0;
}

bool bootconsole_is_enabled() {
	return bootconsole_enabled;
}

void bootconsole_init(bootconsole_type_t console_type) {
	switch (console_type) {
		case BOOTCONSOLE_VGA:
			bootconsole = &bootconsole_vga_text_mode;
			break;
		case BOOTCONSOLE_SERIAL:
			//TODO: add serial bootconsole implementation
			break;
			//TODO: support other bootconsole types in the future, like framebuffer etc...
		default:
			//default to the vga implementation.
			bootconsole = &bootconsole_vga_text_mode;
	};
	bootconsole->console_init();
}

void bootconsole_put_char(char c) {
	bootconsole->console_put_char(c);
}

void bootconsole_put_string(const char* s, size_t size) {
	bootconsole->console_put_string(s, size);
}
