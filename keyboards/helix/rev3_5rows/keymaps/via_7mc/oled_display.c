/* Copyright 2023 nanamachi, original code by yushakobo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H

#include <stdio.h>
#include "split_util.h"

// Defines names for use in layer keycodes and the keymap
enum layer_names {
  _DEFAULT = 0,
  _QWERTY,
  _LOWER,
  _UPPER
};

#ifdef OLED_ENABLE

uint16_t RGBLED_STATUS_DELAY = 1000;
uint16_t layer_lower_timer = 0;

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_270;
}

void render_mode_status(void) {
  // Host Keyboard Layer Status
  oled_write_P(PSTR("\n"), false);

  switch (get_highest_layer(layer_state)) {
      case _DEFAULT:
          oled_write_P(PSTR("\n"), false);
          break;
      case _QWERTY:
          oled_write_P(PSTR("QWERT"), true);
          break;
      case _LOWER:
          oled_write_P(PSTR("LWR"), false);
          oled_write_P(PSTR("\x03\x04"), false);
          break;
      case _UPPER:
          oled_write_P(PSTR("\x05\x06"), false);
          oled_write_P(PSTR("UPR"), false);
          break;
      default:
          // Or use the write_ln shortcut over adding '\n' to the end of your string
          oled_write_P(PSTR("undef"), true);
  }

  oled_write_P(PSTR("\n"), false);
}


void render_horizontal_line(void) {
    oled_write_P(PSTR("\n"), false);
    oled_write_P(PSTR("\x1c\x1c\x1c\x1c\x1c"), false);
    oled_write_P(PSTR("\n"), false);
}


void render_lock_status(void) {
  // Host Keyboard LED Status
  led_t led_state = host_keyboard_led_state();
  oled_write_P(led_state.num_lock ? PSTR("NumLk") : PSTR("     "), false);
  oled_write_P(led_state.caps_lock ? PSTR("CpsLk") : PSTR("     "), false);
  oled_write_P(led_state.scroll_lock ? PSTR("ScrLk") : PSTR("     "), false);
}


static void render_logo(bool invert) {
    static const char PROGMEM nanamachi_logo[] = {
        0x80, 0x81, 0x82, 0x83, 0x84,
        0xA0, 0xA1, 0xA2, 0xA3, 0xA4,
        0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0x00
    };

    oled_write_P(nanamachi_logo, invert);
}

static void render_rgbled_status(bool full) {
#ifdef RGBLIGHT_ENABLE
  char buf[24];
  if (RGBLIGHT_MODES > 1 && rgblight_is_enabled()) {
      if (full) {
          snprintf(buf, sizeof(buf), "M| %2dH| %2dS| %2dV| %2d",
                   rgblight_get_mode(),
                   rgblight_get_hue()/RGBLIGHT_HUE_STEP,
                   rgblight_get_sat()/RGBLIGHT_SAT_STEP,
                   rgblight_get_val()/RGBLIGHT_VAL_STEP);
      } else {
          snprintf(buf, sizeof(buf), "[%2d] ", rgblight_get_mode());
      }
  } else {
      snprintf(buf, sizeof(buf), "      LED  OFF      ");
  }
  oled_write(buf, false);
#endif
}

static void render_rgbled_mode(void) {
#ifdef RGBLIGHT_ENABLE
  char buf[24];
    snprintf(buf, sizeof(buf), "Mode %5dSpd  %5d",
        rgblight_get_mode(),
        rgblight_get_speed()
    );
  oled_write(buf, false);
#endif
}

static void render_rgbled_color(void) {
#ifdef RGBLIGHT_ENABLE
  char buf[24];
    snprintf(buf, sizeof(buf), "H| %2dS| %2dV| %2d",
        rgblight_get_hue()/RGBLIGHT_HUE_STEP,
        rgblight_get_sat()/RGBLIGHT_SAT_STEP,
        rgblight_get_val()/RGBLIGHT_VAL_STEP
    );
  oled_write(buf, false);
#endif
}

bool oled_task_user(void) {
  // Render logo for both master and slave
    render_logo(!is_transport_connected());
        render_horizontal_line();

  if(is_keyboard_master()){
    render_mode_status();
        render_horizontal_line();

    if (!is_transport_connected()) {
        // If the keyboard is used as a single keyboard,
        // render RGB LED status on holding Lower key with a delay

        if (get_highest_layer(layer_state) != _LOWER) {
            layer_lower_timer = timer_read() + RGBLED_STATUS_DELAY;
        }

        if (timer_expired(timer_read(), layer_lower_timer)) {
            render_rgbled_status(true);
            return false;
        }
    }
    // Otherwise, render Lock status
    render_lock_status();
    oled_write_P(PSTR("     "), false);

  }else{
    // Render RGB LED status for the slave
    render_rgbled_mode();
        render_horizontal_line();
    render_rgbled_color();
  }
    return false;
}
#endif
