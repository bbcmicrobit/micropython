
/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


/*
 * When the events module is initialised, the global microbit_events_obj is
 * allocated.  This has pointers to a list of scanners and to an event queue.
 *
 * The purpose of a scanner is to watch for some condition to become true.
 *
 * A scanner has a callback function and some arguments, as well as a unique
 * id.  While the event loop is running, on each tick of the main loop, each
 * scanner's callback function is called with the given arguments, and if the
 * callback function returns true, an event with the scanner's id is added to
 * the event queue.
 *
 * The event loop is started when Python code iterates over events.events(),
 * and is stopped when the iteration is broken out of.  When the loop stops,
 * the event queue is drained, so that if it starts again, events from the last
 * time it was running are not processed this time.
 *
 * When events.events() is iterated over, it pops the oldest event off the
 * event queue, and yields the id of the event's scanner.  If the event queue
 * is empty, it yields None instead.  (It would be much better if it didn't
 * yield at all when the event queue is empty, but I haven't worked out how to
 * do this yet.)
 *
 * This allows us to write Python code like:
 *
 * >>> import events
 * >>> button_a_pressed = events.when_button_a_pressed()
 * >>> button_b_pressed = events.when_button_b_pressed()
 * >>> for e in events.events():
 * ...     if e == button_a_pressed:
 * ...         # handle button A being pressed
 * ...     elif e == button_b_pressed:
 * ...         # handle button B being pressed
 *
 * The list of currently-available events is:
 *
 * - events.tick(interval_ms)
 *     fires every interval_ms milliseconds
 * - events.tick_once(delay_ms)
 *     fires once, after interval_ms milliseconds
 * - events.when_button_[a|b]_pressed()
 *     fires when given button is pressed
 * - events.when_compass_between(angle1, angle2)
 *     fires when compass is between given angles
 * - events.when_accelerometer_[x|y|z]_between(value1, value2)
 *     fires when accelerometer's value in given direction is between values
 */

#include <stdio.h>
#include "MicroBit.h"

