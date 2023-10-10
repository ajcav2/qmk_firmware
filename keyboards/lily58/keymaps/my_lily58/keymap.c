#include QMK_KEYBOARD_H
#include "quantum.h"

#ifdef PROTOCOL_LUFA
  #include "lufa.h"
  #include "split_util.h"
#endif
#ifdef SSD1306OLED
  #include "ssd1306.h"
#endif

#ifdef RGBLIGHT_ENABLE
//Following line allows macro to read current RGB settings
extern rgblight_config_t rgblight_config;
#endif

// To flash: sudo make lily58:my_lily58:avrdude, then double click reset.
// Flash each side
// Vibration pin is D4

extern uint8_t is_master;

#define _QWERTY 0
#define _RAISE 1

enum custom_keycodes {
  QWERTY = SAFE_RANGE,
  LOWER,
  RAISE,
};

enum {
  BRACK = 0,
};

// Used to hide a number of keypresses from being displayed on the oled
int anon_keys = 0;

qk_tap_dance_action_t tap_dance_actions[] = {
[BRACK]   = ACTION_TAP_DANCE_DOUBLE(KC_LBRC, KC_RBRC)
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

/* QWERTY
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * | ESC  |   1  |   2  |   3  |   4  |   5  |                    |   6  |   7  |   8  |   9  |   0  |BackSP|
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  |   Q  |   W  |   E  |   R  |   T  |                    |   Y  |   U  |   I  |   O  |   P  |  \   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |LCtrl |   A  |   S  |   D  |   F  |   G  |-------.    ,-------|   H  |   J  |   K  |   L  |   ;  |   '  |
 * |------+------+------+------+------+------|   [   |    |    ]  |------+------+------+------+------+------|
 * |LShift|   Z  |   X  |   C  |   V  |   B  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *                   | LAlt | LGUI | DEL  | /Space  /       \Enter \  |   -  |   `  |  =   |
 *                   |      |      |      |/(RAISE)/         \(LOWER)\ |      |      |      |
 *                   `----------------------------'           '------''--------------------'
 */

 [_QWERTY] = LAYOUT( \
  KC_ESC,         KC_1,   KC_2,    KC_3,    KC_4,    KC_5,                               KC_6,    KC_7,    KC_8,    KC_9,    KC_0,     KC_BSPC, \
  KC_TAB,         KC_Q,   KC_W,    KC_E,    KC_R,    KC_T,                               KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,     KC_BSLS, \
  LCTL_T(KC_INS), KC_A,   KC_S,    KC_D,    KC_F,    KC_G,                               KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN,  KC_QUOT, \
  SFT_T(KC_HOME), KC_Z,   KC_X,    KC_C,    KC_V,    KC_B, TD(BRACK),        KC_RBRC,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,  RSFT_T(KC_END), \
                   KC_LALT, KC_LGUI,  KC_DEL, LT(_RAISE, KC_SPC),   RCTL_T(KC_ENT), KC_MINUS,KC_GRV,  KC_EQUAL \
),

/* RAISE
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |      |  F1  |  F2  |  F3  |  F4  |  F5  |                    |  F6  |  F7  |  F8  |  F9  |  F10 |  Del |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |      | Home |  Up  | End  |      |      |                    |      |      |      |      |      |  Ins |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |      | Left | Down |Right |      |      |-------.    ,-------|      |      |      |      |      |      |
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * |      |      |      |      |      |      |-------|    |-------|      |      |      |      |      |      |
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *                   |      |      |      | /       /       \      \  |  F5  |  F10 | F11  |
 *                   |      |      |      |/       /         \      \ |      |      |      |
 *                   `----------------------------'           '------''--------------------'
 */
[_RAISE] = LAYOUT( \
  _______,   KC_F1,       KC_F2,   KC_F3,           KC_F4,  KC_F5,                         KC_F6, KC_F7, KC_F8, KC_F9,  KC_F10,   KC_DEL, \
  _______, LCTL(KC_LEFT), KC_UP,   LCTL(KC_RIGHT),  DM_REC1, DM_PLY1,                      KC_7,  KC_8,  KC_9, _______, KC_EQUAL, KC_INS, \
  _______, KC_LEFT,       KC_DOWN, KC_RIGHT,        DM_REC2, DM_PLY2,                      KC_4,  KC_5,  KC_6, _______, KC_BTN1,  KC_BTN2, \
  _______, _______,       _______, _______,         DM_RSTP, _______,   KC_RBRC, _______,  KC_1,  KC_2,  KC_3, _______, _______,  _______, \
                                           _______, _______, _______,   _______,  KC_0  ,  KC_F5, KC_F10, KC_F11 \
)
};

