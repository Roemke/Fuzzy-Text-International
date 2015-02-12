#pragma once
#include "string.h"

typedef enum {
  CA    = 0x0,
  DE    = 0x1,
  EN_GB = 0x2,
  EN_US = 0x3,
  ES    = 0x4,
  FR    = 0x5,
  NO    = 0x6,
  SV    = 0x7
} Language;

void time_to_words(Language lang, int hours, int minutes, int seconds, char* words, size_t length);
