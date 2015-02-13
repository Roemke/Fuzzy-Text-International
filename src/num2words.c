#include "num2words.h"
#include "strings-ca.h"
#include "strings-de.h"
#include "strings-en_GB.h"
#include "strings-en_US.h"
#include "strings-es.h"
#include "strings-fr.h"
#include "strings-no.h"
#include "strings-sv.h"
#include "string.h"


size_t min(const size_t a, const size_t b) {
  return a < b ? a : b;
}

static size_t append_string(char* buffer, const size_t length, const char* str) {
  strncat(buffer, str, length);

  size_t written = strlen(str);
  return (length > written) ? written : length;
}

static size_t interpolate_and_append(char* buffer, const size_t length,
    const char* parent_str, const char* first_placeholder_str, const char* second_placeholder_str) {
  const char* placeholder_str;
  char* insert_ptr = strstr(parent_str, "$1");

  if (insert_ptr) {
    placeholder_str = first_placeholder_str;
  }
  else {
    insert_ptr = strstr(parent_str, "$2");
    placeholder_str = second_placeholder_str;
  }

  size_t parent_len = strlen(parent_str);
  size_t insert_offset = insert_ptr ? (size_t) insert_ptr - (size_t) parent_str : parent_len;

  size_t remaining = length;

  remaining -= append_string(buffer, min(insert_offset, remaining), parent_str);
  remaining -= append_string(buffer, remaining, placeholder_str);
  if (insert_ptr) {
    remaining -= append_string(buffer, remaining, insert_ptr + 2);
  }

  return remaining;
}

const char* get_hour(Language lang, int index) {
  switch (lang) {
    case CA:
      return HOURS_CA[index];
      break;
    case DE:
      return HOURS_DE[index];
      break;
    case EN_GB:
      return HOURS_EN_GB[index];
      break;
    case ES:
      return HOURS_ES[index];
      break;
    case FR:
      return HOURS_FR[index];
      break;
    case NO:
      return HOURS_NO[index];
      break;
    case SV:
      return HOURS_SV[index];
      break;
    default:
      return HOURS_EN_US[index];
  }
}

const char* get_rel(Language lang, int index) {
  switch (lang) {
    case CA:
      return RELS_CA[index];
      break;
    case DE:
      return RELS_DE[index];
      break;
    case EN_GB:
      return RELS_EN_GB[index];
      break;
    case ES:
      return RELS_ES[index];
      break;
    case FR:
      return RELS_FR[index];
      break;
    case NO:
      return RELS_NO[index];
      break;
    case SV:
      return RELS_SV[index];
      break;
    default:
      return RELS_EN_US[index];
  }
}

//(kr) 2015-02-12: returns now difference to real time
//positiv means: real time is n minutes more 
int time_to_words(Language lang, int hours, int minutes, int seconds, char* words, size_t buffer_size) {

  size_t remaining = buffer_size;
  memset(words, 0, buffer_size);
  // We want to operate with a resolution of 30 seconds.  So multiply
  // minutes and seconds by 2.  Then divide by (2 * 5) to carve the hour
  // into five minute intervals.
  //hmm, seconds should be near zero, cause the function is called from update
  //and this is  bind to a handler with MINUTE_UNIT
  //nearly :-) when the first time is displayed it's not zero after that it is
  int half_mins  = (2 * minutes) + (seconds / 30);
  int rel_index =  ((half_mins + 5) / (2 * 5)) % 12;
  int delta;
  int hour_index;

  if (rel_index == 0 && minutes > 30) { //short before full hour
    hour_index = (hours + 1) % 24;
    delta = half_mins/2 - 60 ; //negative means: behind fuzzy time
  }
  else {
    hour_index = hours % 24;
    delta = half_mins/2 - rel_index*5; 
  }

  const char* hour = get_hour(lang, hour_index);
  const char* next_hour = get_hour(lang, (hour_index + 1) % 24);
  const char* rel  = get_rel(lang, rel_index);
  
  //check seconds
  //sprintf(tempBuffer,"%i",seconds); hihi, gibts fuer pebble nicht
  //char tempBuffer[64];
  //mini_itoa(delta,10,1,tempBuffer,0);
  //const char * rel = tempBuffer;
  
  remaining -= interpolate_and_append(words, remaining, rel, hour, next_hour);

  // Leave one space at the end
  remaining -= append_string(words, remaining, " ");

  return delta;
}
