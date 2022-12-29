
#ifndef DISPLAY_WRAPPER_H_
#define DISPLAY_WRAPPER_H_

/*
 * Ignore the linter error here, it doesn't understand that this is a C
 * wrapper in a C++ file.
 */
#include <stdlib.h> // NOLINT(modernize-deprecated-headers)
#include <stdint.h> // NOLINT(modernize-deprecated-headers)
#include <unistd.h>


#ifdef __cplusplus
extern "C" {
#endif

#include "joystick/joystick.h"

struct display;
typedef struct display display_t;

display_t *display_create();
[[maybe_unused]] void display_destroy(display_t *d);

void display_init(display_t *d);
[[maybe_unused]] void display_start(display_t *d);
void display_set_orientation(display_t *d, bool orientation);
void display_clear(display_t *d);
void display_draw_text_small(display_t *d, const char *text, uint8_t anchor_x, uint8_t anchor_y);
void display_draw_text_medium(display_t *d, const char *text, uint8_t anchor_x, uint8_t anchor_y);
void display_send_buffer(display_t *d);

#ifdef __cplusplus
}
#endif


#endif /* DISPLAY_WRAPPER_H_ */
