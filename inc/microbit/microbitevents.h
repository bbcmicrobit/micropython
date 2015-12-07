
#ifndef __MICROPY_INCLUDED_MICROBIT_MICROBITEVENTS_H__
#define __MICROPY_INCLUDED_MICROBIT_MICROBITEVENTS_H__

#define MICROBIT_EVENTS_NO_EVENT 255

#define MICROBIT_EVENTS_LOOP_NOT_RUNNING   0
#define MICROBIT_EVENTS_LOOP_MAYBE_RUNNING 1
#define MICROBIT_EVENTS_LOOP_RUNNING       2

#define MICROBIT_EVENTS_ACCELEROMETER_X 1
#define MICROBIT_EVENTS_ACCELEROMETER_Y 2
#define MICROBIT_EVENTS_ACCELEROMETER_Z 3

typedef struct _event_t {
    uint16_t scanner_id;
    struct _event_t* prev;
    struct _event_t* next;
} event_t;

typedef struct _event_queue_t {
    event_t *head;
    event_t *tail;
} event_queue_t;

typedef bool (*scanner_cb_t)(void*);

typedef struct _scanner_t {
    uint16_t id;
    scanner_cb_t scanner_cb;
    void* scanner_cb_args;
    struct _scanner_t* next;
} scanner_t;

typedef struct _scanner_list_t {
    uint16_t next_id;
    scanner_t *head;
    scanner_t *tail;
    event_queue_t *event_queue;
} scanner_list_t;

typedef struct _microbit_events_obj_t {
    event_queue_t *event_queue;
    scanner_list_t *scanner_list;
    uint8_t state;
} microbit_events_obj_t;

extern struct _microbit_events_obj_t* microbit_events_obj;

typedef struct _events_t {
    mp_obj_base_t base;
} events_t;

typedef struct _events_iterator_t {
    mp_obj_base_t base;
    events_t *events;
} events_iterator_t;

extern const mp_obj_type_t microbit_events_type;
extern const mp_obj_type_t microbit_events_iterator_type;

typedef struct _tick_scanner_args_t {
    uint32_t time_to_pop;
    uint16_t interval_ms;
} tick_scanner_args_t;

typedef struct _tick_once_scanner_args_t {
    uint32_t time_to_pop;
} tick_once_scanner_args_t;

typedef struct _button_scanner_args_t {
    uint8_t button_id;
} button_scanner_args_t;

typedef struct _compass_scanner_args_t {
    uint16_t angle1;
    uint16_t angle2;
    uint16_t last_angle;
} compass_scanner_args_t;

typedef struct _accelerometer_scanner_args_t {
    uint8_t direction;
    int16_t v1;
    int16_t v2;
    int16_t last_v;
} accelerometer_scanner_args_t;

extern "C" {
void microbit_events_tick(void);
bool tick_once_scanner(void *args);
}

#endif // __MICROPY_INCLUDED_MICROBIT_MICROBITEVENTS_H__
