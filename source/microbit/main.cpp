#include "lib/ticker.h"
#include "lib/pwm.h"
#include "microbit/memory.h"
#include "microbit/filesystem.h"
#include "microbit/microbitdal.h"
#include "_newlib_version.h"

// Global instances of the DAL components that we use
MicroBitDisplay ubit_display;
MicroPythonI2C ubit_i2c(I2C_SDA0, I2C_SCL0);
MicroBitAccelerometer ubit_accelerometer(ubit_i2c);
MicroBitCompass ubit_compass(ubit_i2c, ubit_accelerometer);
MicroBitCompassCalibrator ubit_compass_calibrator(ubit_compass, ubit_accelerometer, ubit_display);

extern "C" {

#include "py/stackctrl.h"
#include "py/gc.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "lib/mp-readline/readline.h"
#include "lib/utils/pyexec.h"
#include "microbit/modmicrobit.h"
#include "microbit/modmusic.h"

void microbit_ticker(void) {
    // Update compass if it is calibrating, but not if it is still
    // updating as compass.idleTick() is not reentrant.
    if (ubit_compass.isCalibrating() && !compass_updating) {
        ubit_compass.idleTick();
    }

    compass_up_to_date = false;
    accelerometer_up_to_date = false;

    // Update buttons and pins with touch.
    microbit_button_tick();

    // Update the display.
    microbit_display_tick();

    // Update the music
    microbit_music_tick();
}

static void microbit_display_exception(mp_obj_t exc_in) {
    mp_uint_t n, *values;
    mp_obj_exception_get_traceback(exc_in, &n, &values);
    if (n >= 3) {
        vstr_t vstr;
        mp_print_t print;
        vstr_init_print(&vstr, 50, &print);
        #if MICROPY_ENABLE_SOURCE_LINE
        mp_printf(&print, "line %u ", values[1]);
        #endif
        if (mp_obj_is_native_exception_instance(exc_in)) {
            mp_obj_exception_t *exc = (mp_obj_exception_t*)MP_OBJ_TO_PTR(exc_in);
            mp_printf(&print, "%q ", exc->base.type->name);
            if (exc->args != NULL && exc->args->len != 0) {
                mp_obj_print_helper(&print, exc->args->items[0], PRINT_STR);
            }
        }
        // Allow ctrl-C to stop the scrolling message
        mp_hal_set_interrupt_char(CHAR_CTRL_C);
        mp_hal_display_string(vstr_null_terminated_str(&vstr));
        vstr_clear(&vstr);
        mp_hal_set_interrupt_char(-1);
        // This is a variant of mp_handle_pending that swallows exceptions
        #if MICROPY_ENABLE_SCHEDULER
        #error Scheduler currently unsupported
        #endif
        if (MP_STATE_VM(mp_pending_exception) != MP_OBJ_NULL) {
            MP_STATE_VM(mp_pending_exception) = MP_OBJ_NULL;
        }
    }
}

static void do_lexer(mp_lexer_t *lex) {
    if (lex == NULL) {
        printf("MemoryError: lexer could not allocate memory\n");
        return;
    }

    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        qstr source_name = lex->source_name;
        mp_parse_tree_t parse_tree = mp_parse(lex, MP_PARSE_FILE_INPUT);
        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, MP_EMIT_OPT_NONE, false);
        mp_hal_set_interrupt_char(3); // allow ctrl-C to interrupt us
        mp_call_function_0(module_fun);
        mp_hal_set_interrupt_char(-1); // disable interrupt
        nlr_pop();
    } else {
        // uncaught exception
        mp_hal_set_interrupt_char(-1); // disable interrupt

        // print exception to stdout
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);

        // print exception to the display, but not if it's SystemExit or KeyboardInterrupt
        mp_obj_type_t *exc_type = mp_obj_get_type((mp_obj_t)nlr.ret_val);
        if (!mp_obj_is_subclass_fast(exc_type, &mp_type_SystemExit)
            && !mp_obj_is_subclass_fast(exc_type, &mp_type_KeyboardInterrupt)) {
            microbit_display_exception(nlr.ret_val);
        }
    }
}

