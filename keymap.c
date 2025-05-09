#include QMK_KEYBOARD_H

#include "oneshot.h"
#include "swapper.h"

#define HOME G(KC_LEFT)
#define END G(KC_RGHT)
#define UNDO G(KC_Z)
#define REDO G(S(KC_Z))
#define CUT G(KC_X)
#define COPY G(KC_C)
#define PASTE G(KC_V)
#define LA_SYM MO(SYM)
#define LA_NAV MO(NAV)
#define LA_FUN MO(FUN)
#define LA_MOU MO(MOU)

enum layers {
    DEF,
    SYM,
    NAV,
    NUM,
    FUN,
    MOU,
};

enum keycodes {
    // Custom oneshot mod implementation with no timers.
    OS_SHFT = SAFE_RANGE,
    OS_CTRL,
    OS_ALT,
    OS_CMD,

    SW_WIN,  // Switch to next window         (cmd-tab)
    SW_LANG, // Switch to next input language (ctl-spc)
    SW_ALT,  // Switch to next window in the same app context (macos behavior) (alt-win)
    SW_CTL, // Switch to next tab in the same window (ctrl-win)
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [DEF] = LAYOUT_split_3x5_3(
        KC_Q,    KC_W,    KC_F,    KC_P,    KC_B,    /**/  KC_J,    KC_L,    KC_U,    KC_Y,    KC_QUOT,
        KC_A,    KC_R,    KC_S,    KC_T,    KC_G,    /**/  KC_M,    KC_N,    KC_E,    KC_I,    KC_O,
        KC_Z,    KC_X,    KC_C,    KC_D,    KC_V,    /**/  KC_K,    KC_H,    KC_COMM, KC_DOT,  KC_SLSH,
                          LA_NAV,  KC_LSFT, LA_FUN,  /**/  LA_MOU,  KC_SPC,  LA_SYM
    ),

    [SYM] = LAYOUT_split_3x5_3(
        KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC, /**/  KC_ASTR, KC_LPRN, KC_RPRN, KC_MINS, KC_EQL,
        OS_SHFT, OS_CTRL, OS_ALT,  OS_CMD,  KC_CIRC, /**/  KC_AMPR, KC_LBRC, KC_RBRC, KC_GRV,  KC_BSLS,
        UNDO,    CUT,     COPY,    REDO,    PASTE,   /**/  KC_PIPE, KC_LCBR, KC_RCBR, KC_COLN, KC_SCLN,
                          _______, _______, _______, /**/  _______, _______, _______
    ),

    [NAV] = LAYOUT_split_3x5_3(
        KC_ESC,  KC_TAB,  XXXXXXX, XXXXXXX, XXXXXXX, /**/  KC_PGUP, HOME,    KC_UP,   END,     KC_BSPC,
        OS_SHFT, OS_CTRL, OS_ALT,  OS_CMD,  XXXXXXX, /**/  KC_PGDN, KC_LEFT, KC_DOWN, KC_RGHT, KC_DEL,
        UNDO,    CUT,     COPY,    REDO,    PASTE,   /**/  KC_CAPS, SW_WIN,  SW_ALT,  SW_CTL, KC_ENT,
                          _______, _______, _______, /**/  _______, _______, _______
    ),

    [NUM] = LAYOUT_split_3x5_3(
        KC_PLUS, KC_MINS, KC_ASTR, KC_SLASH, KC_EQL,  /**/  XXXXXXX, KC_7,  KC_8,  KC_9, KC_BSPC,
        OS_SHFT, OS_CTRL, OS_ALT,  OS_CMD,   XXXXXXX, /**/  KC_0,    KC_4,  KC_5,  KC_6, KC_COMM,
        UNDO,    CUT,     COPY,    REDO,     PASTE,   /**/  XXXXXXX, KC_1,  KC_2,  KC_3, KC_DOT,
                          _______, _______,  QK_BOOT, /**/  EE_CLR, _______, _______
    ),

    [FUN] = LAYOUT_split_3x5_3(
        KC_MSTP, KC_MPRV, KC_MNXT, KC_MPLY, KC_VOLU, /**/  XXXXXXX,  KC_F9,  KC_F10, KC_F11, KC_F12,
        OS_SHFT, OS_CTRL, OS_ALT,  OS_CMD,  KC_VOLD, /**/  XXXXXXX,  KC_F5,  KC_F6,  KC_F7,  KC_F8,
        UNDO,    CUT,     COPY,    REDO,    PASTE,   /**/  XXXXXXX,  KC_F1,  KC_F2,  KC_F3,  KC_F4,
                          _______, _______, _______, /**/  _______, _______, _______
    ),

    [MOU] = LAYOUT_split_3x5_3(
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, /**/  XXXXXXX,  XXXXXXX,  XXXXXXX, RGB_RMOD, DPI_RMOD,
        OS_SHFT, OS_CTRL, OS_ALT,  OS_CMD,  XXXXXXX, /**/  XXXXXXX,  XXXXXXX,  XXXXXXX, RGB_MOD,  DPI_MOD,
        UNDO,    CUT,     COPY,    REDO,    PASTE,   /**/  XXXXXXX,  XXXXXXX,  XXXXXXX, RGB_TOG,  XXXXXXX,
                          KC_BTN1, KC_BTN2, DRGSCRL, /**/  _______, _______, _______
    ),
};

bool is_oneshot_cancel_key(uint16_t keycode) {
    switch (keycode) {
    case LA_MOU:
    case LA_SYM:
    case LA_NAV:
        return true;
    default:
        return false;
    }
}

bool is_oneshot_ignored_key(uint16_t keycode) {
    switch (keycode) {
    case LA_MOU:
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
bool sw_alt_active = false;
bool sw_ctl_active = false;

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
    update_swapper(
        &sw_alt_active, KC_LALT, KC_TAB, SW_ALT,
        keycode, record
    );
    update_swapper(
        &sw_ctl_active, KC_LCTL, KC_TAB, SW_CTL,
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