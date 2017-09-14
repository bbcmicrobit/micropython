#include <stdint.h>

// options to control how MicroPython is built

#define MICROPY_OBJ_REPR            (MICROPY_OBJ_REPR_C)
#define MICROPY_ALLOC_GC_STACK_SIZE (32)
#define MICROPY_ALLOC_PATH_MAX      (64)
#define MICROPY_QSTR_BYTES_IN_HASH  (1)
#define MICROPY_EMIT_X64            (0)
#define MICROPY_EMIT_THUMB          (0)
#define MICROPY_EMIT_INLINE_THUMB   (1)
#define MICROPY_EMIT_INLINE_THUMB_ARMV7M (0)
#define MICROPY_EMIT_INLINE_THUMB_FLOAT (0)
#define MICROPY_USE_SMALL_HEAP_COMPILER (1)
#define MICROPY_COMP_MODULE_CONST   (0)
#define MICROPY_COMP_CONST          (0)
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN (0)
#define MICROPY_COMP_TRIPLE_TUPLE_ASSIGN (0)
#define MICROPY_MEM_STATS           (0)
#define MICROPY_DEBUG_PRINTERS      (0)
#define MICROPY_ENABLE_GC           (1)
#define MICROPY_STACK_CHECK         (1)
#define MICROPY_KBD_EXCEPTION       (1)
#define MICROPY_REPL_EVENT_DRIVEN   (0)
#define MICROPY_REPL_EMACS_KEYS     (1)
#define MICROPY_REPL_AUTO_INDENT    (1)
#define MICROPY_HELPER_REPL         (1)
#define MICROPY_HELPER_LEXER_UNIX   (0)
#define MICROPY_ENABLE_SOURCE_LINE  (1)
#define MICROPY_ENABLE_DOC_STRING   (0)
#define MICROPY_ERROR_REPORTING     (MICROPY_ERROR_REPORTING_NORMAL)
#define MICROPY_STREAMS_NON_BLOCK   (1)
#define MICROPY_CAN_OVERRIDE_BUILTINS (1)
#define MICROPY_PY_BUILTINS_STR_UNICODE (1)
#define MICROPY_PY_BUILTINS_BYTEARRAY (1)
#define MICROPY_PY_BUILTINS_MEMORYVIEW (0)
#define MICROPY_PY_BUILTINS_ENUMERATE (1)
#define MICROPY_PY_BUILTINS_FROZENSET (1)
#define MICROPY_PY_BUILTINS_REVERSED (1)
#define MICROPY_PY_BUILTINS_SET     (1)
#define MICROPY_PY_BUILTINS_SLICE   (1)
#define MICROPY_PY_BUILTINS_PROPERTY (0)
#define MICROPY_PY_BUILTINS_INPUT   (1)
#define MICROPY_PY_BUILTINS_HELP    (1)
#define MICROPY_PY_BUILTINS_HELP_TEXT microbit_help_text
#define MICROPY_PY_BUILTINS_HELP_MODULES (1)
#define MICROPY_PY___FILE__         (0)
#define MICROPY_PY_MICROPYTHON_MEM_INFO (1)
#define MICROPY_PY_GC               (1)
#define MICROPY_PY_ARRAY            (1)
#define MICROPY_PY_ATTRTUPLE        (1)
#define MICROPY_PY_COLLECTIONS      (1)
#define MICROPY_PY_COLLECTIONS_ORDEREDDICT (1)
#define MICROPY_PY_MATH             (1)
#define MICROPY_PY_CMATH            (0)
#define MICROPY_PY_IO               (0)
#define MICROPY_PY_STRUCT           (1)
#define MICROPY_PY_SYS              (1)
#define MICROPY_PY_SYS_PLATFORM     "microbit"
#define MICROPY_PY_SYS_MODULES      (0)
#define MICROPY_PY_UTIME_MP_HAL     (1)
#define MICROPY_PY_MACHINE          (1)
#define MICROPY_PY_MACHINE_PULSE    (1)
#define MICROPY_MODULE_BUILTIN_INIT (1)
#define MICROPY_MODULE_FROZEN       (0)
#define MICROPY_CPYTHON_COMPAT      (0)
#define MICROPY_LONGINT_IMPL        (MICROPY_LONGINT_IMPL_MPZ)
#define MICROPY_FLOAT_IMPL          (MICROPY_FLOAT_IMPL_FLOAT)
#define MICROPY_PY_BUILTINS_COMPLEX (0)
#define MICROPY_HAL_HAS_VT100       (0)

// type definitions for the specific machine