static void do_strn(const char *src, size_t len) {
    mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR___main__, src, len, 0);
    do_lexer(lex);
}

static void do_file(file_descriptor_obj *fd) {
    mp_lexer_t *lex = microbit_file_lexer(MP_QSTR___main__, fd);
    do_lexer(lex);
}

typedef struct _appended_script_t {
    byte header[2]; // should be "MP"
    uint16_t len; // length of script stored little endian
    char str[]; // data of script
} appended_script_t;

#define APPENDED_SCRIPT ((const appended_script_t*)microbit_mp_appended_script())

int main(void) {
    for (;;) {
        extern uint32_t __StackTop;
#if __NEWLIB__ > 2 || (__NEWLIB__ == 2 && (__NEWLIB_MINOR__ > 4 || (__NEWLIB_MINOR == 4 && __NEWLIB_PATCHLEVEL > 0)))
        // newlib >2.4.0 uses more RAM for locale data.
#warning "Detected newlib >2.4.0 so reducing heap by 300 bytes. See https://github.com/bbcmicrobit/micropython/issues/363 for details."
        static uint32_t mp_heap[9940 / sizeof(uint32_t)];
#else
        static uint32_t mp_heap[10240 / sizeof(uint32_t)];
#endif

        // Initialise memory regions: stack and MicroPython heap
        mp_stack_set_top(&__StackTop);
        mp_stack_set_limit(1800); // stack is 2k
        gc_init(mp_heap, (uint8_t*)mp_heap + sizeof(mp_heap));

        // Initialise the MicroPython runtime
        mp_init();
        mp_hal_init();
        readline_init0();

        // Initialise the micro:bit peripherals
        microbit_seed_random();
        ubit_display.disable();
        microbit_display_init();
        microbit_filesystem_init();
        microbit_pin_init();
        microbit_compass_init();
        pwm_init();
        MP_STATE_PORT(radio_buf) = NULL;

        // Start our ticker
        // Note that the DAL has a separate ticker which is also running
        ticker_init(microbit_ticker);
        ticker_start();
        pwm_start();

        // Only run initial script (or import from microbit) if we are in "friendly REPL"
        // mode.  If we are in "raw REPL" mode then this will be skipped.
        if (pyexec_mode_kind == PYEXEC_MODE_FRIENDLY_REPL) {
            file_descriptor_obj *main_module;
            if (APPENDED_SCRIPT->header[0] == 'M' && APPENDED_SCRIPT->header[1] == 'P') {
                // run appended script
                do_strn(APPENDED_SCRIPT->str, APPENDED_SCRIPT->len);
            } else if ((main_module = microbit_file_open("main.py", 7, false, false))) {
                do_file(main_module);
            } else {
                // from microbit import *
                mp_import_all(mp_import_name(MP_QSTR_microbit, mp_const_empty_tuple, MP_OBJ_NEW_SMALL_INT(0)));
            }
        }

        // Run the REPL until the user wants to exit
        for (;;) {
            if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
                if (pyexec_raw_repl() != 0) {
                    break;
                }
            } else {
                if (pyexec_friendly_repl() != 0) {
                    break;
                }
            }
        }

        // Print the special string for pyboard.py to detect the soft reset
        mp_hal_stdout_tx_str("soft reboot\r\n");

        // Stop the ticker to prevent any background tasks from running
        ticker_stop();

        // Reset state associated with background tasks
        memset(&MP_STATE_PORT(async_data)[0], 0, sizeof(MP_STATE_PORT(async_data)));
        MP_STATE_PORT(audio_buffer) = NULL;
        MP_STATE_PORT(music_data) = NULL;
    }
}

mp_import_stat_t mp_import_stat(const char *path) {
    if (microbit_find_file(path, strlen(path)) != FILE_NOT_FOUND) {
        return MP_IMPORT_STAT_FILE;
    }
    return MP_IMPORT_STAT_NO_EXIST;
}

NORETURN void nlr_jump_fail(void *val) {
    (void)val;
    for (;;) {
    }
}

// We need to override this function so that the linker does not pull in
// unnecessary code and static RAM usage for unused system exit functionality.
// There can be large static data structures to store the exit functions.
void __register_exitproc() {
}

}
