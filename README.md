
Fuzzy Text International Student & Teacher Version
========================
This is heavily based on Fuzzy Text International from Jesse Hallett and
others 

Bug fixes and additions to the original work (details see CHANGES)

Additions (optional):
- shows battery status
- shows delta between fuzzy and real time (real - fuzzy)
- Timetable for lessons
- shows amount of time which has passed / left in lesson
  or start of next lesson
- warns / vibrates 5 minutes before next lesson


-----------------------------------------------------
original README.md

This is a watchface for the [Pebble][].  It is originally based on the
[PebbleTextWatch][] by Mihai Dumitrache, which reproduced the look of
the Text Watch that comes standard with the Pebble.

[Pebble]: https://getpebble.com/
[PebbleTextWatch]: https://github.com/wearewip/PebbleTextWatch

Mattias Bäcklund created a modified version, [Swedish fuzzy text
watch][], that displays fuzzy time.  Mattias wanted to combine the
elegant layout and animations of the Text Watch with the natural
language of the Fuzzy Time watchface, and wanted it in his native
language, Swedish.

[Swedish fuzzy text watch]: https://github.com/Sarastro72/Swedish-Fuzzy-Text-watch

This version builds upon the work by Mihai and Mattias: it supports
multiple languages, and provides options to change the visual style.

Features:

 - Fuzzy time in natural language
 - The large and easy to read fonts of the original Text Watch
 - Nice staggered animation
 - Between one and four lines of text, depending on need
 - Smaller words may share a single line (such as "fem i")

The following options can be configured, using the Pebble app on your
phone:

- Invert colors (white-on-black or black-on-white)
- Text alignment (centered, left, or right)
- Language

At this time the included languages are:

- English
- French
- German
- Norwegian
- Spanish
- Swedish


Authors
-------

Thanks to all of the people who made this watchface possible:

- [Mihai Dumitrache][Mihai], implemented an open source version of Text Watch
- [Mattias Bäcklund][Mattias], created Swedish fuzzy text watch
- [Jesse Hallett][Jesse], added configuration options and multiple language support
- [Filip Horvei][iFlips], provided Norwegian translation
- Tomi De Lucca, discovered fix for a severe iOS bug & assisted with Spanish translation

[Mihai]: https://github.com/mmdumi
[Mattias]: https://github.com/Sarastro72
[Jesse]: https://github.com/hallettj
[iFlips]: https://github.com/iFlips


Contributing
------------

If you would like to request a translation, provide a translation, or
point out errors in a translation, please [open an issue][issue].

[issue]: https://github.com/hallettj/Fuzzy-Text-International/issues/new

For an example of what is needed for translations, take a look at
[`strings-en.c`][en].  In case you want to implement a translation
yourself, look at [818e076][es] to see all of the code changes that are
necessary to do so.

[en]: https://github.com/hallettj/Fuzzy-Text-International/blob/master/src/strings-en.c
[es]: https://github.com/hallettj/Fuzzy-Text-International/commit/818e07686761adc00245986f6d389076534a5c1a

Please feel free to open issues for matters other than translations!
Pull requests are welcome as well.
