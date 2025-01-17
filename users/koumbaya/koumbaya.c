/*
Copyright 2017 Christopher Courtney <drashna@live.com> @drashna

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "koumbaya.h"

userspace_config_t userspace_config;
#if (defined(UNICODE_ENABLE) || defined(UNICODEMAP_ENABLE) || defined(UCIS_ENABLE))
#    define DRASHNA_UNICODE_MODE UC_WIN
#else
// set to 2 for UC_WIN, set to 4 for UC_WINC
#    define DRASHNA_UNICODE_MODE 2
#endif



bool mod_key_press_timer(uint16_t code, uint16_t mod_code, bool pressed) {
    static uint16_t this_timer;
    if (pressed) {
        this_timer = timer_read();
    } else {
        if (timer_elapsed(this_timer) < TAPPING_TERM) {
            tap_code(code);
        } else {
            register_code(mod_code);
            tap_code(code);
            unregister_code(mod_code);
        }
    }
    return false;
}

bool mod_key_press(uint16_t code, uint16_t mod_code, bool pressed, uint16_t this_timer) {
    if (pressed) {
        this_timer = timer_read();
    } else {
        if (timer_elapsed(this_timer) < TAPPING_TERM) {
            tap_code(code);
        } else {
            register_code(mod_code);
            tap_code(code);
            unregister_code(mod_code);
        }
    }
    return false;
}

void bootmagic_lite(void) {
    matrix_scan();
#if defined(DEBOUNCING_DELAY) && DEBOUNCING_DELAY > 0
    wait_ms(DEBOUNCING_DELAY * 2);
#elif defined(DEBOUNCE) && DEBOUNCE > 0
    wait_ms(DEBOUNCE * 2);
#else
    wait_ms(30);
#endif
    matrix_scan();
    if (matrix_get_row(BOOTMAGIC_LITE_ROW) & (1 << BOOTMAGIC_LITE_COLUMN)) {
        bootloader_jump();
    }
}

// Add reconfigurable functions here, for keymap customization
// This allows for a global, userspace functions, and continued
// customization of the keymap.  Use _keymap instead of _user
// functions in the keymaps
__attribute__((weak))
void matrix_init_keymap(void) {}

// Call user matrix init, set default RGB colors and then
// call the keymap's init function
void matrix_init_user(void) {
    userspace_config.raw = eeconfig_read_user();

#ifdef BOOTLOADER_CATERINA
    DDRD &= ~(1 << 5);
    PORTD &= ~(1 << 5);

    DDRB &= ~(1 << 0);
    PORTB &= ~(1 << 0);
#endif

#if (defined(UNICODE_ENABLE) || defined(UNICODEMAP_ENABLE) || defined(UCIS_ENABLE))
    set_unicode_input_mode(DRASHNA_UNICODE_MODE);
    get_unicode_input_mode();
#endif  // UNICODE_ENABLE
    matrix_init_keymap();
}

__attribute__((weak))
void keyboard_post_init_keymap(void) {}

void keyboard_post_init_user(void) {
#if defined(RGBLIGHT_ENABLE) || defined(RGB_MATRIX_ENABLE)
    keyboard_post_init_rgb();
#endif
    keyboard_post_init_keymap();
}

__attribute__((weak))
void shutdown_keymap(void) {}

 void rgb_matrix_update_pwm_buffers(void);

void shutdown_user(void) {
#ifdef RGBLIGHT_ENABLE
    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(1);
    rgblight_setrgb_red();
#endif  // RGBLIGHT_ENABLE
#ifdef RGB_MATRIX_ENABLE
    rgb_matrix_set_color_all( 0xFF, 0x00, 0x00 );
    rgb_matrix_update_pwm_buffers();

#endif  // RGB_MATRIX_ENABLE
    shutdown_keymap();
}

__attribute__((weak))
void suspend_power_down_keymap(void) {}

void suspend_power_down_user(void) {
    suspend_power_down_keymap();
}

__attribute__((weak))
void suspend_wakeup_init_keymap(void) {}

void suspend_wakeup_init_user(void) {
    suspend_wakeup_init_keymap();
}

__attribute__((weak))
void matrix_scan_keymap(void) {}

// No global matrix scan code, so just run keymap's matrix
// scan function
void matrix_scan_user(void) {
    static bool has_ran_yet;
    if (!has_ran_yet) {
        has_ran_yet = true;
        startup_user();
    }

#ifdef TAP_DANCE_ENABLE  // Run Diablo 3 macro checking code.
    run_diablo_macro_check();
#endif  // TAP_DANCE_ENABLE

#if defined(RGBLIGHT_ENABLE) || defined(RGB_MATRIX_ENABLE)
    matrix_scan_rgb();
#endif  // RGBLIGHT_ENABLE

    matrix_scan_keymap();
}

__attribute__((weak))
layer_state_t layer_state_set_keymap(layer_state_t state) { return state; }

// on layer change, no matter where the change was initiated
// Then runs keymap's layer change check
layer_state_t layer_state_set_user(layer_state_t state) {
    state = update_tri_layer_state(state, _RAISE, _LOWER, _ADJUST);
#if defined(RGBLIGHT_ENABLE) || defined(RGB_MATRIX_ENABLE)
    state = layer_state_set_rgb(state);
#endif  // RGBLIGHT_ENABLE
    return layer_state_set_keymap(state);
}

__attribute__((weak))
layer_state_t default_layer_state_set_keymap(layer_state_t state) { return state; }

// Runs state check and changes underglow color and animation
layer_state_t default_layer_state_set_user(layer_state_t state) {
    state = default_layer_state_set_keymap(state);
#if 0
#if defined(RGBLIGHT_ENABLE) || defined(RGB_MATRIX_ENABLE)
  state = default_layer_state_set_rgb(state);
#    endif  // RGBLIGHT_ENABLE
#endif
    return state;
}

__attribute__((weak))
void led_set_keymap(uint8_t usb_led) {}

// Any custom LED code goes here.
// So far, I only have keyboard specific code,
// So nothing goes here.
void led_set_user(uint8_t usb_led) {
    led_set_keymap(usb_led);
}

__attribute__((weak))
void eeconfig_init_keymap(void) {}

void eeconfig_init_user(void) {
    userspace_config.raw              = 0;
    userspace_config.rgb_layer_change = true;
    eeconfig_update_user(userspace_config.raw);
#if (defined(UNICODE_ENABLE) || defined(UNICODEMAP_ENABLE) || defined(UCIS_ENABLE))
    set_unicode_input_mode(DRASHNA_UNICODE_MODE);
    get_unicode_input_mode();
#else
    eeprom_update_byte(EECONFIG_UNICODEMODE, DRASHNA_UNICODE_MODE);
#endif
    eeconfig_init_keymap();
    keyboard_init();
}

bool hasAllBitsInMask(uint8_t value, uint8_t mask) {
    value &= 0xF;
    mask &= 0xF;

    return (value & mask) == mask;
}
