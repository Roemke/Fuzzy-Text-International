[Go back][back]
[back]: javascript:history.back()

Changes
========

The latest version is 1.3.  To update, remove the watchface from your
watch and then load it again from your locker.

The iOS version has to go through the Apple Store approval process - so
there will be a delay before iOS users are able to download the latest
version.  Wait for the Pebble app to update, then try updating the
watchface. (original comment)

Android users do not need to wait for the Pebble app to update before
updating the watchface.

version 1.3
--------------
Start version of "my app". Based heavily on the original work. I needed an
example to look how it works and after that I have some ideas to fit the 
app to my needs. So it becomes an extended version called Student and Teachers
version. 
 - fixed bug in num2words.h (done)
 - appinfo.json version is not ok (only two numbers not three)
 - Text in German is adapted ein / eins and fuenf vor halb instead of 25 nach
   changed German REL text from *$1 Uhr to es ist *$1
   advantage: line three is small in German Display - use this part for Delta
   (see additions)
 - changed positioning of rows 
 - fixed debugging option, remember to use different appinfo.json 
   use debug_time to avoid conflicts with the "normal time" (done)
 - in german zwanzig is not shown correct because it's to wide
   don't understand that because the size of zwanzig is 142 and 144
	 should be possible - nevertheless, if text is to wide I reduce font
	 of all lines which are not bold


version 1.2.1
-------------

- Version 1.2.0 was released with the wrong settings URL; this version
  fixes that.

version 1.2.0
-------------

- Fixes blank settings page on iOS devices.  (Many thanks to Tomi De Lucca!)
- Adds translation, English (Great Britain).
- Fixes for Spanish translation.  (Again, thanks to Tomi De Lucca!)

version 1.1.0
-------------

- Adds Norwegian translation provided by iFlip.

version 1.0.0
-------------

- first release