extern "C" {

#include "py/mphal.h"
#include "modmicrobit.h"
#include "microbitevents.h"
#include "microbitbutton.h"

microbit_events_obj_t* microbit_events_obj = NULL;

event_queue_t *event_queue_new() {
    event_queue_t *event_queue = (event_queue_t *)malloc(sizeof(event_queue_t));
    if (!event_queue) {
        return NULL;
    }
    event_queue->head = NULL;
    event_queue->tail = NULL;
    return event_queue;
}

void event_queue_enqueue(event_queue_t *event_queue, uint16_t scanner_id) {
    event_t *event = (event_t *)malloc(sizeof(event_t));
    if (!event) {
        return;  // Should we actually indicate failure?
    }

    if (event_queue->head == NULL) {
        event->next = NULL;
        event_queue->head = event;
        event_queue->tail = event;
    } else {
        event->next = event_queue->head;
        event_queue->head->prev = event;
        event_queue->head = event;
    };

    event->prev = NULL;
    event->scanner_id = scanner_id;
};

uint16_t event_queue_dequeue(event_queue_t *event_queue) {
    event_t *event;
    uint16_t scanner_id;
    if (event_queue->tail == NULL) {
        return MICROBIT_EVENTS_NO_EVENT;
    } else {
        event = event_queue->tail;
        scanner_id = event->scanner_id;
        if (event->prev == NULL) {
            event_queue->head = NULL;
            event_queue->tail = NULL;
        } else {
            event_queue->tail = event->prev;
            event_queue->tail->next = NULL;
        }
        free(event);
        return scanner_id;
    }
};

scanner_list_t *scanner_list_new(event_queue_t *event_queue) {
    scanner_list_t *scanner_list = (scanner_list_t *)malloc(sizeof(scanner_list_t));
    if (!scanner_list) {
        return NULL;
    }
    scanner_list->event_queue = event_queue;
    scanner_list->next_id = 1;
    scanner_list->head = NULL;
    scanner_list->tail = NULL;
    return scanner_list;
}

uint16_t scanner_list_add(scanner_list_t *scanner_list, scanner_cb_t scanner_cb, void *scanner_cb_args) {
    scanner_t *scanner = (scanner_t *)malloc(sizeof(scanner_t));

    if (!scanner) {
        return -1;
    }

    scanner->scanner_cb = scanner_cb;
    scanner->scanner_cb_args = scanner_cb_args;
    scanner->id = scanner_list->next_id;
    scanner_list->next_id++;

    if (scanner_list->head == NULL) {
        scanner_list->head = scanner;
    } else {
        scanner_list->tail->next = scanner;
    };
    scanner_list->tail = scanner;
    scanner->next = NULL;

    return scanner->id;
};

void scanner_list_remove(scanner_list_t *scanner_list, uint16_t scanner_id) {
    scanner_t *scanner, *prev_scanner;

    scanner = scanner_list->head;
    if (scanner != NULL) {
        prev_scanner = NULL;
        for (;;) {
            if (scanner->id == scanner_id) {
                if (prev_scanner == NULL) {
                    scanner_list->head = scanner->next;
                } else {
                    prev_scanner->next = scanner->next;
                };
                free(scanner->scanner_cb_args);
                free(scanner);
                break;
            } else {
                prev_scanner = scanner;
            };
            scanner = scanner->next;
            if (scanner == NULL) {
                break;
            };
        };
    };
};

void scanner_list_scan(scanner_list_t *scanner_list) {
    scanner_t *scanner;

    scanner = scanner_list->head;
    if (scanner != NULL) {
        for (;;) {
            if (scanner->scanner_cb(scanner->scanner_cb_args)) {
                event_queue_enqueue(scanner_list->event_queue, scanner->id);
		if (scanner->scanner_cb == tick_once_scanner) {
			scanner_list_remove(scanner_list, scanner->id);
		};
            };
            scanner = scanner->next;
            if (scanner == NULL) {
                break;
            };
        };
    };
};

void events_event_loop_start() {
    microbit_button_obj_t *button;

    button = microbit_get_button_by_id(MICROBIT_ID_BUTTON_A);
    button->pressed = button->pressed & -2;
    button = microbit_get_button_by_id(MICROBIT_ID_BUTTON_B);
    button->pressed = button->pressed & -2;

    microbit_events_obj->state = MICROBIT_EVENTS_LOOP_RUNNING;
};

void events_event_loop_stop() {
    uint16_t event_id;
    for (;;) {
        event_id = event_queue_dequeue(microbit_events_obj->event_queue);
        if (event_id == MICROBIT_EVENTS_NO_EVENT) {
            break;
        }
    }

    microbit_events_obj->state = MICROBIT_EVENTS_LOOP_NOT_RUNNING;
};

bool button_scanner(void *args) {
    button_scanner_args_t *button_args = (button_scanner_args_t *)args;
    microbit_button_obj_t *button = microbit_get_button_by_id(button_args->button_id);
    bool result;

    result = button->pressed & 1;
    if (result) {
        button->pressed = button->pressed & -2;
    };
    return result;
}

uint16_t add_button_scanner(uint8_t button_id) {
    uint16_t id;
    microbit_button_obj_t *button = microbit_get_button_by_id(button_id);
    button_scanner_args_t *args = (button_scanner_args_t *)malloc(sizeof(button_scanner_args_t));
    args->button_id = button_id;

    button->pressed = button->pressed & -2;

    id = scanner_list_add(microbit_events_obj->scanner_list, *button_scanner, args);
    return id;
}

bool tick_scanner(void *args) {
    tick_scanner_args_t *tick_args = (tick_scanner_args_t *)args;
    bool result;

    if (tick_args->time_to_pop <= uBit.systemTime()) {
        tick_args->time_to_pop += tick_args->interval_ms;
        result = true;
    } else {
        result = false;
    }

    return result;
}

uint16_t add_tick_scanner(uint16_t interval_ms) {
    uint16_t id;
    tick_scanner_args_t *args = (tick_scanner_args_t *)malloc(sizeof(tick_scanner_args_t));
    args->time_to_pop = uBit.systemTime();
    args->interval_ms = interval_ms;

    id = scanner_list_add(microbit_events_obj->scanner_list, *tick_scanner, args);
    return id;
}

bool tick_once_scanner(void *args) {
    tick_once_scanner_args_t *tick_once_args = (tick_once_scanner_args_t *)args;
    bool result;

    if (tick_once_args->time_to_pop <= uBit.systemTime()) {
        result = true;
    } else {
        result = false;
    }

    return result;
}

uint16_t add_tick_once_scanner(uint16_t delay_ms) {
    uint16_t id;
    tick_once_scanner_args_t *args = (tick_once_scanner_args_t *)malloc(sizeof(tick_once_scanner_args_t));
    args->time_to_pop = uBit.systemTime() + delay_ms;

    id = scanner_list_add(microbit_events_obj->scanner_list, *tick_once_scanner, args);
    return id;
}

bool compass_scanner(void *args) {
    compass_scanner_args_t *compass_args = (compass_scanner_args_t *)args;
    bool result;
    uint16_t angle1 = compass_args->angle1;
    uint16_t angle2 = compass_args->angle2;
    uint16_t last_angle = compass_args->last_angle;
    uBit.compass.idleTick();
    uint16_t heading = uBit.compass.heading();

    if (angle1 < angle2) {
        if (angle1 < last_angle && last_angle < angle2) {
            result = false;
        } else if (angle1 < heading && heading < angle2) {
            result = true;
        } else {
            result = false;
        }
    } else {
        if (angle1 < last_angle || last_angle < angle2) {
            result = false;
        } else if (angle1 < heading || heading < angle2) {
            result = true;
        } else {
            result = false;
        }
    }

    compass_args->last_angle = uBit.compass.heading();
    return result;
}

uint16_t add_compass_scanner(uint16_t angle1, uint16_t angle2) {
    uint16_t id;
    compass_scanner_args_t *args = (compass_scanner_args_t *)malloc(sizeof(compass_scanner_args_t));
    args->angle1 = angle1;
    args->angle2 = angle2;
    args->last_angle = uBit.compass.heading();

    id = scanner_list_add(microbit_events_obj->scanner_list, *compass_scanner, args);
    return id;
}

bool accelerometer_scanner(void *args) {
    accelerometer_scanner_args_t *accelerometer_args = (accelerometer_scanner_args_t *)args;
    bool result;
    int16_t v1 = accelerometer_args->v1;
    int16_t v2 = accelerometer_args->v2;
    int16_t last_v = accelerometer_args->last_v;
    int16_t v;
    uBit.accelerometer.idleTick();

    switch (accelerometer_args->direction) {
        case MICROBIT_EVENTS_ACCELEROMETER_X:
            v = uBit.accelerometer.getX();
            break;
        case MICROBIT_EVENTS_ACCELEROMETER_Y:
            v = uBit.accelerometer.getY();
            break;
        case MICROBIT_EVENTS_ACCELEROMETER_Z:
            v = uBit.accelerometer.getZ();
            break;
        default:
            v = 0;
    }

    if (v1 < last_v && last_v < v2) {
        result = false;
    } else if (v1 < v && v < v2) {
        result = true;
    } else {
        result = false;
    }

    accelerometer_args->last_v = v;
    return result;
}

uint16_t add_accelerometer_scanner(uint8_t direction, int16_t v1, int16_t v2) {
    uint16_t id;
    accelerometer_scanner_args_t *args = (accelerometer_scanner_args_t *)malloc(sizeof(accelerometer_scanner_args_t));
    args->direction = direction;

    if (v1 < v2) {
        args->v1 = v1;
        args->v2 = v2;
    } else {
        args->v1 = v2;
        args->v2 = v1;
    }

    id = scanner_list_add(microbit_events_obj->scanner_list, *accelerometer_scanner, args);
    return id;
}

STATIC mp_obj_t events_tick(mp_obj_t ms_in) {
    mp_int_t interval_ms = mp_obj_get_int(ms_in);
    return mp_obj_new_int(add_tick_scanner(interval_ms));
}
MP_DEFINE_CONST_FUN_OBJ_1(events_tick_obj, events_tick);

STATIC mp_obj_t events_tick_once(mp_obj_t ms_in) {
    mp_int_t delay_ms = mp_obj_get_int(ms_in);
    return mp_obj_new_int(add_tick_once_scanner(delay_ms));
}
MP_DEFINE_CONST_FUN_OBJ_1(events_tick_once_obj, events_tick_once);

STATIC mp_obj_t events_when_button_a_pressed(void) {
    return mp_obj_new_int(add_button_scanner(MICROBIT_ID_BUTTON_A));
}
MP_DEFINE_CONST_FUN_OBJ_0(events_when_button_a_pressed_obj, events_when_button_a_pressed);

STATIC mp_obj_t events_when_button_b_pressed(void) {
    return mp_obj_new_int(add_button_scanner(MICROBIT_ID_BUTTON_B));
}
MP_DEFINE_CONST_FUN_OBJ_0(events_when_button_b_pressed_obj, events_when_button_b_pressed);

STATIC mp_obj_t events_when_compass_between(mp_obj_t angle1_in, mp_obj_t angle2_in) {
    mp_int_t angle1 = mp_obj_get_int(angle1_in);
    mp_int_t angle2 = mp_obj_get_int(angle2_in);
    return mp_obj_new_int(add_compass_scanner(angle1, angle2));
}
MP_DEFINE_CONST_FUN_OBJ_2(events_when_compass_between_obj, events_when_compass_between);

STATIC mp_obj_t events_when_accelerometer_x_between(mp_obj_t v1_in, mp_obj_t v2_in) {
    mp_int_t v1 = mp_obj_get_int(v1_in);
    mp_int_t v2 = mp_obj_get_int(v2_in);
    return mp_obj_new_int(add_accelerometer_scanner(MICROBIT_EVENTS_ACCELEROMETER_X, v1, v2));
}
MP_DEFINE_CONST_FUN_OBJ_2(events_when_accelerometer_x_between_obj, events_when_accelerometer_x_between);

STATIC mp_obj_t events_when_accelerometer_y_between(mp_obj_t v1_in, mp_obj_t v2_in) {
    mp_int_t v1 = mp_obj_get_int(v1_in);
    mp_int_t v2 = mp_obj_get_int(v2_in);
    return mp_obj_new_int(add_accelerometer_scanner(MICROBIT_EVENTS_ACCELEROMETER_Y, v1, v2));
}
MP_DEFINE_CONST_FUN_OBJ_2(events_when_accelerometer_y_between_obj, events_when_accelerometer_y_between);

STATIC mp_obj_t events_when_accelerometer_z_between(mp_obj_t v1_in, mp_obj_t v2_in) {
    mp_int_t v1 = mp_obj_get_int(v1_in);
    mp_int_t v2 = mp_obj_get_int(v2_in);
    return mp_obj_new_int(add_accelerometer_scanner(MICROBIT_EVENTS_ACCELEROMETER_Z, v1, v2));
}
MP_DEFINE_CONST_FUN_OBJ_2(events_when_accelerometer_z_between_obj, events_when_accelerometer_z_between);

mp_obj_t microbit_events(void){
    events_t *result = m_new_obj(events_t);
    result->base.type = &microbit_events_type;
    return result;
}

STATIC mp_obj_t get_microbit_events_iter(mp_obj_t o_in) {
    events_t *events = (events_t *)o_in;
    events_iterator_t *result = m_new_obj(events_iterator_t);
    result->base.type = &microbit_events_iterator_type;
    result->events = events;
    return result;
}

STATIC mp_obj_t microbit_events_iter_next(mp_obj_t o_in) {
    // Is there a way to mark o_in as unused?
    uint16_t event_id;
    microbit_events_obj->state = MICROBIT_EVENTS_LOOP_RUNNING;
    event_id = event_queue_dequeue(microbit_events_obj->event_queue);
    if (event_id == MICROBIT_EVENTS_NO_EVENT) {
        // Is there a way to indicate that the iterator should not yield?
        return mp_const_none;
    } else {
        return mp_obj_new_int(event_id);
    }
}

const mp_obj_type_t microbit_events_type = {
    { &mp_type_type },
    .name = MP_QSTR_events,
    .print = NULL,
    .make_new = NULL,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = NULL,
    .subscr = NULL,
    .getiter = get_microbit_events_iter,
    .iternext = NULL,
    .buffer_p = {NULL},
    .stream_p = NULL,
    .bases_tuple = NULL,
    .locals_dict = NULL,
};

const mp_obj_type_t microbit_events_iterator_type = {
    { &mp_type_type },
    .name = MP_QSTR_iterator,
    .print = NULL,
    .make_new = NULL,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = NULL,
    .subscr = NULL,
    .getiter = mp_identity,
    .iternext = microbit_events_iter_next,
    .buffer_p = {NULL},
    .stream_p = NULL,
    .bases_tuple = NULL,
    .locals_dict = NULL,
};

STATIC mp_obj_t events_events(void) {
    events_event_loop_start();
    return microbit_events();
}
MP_DEFINE_CONST_FUN_OBJ_0(events_events_obj, events_events);

STATIC mp_obj_t events_remove_scanner(mp_obj_t scanner_id_in) {
    mp_int_t scanner_id = mp_obj_get_int(scanner_id_in);
    scanner_list_remove(microbit_events_obj->scanner_list, scanner_id);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(events_remove_scanner_obj, events_remove_scanner);

STATIC mp_obj_t events__init__(void) {
    microbit_events_obj = (microbit_events_obj_t *)malloc(sizeof(microbit_events_obj_t));
    event_queue_t *event_queue = event_queue_new();
    scanner_list_t *scanner_list = scanner_list_new(event_queue);
    
    microbit_events_obj->event_queue = event_queue;
    microbit_events_obj->scanner_list = scanner_list;
    microbit_events_obj->state = MICROBIT_EVENTS_LOOP_NOT_RUNNING;

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(events___init___obj, events__init__);

STATIC const mp_map_elem_t events_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_events) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&events___init___obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_events), (mp_obj_t)&events_events_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_remove_scanner), (mp_obj_t)&events_remove_scanner_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_tick), (mp_obj_t)&events_tick_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_tick_once), (mp_obj_t)&events_tick_once_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_when_button_a_pressed), (mp_obj_t)&events_when_button_a_pressed_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_when_button_b_pressed), (mp_obj_t)&events_when_button_b_pressed_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_when_compass_between), (mp_obj_t)&events_when_compass_between_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_when_accelerometer_x_between), (mp_obj_t)&events_when_accelerometer_x_between_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_when_accelerometer_y_between), (mp_obj_t)&events_when_accelerometer_y_between_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_when_accelerometer_z_between), (mp_obj_t)&events_when_accelerometer_z_between_obj},
};

STATIC MP_DEFINE_CONST_DICT(events_module_globals, events_module_globals_table);

const mp_obj_module_t events_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_events,
    .globals = (mp_obj_dict_t*)&events_module_globals,
};

void microbit_events_tick(void) {
    if (microbit_events_obj == NULL) {
        return;
    }

    if (microbit_events_obj->state == MICROBIT_EVENTS_LOOP_RUNNING) {
        microbit_events_obj->state = MICROBIT_EVENTS_LOOP_MAYBE_RUNNING;
        scanner_list_scan(microbit_events_obj->scanner_list);
    } else {
        events_event_loop_stop();
    }
}
}
