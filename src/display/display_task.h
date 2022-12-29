
#ifndef DISPLAY_TASK_H
#define DISPLAY_TASK_H

#include <FreeRTOS.h>
#include <task.h>

#include "display/display_wrapper.h"

#include "controller-config.h"


void display_start_task_running(display_t *d);


portTASK_FUNCTION_PROTO(display_update_task, pvParameters);





#endif /* DISPLAY_TASK_H */