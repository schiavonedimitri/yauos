#include <stdbool.h>
#include <stddef.h>
#include <kernel/bootconsole.h>

extern bootconsole_t bootconsole_mem;
extern bootconsole_t bootconsole_serial;
extern bootconsole_t bootconsole_vga_text_mode;

static bootconsole_t *bootconsole;
static bool bootconsole_enabled = 1;

void bootconsole_disable() {
	bootconsole_enabled = 0;
}

bool bootconsole_is_enabled() {
	return bootconsole_enabled;
}

int bootconsole_init(bootconsole_type_t console_type) {
	switch (console_type) {
		case BOOTCONSOLE_MEM:
			bootconsole = &bootconsole_mem;
			break;
		case BOOTCONSOLE_SERIAL:
		        bootconsole = &bootconsole_serial;
			break;
		case BOOTCONSOLE_VGA_TEXT_MODE:
			bootconsole = &bootconsole_vga_text_mode;
			break;
			
			//TODO: support other bootconsole types in the future, like framebuffer etc...
		
		default:
			
			//default to the serial implementation.
			
			bootconsole = &bootconsole_serial;
	};
	return bootconsole->console_init();
}

void bootconsole_put_char(char c) {
	bootconsole->console_put_char(c);
}

void bootconsole_put_string(const char* s, size_t size) {
	bootconsole->console_put_string(s, size);
}