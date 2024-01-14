

/*
 * C wrapper for the CPP object that the SSD-1360 library uses.
 *
 * Big thank you to:
 *    https://nachtimwald.com/2017/08/18/wrapping-c-objects-in-c/
 *
 * ...for teaching me how this stuff works!
 */


// SSD-1306 lib
#include "ssd1306.h"

// Ours
#include "display.h"
#include "display_wrapper.h"
#include "logging/logging.h"


struct display {
    void *obj;
};


/**
 * @brief Creates an instance of a Display an wraps it in a `display_t` struct
 *
 * @return a pointer to a newly created display_t
 */
display_t *display_create()
{
    display_t *d;
    Display *obj;

    d      = (typeof(d))malloc(sizeof(*d));
    obj    = new Display();
    d->obj = obj;

    return d;
}

/**
 * @brief Removes a Display instance from memory
 *
 * This never actually gets used. We never delete displays since it's a representation
 * of an actual device. It's included here just to be complete. 😅
 *
 * @param d a pointer to the `display_t` that the display represents
 */
[[maybe_unused]] void display_destroy(display_t *d)
{
    if (d == nullptr)
        return;
    delete static_cast<Display *>(d->obj);
    free(d);
}

void display_set_orientation(display_t *d, bool orientation) {

    Display *obj;

    if (d == nullptr)
        return;

    obj = static_cast<Display *>(d->obj);
    obj->setOrientation(orientation);
}

void display_draw_text_small(display_t *d, const char *text, uint8_t anchor_x, uint8_t anchor_y) {

    Display *obj;

    if (d == nullptr)
        return;

    obj = static_cast<Display *>(d->obj);
    obj->drawTextSmall(text, anchor_x, anchor_y);
}

void display_draw_text_medium(display_t *d, const char *text, uint8_t anchor_x, uint8_t anchor_y) {

    Display *obj;

    if (d == nullptr)
        return;

    obj = static_cast<Display *>(d->obj);
    obj->drawTextMedium(text, anchor_x, anchor_y);
}

void display_send_buffer(display_t *d) {
    Display *obj;

    if (d == nullptr)
        return;

    obj = static_cast<Display *>(d->obj);
    obj->sendBuffer();
}

void display_init(display_t *d) {
    Display *obj;

    if (d == nullptr)
        return;

    obj = static_cast<Display *>(d->obj);
    obj->init();
}

[[maybe_unused]] void display_start(display_t *d) {
    Display *obj;

    if (d == nullptr)
        return;

    obj = static_cast<Display *>(d->obj);
    obj->start();
}

void display_clear(display_t *d) {
    Display *obj;

    if (d == nullptr)
        return;

    obj = static_cast<Display *>(d->obj);
    obj->clear();
}