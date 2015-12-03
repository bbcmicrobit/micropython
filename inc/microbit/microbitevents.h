
#ifndef __MICROPY_INCLUDED_MICROBIT_MICROBITEVENTS_H__
#define __MICROPY_INCLUDED_MICROBIT_MICROBITEVENTS_H__

#define MICROBIT_EVENTS_NO_EVENT 255

#define MICROBIT_EVENTS_ACCELEROMETER_X 1
#define MICROBIT_EVENTS_ACCELEROMETER_Y 2
#define MICROBIT_EVENTS_ACCELEROMETER_Z 3

typedef struct event_t {
	uint16_t scanner_id;
	struct event_t* prev;
	struct event_t* next;
} event_t;

typedef struct {
	event_t *head;
	event_t *tail;
} event_queue_t;

typedef bool (*scanner_cb_t)(void*);

typedef struct scanner_t {
	uint16_t id;
	scanner_cb_t scanner_cb;
	void* scanner_cb_args;
	struct scanner_t* next;
} scanner_t;

typedef struct {
	uint16_t next_id;
	scanner_t *head;
	scanner_t *tail;
	event_queue_t *event_queue;
} scanner_list_t;

typedef struct _microbit_events_obj_t {
	event_queue_t *event_queue;
	scanner_list_t *scanner_list;
} microbit_events_obj_t;

extern struct _microbit_events_obj_t* microbit_events_obj;

typedef struct {
	uint32_t time_to_pop;
	uint16_t interval_ms;
} tick_scanner_args_t;

typedef struct {
	uint8_t button_id;
} button_scanner_args_t;

typedef struct {
	uint16_t angle1;
	uint16_t angle2;
	uint16_t last_heading;
} compass_scanner_args_t;

typedef struct {
	uint8_t direction;
	int16_t v1;
	int16_t v2;
	int16_t last_v;
} accelerometer_scanner_args_t;

extern "C" {
void microbit_events_tick(void);
}

#endif // __MICROPY_INCLUDED_MICROBIT_MICROBITEVENTS_H__
