

#include <stdio.h>

#include "controller-config.h"

#include "joystick/adc.h"
#include "joystick/joystick.h"

#include "logging/logging.h"


// Keep track of the number of axis we read
uint8_t number_of_axen;
axis* axis_collection[MAX_NUMBER_OF_AXEN];

// The current state of the buttons. This needs to be done as a mask
// so that we can send it to the computer over USB as a gamepad HID
// device report
button_t button_state_mask = 0;

/**
 * This is an array of masks for the MUX for the buttons
 *
 * Right now only 0-7 are actually being used, but it doesn't hurt to add them all for later.
 */
const uint32_t BUTTON_MUX_MASKS[16] = {
        0, // 0000 - All lines off
        GPIO_MASK(BUTTON_MUX0), // 0001 - Line 0 on
        GPIO_MASK(BUTTON_MUX1), // 0010 - Line 1 on
        GPIO_MASK(BUTTON_MUX0) | GPIO_MASK(BUTTON_MUX1), // 0011 - Lines 0 and 1 on
        GPIO_MASK(BUTTON_MUX2), // 0100 - Line 2 on
        GPIO_MASK(BUTTON_MUX2) | GPIO_MASK(BUTTON_MUX0), // 0101 - Lines 2 and 0 on
        GPIO_MASK(BUTTON_MUX2) | GPIO_MASK(BUTTON_MUX1), // 0110 - Lines 2 and 1 on
        GPIO_MASK(BUTTON_MUX2) | GPIO_MASK(BUTTON_MUX1) | GPIO_MASK(BUTTON_MUX0), // 0111 - Lines 2, 1, and 0 on
        GPIO_MASK(BUTTON_MUX3), // 1000 - Line 3 on
        GPIO_MASK(BUTTON_MUX3) | GPIO_MASK(BUTTON_MUX0), // 1001 - Lines 3 and 0 on
        GPIO_MASK(BUTTON_MUX3) | GPIO_MASK(BUTTON_MUX1), // 1010 - Lines 3 and 1 on
        GPIO_MASK(BUTTON_MUX3) | GPIO_MASK(BUTTON_MUX1) | GPIO_MASK(BUTTON_MUX0), // 1011 - Lines 3, 1, and 0 on
        GPIO_MASK(BUTTON_MUX3) | GPIO_MASK(BUTTON_MUX2), // 1100 - Lines 3 and 2 on
        GPIO_MASK(BUTTON_MUX3) | GPIO_MASK(BUTTON_MUX2) | GPIO_MASK(BUTTON_MUX0), // 1101 - Lines 3, 2, and 0 on
        GPIO_MASK(BUTTON_MUX3) | GPIO_MASK(BUTTON_MUX2) | GPIO_MASK(BUTTON_MUX1), // 1110 - Lines 3, 2, and 1 on
        GPIO_MASK(BUTTON_MUX3) | GPIO_MASK(BUTTON_MUX2) | GPIO_MASK(BUTTON_MUX1) | GPIO_MASK(BUTTON_MUX0)  // 1111 - All lines on
};

const uint32_t BUTTON_GPIO_MASK = BUTTON_MUX_MASKS[15];

void init_reader() {

    number_of_axen = 0;

    // Wipe out our axis collection
    memset(axis_collection, '\0', sizeof(axis*) * MAX_NUMBER_OF_AXEN);
    debug("created the array of axen");

    // Set up all of the pins on the MUX
    for( int i = BUTTON_MUX0; i <= BUTTON_MUX3; i++) {
        gpio_set_function(i, GPIO_FUNC_SIO);
        gpio_set_dir(i, 1);
        gpio_pull_up(i);
    }

    // ...and the button input pin
    gpio_set_function(BUTTON_IN, GPIO_FUNC_SIO);
    gpio_set_dir(BUTTON_IN, 0);

    debug("configured the MUX and button input GPIO pins");
}

void register_axis(axis* a) {

    if(number_of_axen >= MAX_NUMBER_OF_AXEN){
        fatal("more than %d axen registered", MAX_NUMBER_OF_AXEN);
        tight_loop_contents();
    }

    axis_collection[number_of_axen] = a;
    number_of_axen++;
    debug("registered new axis on channel %d. total number: %d", a->adc_channel, number_of_axen);
}

/**
 * @brief Reads a value on an axis from the hardware
 *
 * @param a the axis to check (in/out)
 * @param read_mode which mode should we be in?
 */
void read_value(axis* a) {

    uint16_t read_value = joystick_read_adc(a->adc_channel);

    if(a->inverted) {
        read_value = a->adc_max - read_value;
    }

    // Update the raw value
    a->raw_value = read_value;

    if(read_value > a->adc_max) {
        warning("clipping adc channel %d reading at %d (was %d)",
                a->adc_channel, a->adc_max, read_value);
        read_value = a->adc_max;
    }

    if(read_value < a->adc_min) {
        warning("clipping adc channel %d reading at %d (was %d)",
                a->adc_channel, a->adc_min, read_value);
        read_value = a->adc_min;
    }

    // Update the filter
    analog_filter_update(&a->filter, read_value);

    // Get the filter's current value
    uint16_t filter_value = analog_filter_get_value(&a->filter);

    // Convert this to an 8-bit value
    a->filtered_value = (uint8_t)(filter_value >> 4);

    verbose("read adc %d - raw: %d, filtered: %d, 8-bit: %d",
            a->adc_channel,read_value, filter_value, a->filtered_value);
}


