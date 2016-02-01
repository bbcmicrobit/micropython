#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "py/nlr.h"
#include "py/stackctrl.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mphal.h"
#include "lib/readline.h"
#include "lib/utils/pyexec.h"

extern void microbit_init(void);
    
void microbit_display_exception(mp_obj_t exc_in) {
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
            mp_obj_exception_t *exc = exc_in;
            mp_printf(&print, "%q ", exc->base.type->name);
            if (exc->args != NULL && exc->args->len != 0) {
                mp_obj_print_helper(&print, exc->args->items[0], PRINT_STR);
            }
        }
        mp_hal_display_string(vstr_null_terminated_str(&vstr));
        vstr_clear(&vstr);
    }
}

void do_strn(const char *src, size_t len) {
    mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR___main__, src, len, 0);
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
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
        microbit_display_exception(nlr.ret_val);
    }
}

static char *stack_top;

typedef struct _appended_script_t {
    byte header[2]; // should be "MP"
    uint16_t len; // length of script stored little endian
    char str[]; // data of script
} appended_script_t;

#define APPENDED_SCRIPT ((const appended_script_t*)0x3e000)

void mp_run(void) {
    int stack_dummy;
    stack_top = (char*)&stack_dummy;
    mp_stack_ctrl_init();
    mp_stack_set_limit(1800); // stack is 2k

    // allocate the heap statically in the bss
    static uint32_t heap[9820 / 4];
    gc_init(heap, (uint8_t*)heap + sizeof(heap));

    /*
    // allocate the heap using system malloc
    extern void *malloc(int);
    void *mheap = malloc(2000);
    gc_init(mheap, (byte*)mheap + 2000);
    */

    /*
    // allocate the heap statically (will clash with BLE)
    gc_init((void*)0x20000100, (void*)0x20002000);
    */

    mp_init();
    mp_hal_init();
    readline_init0();
    microbit_init();

    if (APPENDED_SCRIPT->header[0] == 'M' && APPENDED_SCRIPT->header[1] == 'P') {
        // run appended script
        do_strn(APPENDED_SCRIPT->str, APPENDED_SCRIPT->len);
    } else if (pyexec_mode_kind == PYEXEC_MODE_FRIENDLY_REPL) {
        // from microbit import *
        mp_import_all(mp_import_name(MP_QSTR_microbit, mp_const_empty_tuple, MP_OBJ_NEW_SMALL_INT(0)));
    }

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

    mp_hal_stdout_tx_str("soft reboot\r\n");

    memset(&MP_STATE_PORT(async_data)[0], 0, sizeof(MP_STATE_PORT(async_data)));

    mp_deinit();
}

void gc_collect(void) {
    // WARNING: This gc_collect implementation doesn't try to get root
    // pointers from CPU registers, and thus may function incorrectly.
    void *dummy;
    gc_collect_start();
    gc_collect_root(&dummy, ((mp_uint_t)stack_top - (mp_uint_t)&dummy) / sizeof(mp_uint_t));
    gc_collect_end();
}

mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    (void)filename;
    return NULL;
}

mp_import_stat_t mp_import_stat(const char *path) {
    (void)path;
    return MP_IMPORT_STAT_NO_EXIST;
}

mp_obj_t mp_builtin_open(uint n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    (void)n_args;
    (void)args;
    (void)kwargs;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);

void nlr_jump_fail(void *val) {
    (void)val;
}
