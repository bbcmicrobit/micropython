#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "py/nlr.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "pyexec.h"
#include MICROPY_HAL_H

void do_strn(const char *src, size_t len) {
    mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR___main__, src, len, 0);
    if (lex == NULL) {
        printf("MemoryError: lexer could not allocate memory\n");
        return;
    }

    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        qstr source_name = lex->source_name;
        mp_parse_node_t pn = mp_parse(lex, MP_PARSE_FILE_INPUT);
        mp_obj_t module_fun = mp_compile(pn, source_name, MP_EMIT_OPT_NONE, false);
        mp_hal_set_interrupt_char(3); // allow ctrl-C to interrupt us
        mp_call_function_0(module_fun);
        mp_hal_set_interrupt_char(-1); // disable interrupt
        nlr_pop();
    } else {
        // uncaught exception
        mp_hal_set_interrupt_char(-1); // disable interrupt
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
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

    // allocate the heap statically in the bss
    static char heap[9728];
    gc_init(heap, heap + sizeof(heap));

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

    // run appended script if it exists
    if (APPENDED_SCRIPT->header[0] == 'M' && APPENDED_SCRIPT->header[1] == 'P') {
        do_strn(APPENDED_SCRIPT->str, APPENDED_SCRIPT->len);
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
