#include "strings-de.h"

const char* const HOURS_DE[] = {
  // AM hours
  "zwölf",
  "ein", //noch ein problem eins <-> ein, evtl. einfach im code beruecksichtigen
  "zwei",
  "drei",
  "vier",
  "fünf",
  "sechs",
  "sieben",
  "acht",
  "neun",
  "zehn",
  "elf",

  // PM hours
  "zwölf",
  "ein",
  "zwei",
  "drei",
  "vier",
  "fünf",
  "sechs",
  "sieben",
  "acht",
  "neun",
  "zehn",
  "elf"
};

/**
 * The string "$1" will be replaced with the current hour (e.g., "three"
 * at 3:45).  The string "$2" will be replaced with the *next* hour
 * (e.g., "four" at 3:45).
 *
 * A "*" character before a word makes that word bold.
 */
const char* const RELS_DE[] = {
  "*$1 Uhr",
  "fünf nach *$1",
  "zehn nach *$1",
  "viertel nach *$1",
  "zwanzig nach *$1",
  //"fünfund- zwanzig nach *$1",
  "fünf vor halb *$2",
  "halb *$2",
  //"fünfund- zwanzig vor *$2",
  "fünf nach halb *$2",
  "zwanzig vor *$2",
  "viertel vor *$2",
  "zehn vor *$2",
  "fünf vor *$2"
}; 

