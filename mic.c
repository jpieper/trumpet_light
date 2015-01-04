/** Copyright 2015 Josh Pieper, jjp@pobox.com.  All rights reserved. */

#include "mic.h"

#include <avr/io.h>

static int32_t g_mic_filter;
static int16_t g_mic_last_value;

static int32_t g_mic_running_power;
static int32_t g_mic_last_power;
static int32_t g_mic_filtered_power;
static int32_t g_mic_max_power;
static int16_t g_cycle_count;

void mic_init(void) {
  // Enable the PLL.
  PLLCSR = (1 << PCKE) | (1 << PLLE);

  // Configure Timer 1 for PWM out.

  TCCR1A = (1 << COM1B1) | (1 << PWM1B); // Fast PWM COM1B connected (OCR1B)
  TCCR1B = 0x1; // CK/1
  TCCR1D = 0x00; // fast PWM

  // Configure the ADC.
  ADMUX = 0x01;
  ADCSRB = (1 << MUX5) | (1 << GSEL) | (1 << BIN); // PA0/PA1 8x
  ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (1 << ADPS2); // start it up w/ CK/16

  // 8000000/16/13.5 ~= 37k samples per second

  // Initialize the filter with the first reading.
  while (!(ADCSRA & (1 << ADIF)));

  uint16_t initial_unsigned = ADC;
  int16_t initial =
      (initial_unsigned < 0x200) ? initial_unsigned :
      (((int16_t) initial_unsigned) - 0x400);
  g_mic_filter = initial * 32;
}

void mic_poll(void) {
  if (!(ADCSRA & (1 << ADIF))) { return; }

  uint16_t current_unsigned = ADC;
  int16_t current =
      (current_unsigned < 0x200) ? current_unsigned :
      (((int16_t) current_unsigned) - 0x400);

  g_mic_filter += current;
  g_mic_filter = (g_mic_filter * 32 / 33);

  int16_t average = g_mic_filter / 32;

  g_mic_last_value = current - average;

  g_mic_running_power += g_mic_last_value * g_mic_last_value;
}

void mic_timer_update(void) {
  g_mic_last_power = g_mic_running_power;
  g_mic_running_power = 0;

  g_mic_filtered_power += g_mic_last_power;
  g_mic_filtered_power = (g_mic_filtered_power * 16 / 17);

  g_cycle_count++;
  if (g_cycle_count > 1000) {
    g_mic_max_power = g_mic_max_power * 31 / 32;
    if (g_mic_max_power < 0x1000) {
      g_mic_max_power = 0x1000;
    }
    g_cycle_count = 0;
  }

  if (g_mic_filtered_power > g_mic_max_power) {
    g_mic_max_power = g_mic_filtered_power;
  }

  int32_t power = g_mic_filtered_power;
  if (power < 0x100) {
    power = 0;
  } else {
    power -= 0x100;
  }
  uint32_t ratio = 300 * power / g_mic_max_power;
  if (ratio > 255) { ratio = 255; }
  if (OCR1B != 0 && ratio == 0) {
    g_mic_filtered_power = 0;
  }
  OCR1B = ratio;
}

int32_t mic_last_power(void) {
  return g_mic_filtered_power;
}

int32_t mic_max_power(void) {
  return g_mic_max_power;
}

int16_t mic_last_value(void) {
  return g_mic_last_value;
}