#define BYTES_PER_WORD (4)

#define MICROPY_MAKE_POINTER_CALLABLE(p) ((void*)((mp_uint_t)(p) | 1))

#define UINT_FMT "%u"
#define INT_FMT "%d"
typedef int mp_int_t; // must be pointer size
typedef unsigned mp_uint_t; // must be pointer size

typedef void *machine_ptr_t; // must be of pointer size
typedef const void *machine_const_ptr_t; // must be of pointer size
typedef long mp_off_t;

void mp_hal_stdout_tx_strn_cooked(const char *str, mp_uint_t len);
#define MP_PLAT_PRINT_STRN(str, len) mp_hal_stdout_tx_strn_cooked(str, len)

// extra built in names to add to the global namespace
#define MICROPY_PORT_BUILTINS \
    { MP_OBJ_NEW_QSTR(MP_QSTR_open), (mp_obj_t)&mp_builtin_open_obj }, \

// extra builtin modules to add to the list of known ones
extern const struct _mp_obj_module_t microbit_module;
extern const struct _mp_obj_module_t music_module;
extern const struct _mp_obj_module_t this_module;
extern const struct _mp_obj_module_t antigravity_module;
extern const struct _mp_obj_module_t love_module;
extern const struct _mp_obj_module_t neopixel_module;
extern const struct _mp_obj_module_t random_module;
extern const struct _mp_obj_module_t os_module;
extern const struct _mp_obj_module_t radio_module;
extern const struct _mp_obj_module_t audio_module;
extern const struct _mp_obj_module_t speech_module;
extern const struct _mp_obj_module_t utime_module;
extern const struct _mp_obj_module_t machine_module;

#define MICROPY_PORT_BUILTIN_MODULES \
    { MP_ROM_QSTR(MP_QSTR_microbit), MP_ROM_PTR(&microbit_module) }, \
    { MP_ROM_QSTR(MP_QSTR_music), MP_ROM_PTR(&music_module) }, \
    { MP_ROM_QSTR(MP_QSTR_this), MP_ROM_PTR(&this_module) }, \
    { MP_ROM_QSTR(MP_QSTR_antigravity), MP_ROM_PTR(&antigravity_module) }, \
    { MP_ROM_QSTR(MP_QSTR_love), MP_ROM_PTR(&love_module) }, \
    { MP_ROM_QSTR(MP_QSTR_neopixel), MP_ROM_PTR(&neopixel_module) }, \
    { MP_ROM_QSTR(MP_QSTR_random), MP_ROM_PTR(&random_module) }, \
    { MP_ROM_QSTR(MP_QSTR_os), MP_ROM_PTR(&os_module) }, \
    { MP_ROM_QSTR(MP_QSTR_radio), MP_ROM_PTR(&radio_module) }, \
    { MP_ROM_QSTR(MP_QSTR_audio), MP_ROM_PTR(&audio_module) }, \
    { MP_ROM_QSTR(MP_QSTR_speech), MP_ROM_PTR(&speech_module) }, \
    { MP_ROM_QSTR(MP_QSTR_utime), MP_ROM_PTR(&utime_module) }, \
    { MP_ROM_QSTR(MP_QSTR_machine), MP_ROM_PTR(&machine_module) }, \
    \
    /* the following provide aliases for existing modules */ \
    { MP_ROM_QSTR(MP_QSTR_collections), MP_ROM_PTR(&mp_module_collections) }, \
    { MP_ROM_QSTR(MP_QSTR_struct), MP_ROM_PTR(&mp_module_ustruct) }, \
    { MP_ROM_QSTR(MP_QSTR_time), MP_ROM_PTR(&utime_module) }, \

#define MP_STATE_PORT MP_STATE_VM

#define MICROPY_PORT_ROOT_POINTERS \
    const char *readline_hist[8]; \
    void *async_data[2]; \
    uint8_t *radio_buf; \
    void *audio_buffer; \
    void *audio_source; \
    void *speech_data; \
    const struct _pwm_events *pwm_active_events; \
    const struct _pwm_events *pwm_pending_events; \
    struct _music_data_t *music_data; \

// We need to provide a declaration/definition of alloca()
#include <alloca.h>

#define MICROPY_HAL_H "mphal.h"
#define MICROPY_HW_BOARD_NAME "micro:bit"
#define MICROPY_HW_MCU_NAME "nRF51822"

// Toolchain seems to be missing M_PI
#ifndef M_PI
#define M_PI (3.141592653589793)
#endif

// The ticker callback function
extern void microbit_ticker(void);