int RGB_current_mode;

// Setting ADJUST layer RGB back to default
void update_tri_layer_RGB(uint8_t layer1, uint8_t layer2, uint8_t layer3) {
  if (IS_LAYER_ON(layer1) && IS_LAYER_ON(layer2)) {
    layer_on(layer3);
  } else {
    layer_off(layer3);
  }
}

void keyboard_post_init_user(void) {
    setPinOutput(D4);
    writePinLow(D4);
}

static uint32_t vol_timer;
void encoder_update_user(uint8_t index, bool clockwise) {
  if (index == 1) {
    if (clockwise) {
      tap_code(KC_VOLD);
    } else {
      tap_code(KC_VOLU);
    }
    vol_timer = timer_read32();
  }
}

uint16_t prev_state = 0;
static uint32_t layer_timer;
void matrix_scan_user(void) {
  if (layer_state != prev_state && prev_state == 0) {
    layer_timer = timer_read32();
  }
  if (timer_elapsed32(layer_timer) < 175 || timer_elapsed32(vol_timer) < 75) {
    writePinHigh(D4);
  } else {
    writePinLow(D4);
  }
  prev_state = layer_state;
}

uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case RCTL_T(KC_SPC):
            return 300;
        case RALT_T(KC_EQUAL):
            return 300;
        default:
            return TAPPING_TERM;
    }
}

//SSD1306 OLED update loop, make sure to enable OLED_DRIVER_ENABLE=yes in rules.mk
#ifdef OLED_DRIVER_ENABLE

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
  if (!is_keyboard_master())
    return OLED_ROTATION_180;  // flips the display 180 degrees if offhand
  return rotation;
}

// When you add source files to SRC in rules.mk, you can use functions.
const char *read_layer_state(void);
const char *read_logo(void);
void set_keylog(uint16_t keycode, keyrecord_t *record);
const char *read_keylog(void);
const char *read_keylogs(void);

// const char *read_mode_icon(bool swap);
// const char *read_host_led_state(void);
// void set_timelog(void);
// const char *read_timelog(void);

void oled_task_user(void) {
  if (is_keyboard_master()) {
    // If you want to change the display of OLED, you need to change here
    oled_write_ln(read_layer_state(), false);
    oled_write_ln(read_keylog(), false);
    oled_write_ln(read_keylogs(), false);
    //oled_write_ln(read_mode_icon(keymap_config.swap_lalt_lgui), false);
    //oled_write_ln(read_host_led_state(), false);
    // oled_write_ln(read_timelog(), false);
    
    // oled_set_cursor(0, 3);
    // static char l4l[11] = {0};
    // snprintf(l4l, sizeof(l4l), "wpm: %d", get_current_wpm());
    // oled_write(l4l, false);
  } else {
    oled_write(read_logo(), false);
  }
}
#endif // OLED_DRIVER_ENABLE

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  if (record->event.pressed) {
#ifdef OLED_DRIVER_ENABLE
    if (anon_keys <= 0) {
      set_keylog(keycode, record);
    }
#endif
    // set_timelog();
  }

  if (anon_keys > 0) {
    anon_keys--;
  }
  switch (keycode) {
    case QWERTY:
      if (record->event.pressed) {
        set_single_persistent_default_layer(_QWERTY);
      }
      return false;
      break;
    // case LOWER:
    //   if (record->event.pressed) {
    //     layer_on(_LOWER);
    //     update_tri_layer_RGB(_LOWER, _RAISE, _ADJUST);
    //   } else {
    //     layer_off(_LOWER);
    //     update_tri_layer_RGB(_LOWER, _RAISE, _ADJUST);
    //   }
    //   return false;
    //   break;
    case RAISE:
      if (record->event.pressed) {
        layer_on(_RAISE);
        anon_keys = 40;
        writePinHigh(D4);
        // update_tri_layer_RGB(_RAISE);
      } else {
        layer_off(_RAISE);
        // update_tri_layer_RGB(_RAISE);
      }
      return false;
      break;
    // case ADJUST:
    //     if (record->event.pressed) {
    //       layer_on(_ADJUST);
    //     } else {
    //       layer_off(_ADJUST);
    //     }
    //     return false;
    //     break;
  }
  return true;
}
