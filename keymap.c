#include QMK_KEYBOARD_H

#include "oneshot.h"
#include "swapper.h"

#define HOME G(KC_LEFT)
#define END G(KC_RGHT)
#define FWD G(KC_RBRC)
#define BACK G(KC_LBRC)
#define TABL G(S(KC_LBRC))
#define TABR G(S(KC_RBRC))
#define SPCL A(G(KC_LEFT))
#define SPC_R A(G(KC_RGHT))
#define UNDO G(KC_Z)
#define REDO G(S(KC_Z))
#define CUT G(KC_X)
#define COPY G(KC_C)
#define PASTE G(KC_V)
#define LA_SYM MO(SYM)
#define LA_NAV MO(NAV)
#define LA_FUN MO(FUN)

enum layers {
    DEF,
    SYM,
    NAV,
    NUM,
    FUN,
};

enum keycodes {
    // Custom oneshot mod implementation with no timers.
    OS_SHFT = SAFE_RANGE,
    OS_CTRL,
    OS_ALT,
    OS_CMD,

    SW_WIN,  // Switch to next window         (cmd-tab)
    SW_LANG, // Switch to next input language (ctl-spc)
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [DEF] = LAYOUT_split_3x6_3(
        XXXXXXX, KC_Q,    KC_W,    KC_F,    KC_P,    KC_B,    /**/  KC_J,    KC_L,    KC_U,    KC_Y,    KC_QUOT,  XXXXXXX,
        XXXXXXX, KC_A,    KC_R,    KC_S,    KC_T,    KC_G,    /**/  KC_M,    KC_N,    KC_E,    KC_I,    KC_O,     XXXXXXX,
        XXXXXXX, KC_Z,    KC_X,    KC_C,    KC_D,    KC_V,    /**/  KC_K,    KC_H,    KC_COMM, KC_DOT,  KC_SLSH,  XXXXXXX,
                                   LA_NAV,  KC_LSFT, LA_FUN,  /**/  KC_BSPC, KC_SPC,  LA_SYM
    ),

    [SYM] = LAYOUT_split_3x6_3(
        XXXXXXX, KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC, /**/  KC_ASTR, KC_LPRN, KC_RPRN, KC_MINS, KC_EQL,   XXXXXXX,
        XXXXXXX, OS_SHFT, OS_CTRL, OS_ALT,  OS_CMD,  KC_CIRC, /**/  KC_AMPR, KC_LBRC, KC_RBRC, KC_GRV,  KC_SCLN,  XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, KC_PLUS, KC_UNDS, KC_COLN,  /**/  KC_PIPE, KC_LCBR, KC_RCBR, KC_TILD, KC_BSLS,  XXXXXXX,
                                   _______, _______, _______, /**/  _______, _______, _______
    ),

    [NAV] = LAYOUT_split_3x6_3(
        XXXXXXX, KC_ESC,  KC_TAB,  SW_WIN,  XXXXXXX, XXXXXXX, /**/  KC_PGUP, HOME,    KC_UP,   END,     XXXXXXX, XXXXXXX,
        XXXXXXX, OS_SHFT, OS_CTRL, OS_ALT,  OS_CMD,  XXXXXXX, /**/  KC_PGDN, KC_LEFT, KC_DOWN, KC_RGHT, XXXXXXX,  XXXXXXX,
        XXXXXXX, UNDO,    CUT,     COPY,    REDO,    PASTE,   /**/  KC_CAPS, XXXXXXX, XXXXXXX, XXXXXXX, KC_ENT,  XXXXXXX,
                                   _______, _______, _______, /**/  KC_DEL,  _______, _______
    ),

    [NUM] = LAYOUT_split_3x6_3(
        XXXXXXX, KC_PLUS, KC_MINS, KC_ASTR, KC_SLASH, KC_EQL, /**/  XXXXXXX,  KC_7,  KC_8,  KC_9, XXXXXXX,   XXXXXXX,
        XXXXXXX, OS_SHFT, OS_CTRL, OS_ALT,  OS_CMD,   XXXXXXX,/**/  XXXXXXX, KC_4,  KC_5,  KC_6, XXXXXXX,   XXXXXXX,
        XXXXXXX, UNDO,    CUT,     COPY,    REDO,     PASTE,  /**/  KC_0,    KC_1,  KC_2,  KC_3, XXXXXXX,   XXXXXXX,
                                   _______, _______,  QK_BOOT, /**/  _______, _______, _______
    ),

    [FUN] = LAYOUT_split_3x6_3(
        XXXXXXX, KC_MSTP, KC_MPRV, KC_MNXT, KC_MPLY, KC_VOLU, /**/  XXXXXXX,  KC_F9,  KC_F10, KC_F11, KC_F12,  XXXXXXX,
        XXXXXXX, OS_SHFT, OS_CTRL, OS_ALT,  OS_CMD,  KC_VOLD, /**/  XXXXXXX,  KC_F5,  KC_F6,  KC_F7,  KC_F8,   XXXXXXX,
        XXXXXXX, UNDO,    CUT,     COPY,    REDO,    PASTE,   /**/  XXXXXXX,  KC_F1,  KC_F2,  KC_F3,  KC_F4,   XXXXXXX,
                                   _______, _______, _______, /**/  _______, _______, _______
    ),
};

bool is_oneshot_cancel_key(uint16_t keycode) {
    switch (keycode) {
    case LA_SYM:
    case LA_NAV:
        return true;
    default:
        return false;
    }
}

bool is_oneshot_ignored_key(uint16_t keycode) {
    switch (keycode) {
    case LA_SYM:
    case LA_NAV:
    case KC_LSFT:
    case OS_SHFT:
    case OS_CTRL:
    case OS_ALT:
    case OS_CMD:
        return true;
    default:
        return false;
    }
}

bool sw_win_active = false;
bool sw_lang_active = false;

oneshot_state os_shft_state = os_up_unqueued;
oneshot_state os_ctrl_state = os_up_unqueued;
oneshot_state os_alt_state = os_up_unqueued;
oneshot_state os_cmd_state = os_up_unqueued;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    update_swapper(
        &sw_win_active, KC_LGUI, KC_TAB, SW_WIN,
        keycode, record
    );
    update_swapper(
        &sw_lang_active, KC_LCTL, KC_SPC, SW_LANG,
        keycode, record
    );

    update_oneshot(
        &os_shft_state, KC_LSFT, OS_SHFT,
        keycode, record
    );
    update_oneshot(
        &os_ctrl_state, KC_LCTL, OS_CTRL,
        keycode, record
    );
    update_oneshot(
        &os_alt_state, KC_LALT, OS_ALT,
        keycode, record
    );
    update_oneshot(
        &os_cmd_state, KC_LCMD, OS_CMD,
        keycode, record
    );

    return true;
}

layer_state_t layer_state_set_user(layer_state_t state) {
    return update_tri_layer_state(state, SYM, NAV, NUM);
}