axis create_axis(uint8_t adc_channel) {
    axis a;
    a.adc_channel = adc_channel;
    a.raw_value = 0;
    a.filtered_value = 0;
    a.adc_max = 4095;       // We're using 12 bit ADCs
    a.adc_min = 0;
    a.inverted = false;
    a.filter = create_analog_filter(true, (float)ANALOG_READ_FILTER_SNAP_VALUE);

    debug("created a new axis on ADC channel %u", adc_channel);

    return a;
}


joystick create_2axis_joystick(uint8_t x_adc_channel, uint8_t y_adc_channel) {

    joystick j;
    axis x, y;

    x = create_axis(x_adc_channel);
    y = create_axis(y_adc_channel);

    j.x = x;
    j.y = y;

    debug("created a new joystick");
    return j;

}

joystick create_3axis_joystick(uint8_t x_adc_channel, uint8_t y_adc_channel, uint8_t z_adc_channel) {

    joystick j;
    axis x, y, z;

    x = create_axis(x_adc_channel);
    y = create_axis(y_adc_channel);
    z = create_axis(z_adc_channel);

    j.x = x;
    j.y = y;
    j.z = z;

    debug("created a new joystick");
    return j;

}

pot create_pot(uint8_t adc_channel) {

    pot p;
    axis z;

    z = create_axis(adc_channel);
    p.z = z;
    debug("created a new pot");
    return p;

}



void setButton(button_t *buttonState, uint8_t button) {
    if (button < MAX_NUMBER_OF_BUTTONS) {
        *buttonState |= (1u << button);
    }
}

void clearButton(button_t *buttonState, uint8_t button) {
    if (button < MAX_NUMBER_OF_BUTTONS) {
        *buttonState &= ~(1u << button);
    }
}

void toggleButton(button_t *buttonState, uint8_t button) {
    if (button < MAX_NUMBER_OF_BUTTONS) {
        *buttonState ^= (1u << button);
    }
}


TaskHandle_t start_analog_reader_task()
{
    TaskHandle_t reader_handle;

    xTaskCreate(analog_reader_task,
                "analog_reader_task",
                configMINIMAL_STACK_SIZE + 512,
                (void*)0,
                1,
                &reader_handle);

#ifdef SUSPEND_READER_WHEN_NO_USB
    // Start off suspended! Will be started when the device is
    // mounted on the host
    vTaskSuspend(reader_handle);
#endif

    return reader_handle;
}

TaskHandle_t start_button_reader_task()
{
    TaskHandle_t reader_handle;

    xTaskCreate(button_reader_task,
                "button_reader_task",
                configMINIMAL_STACK_SIZE + 512,
                (void*)0,
                1,
                &reader_handle);

#ifdef SUSPEND_READER_WHEN_NO_USB
    // Start off suspended! Will be started when the device is
    // mounted on the host
    vTaskSuspend(reader_handle);
#endif

    return reader_handle;
}


portTASK_FUNCTION(analog_reader_task, pvParameters) {

    joystick_adc_init();

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for(EVER) {

        for(int i = 0; i < number_of_axen; i++) {

            axis* a = axis_collection[i];
            read_value(a);
            verbose("read value %d (%d) from adc_channel %d", a->filtered_value, a->raw_value,  a->adc_channel);
        }

        vTaskDelay(pdMS_TO_TICKS(POLLING_INTERVAL));

    }

#pragma clang diagnostic pop

}


portTASK_FUNCTION(button_reader_task, pvParameters) {


    info("starting the button reader task");

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for(EVER) {

        // Walk all of the buttons and update the button state var
        for(uint8_t i = 0; i < MAX_NUMBER_OF_BUTTONS; i++) {

            // Set the MUX mask for this button
            gpio_put_masked(BUTTON_GPIO_MASK, BUTTON_MUX_MASKS[i]);

            // This might not be needed. The MUX's datasheet says it needs 200ns.
            sleep_us(1);

            if(!gpio_get(BUTTON_IN)) {
                setButton(&button_state_mask, i);
            }
            else {
                clearButton(&button_state_mask, i);
            }

            if(i == 4) {
                debug("button 4 is %s", gpio_get(BUTTON_IN) ? "not pressed" : "pressed");
            } else if(i == 6) {
                debug("button 6 is %s", gpio_get(BUTTON_IN) ? "not pressed" : "pressed");
            }
        }


        vTaskDelay(pdMS_TO_TICKS(POLLING_INTERVAL));

    }

#pragma clang diagnostic pop

}