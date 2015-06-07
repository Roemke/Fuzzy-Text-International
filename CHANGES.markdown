[Go back][back]
[back]: javascript:history.back()

Changes
========

The latest version is 1.7  To update, remove the watchface from your
watch and then load it again from your locker.

The iOS version has to go through the Apple Store approval process - so
there will be a delay before iOS users are able to download the latest
version.  Wait for the Pebble app to update, then try updating the
watchface. (original comment, I havn't access to an iphone)

Android users do not need to wait for the Pebble app to update before
updating the watchface.

version 1.7
-------------
Compiled for pebble time with sdk 3  
Problems with actual_time - on "old pebble" the pointer
was persistant and points to a valid time struct. In pebble time
this seems not to work.

Changed the german zwanzig nach into zehn vor halb &ndash; no font
change should be neccessary, will see if I have it right in mind (thanks to 
dersvenhesse) 

For battery status of phone you need still the android app, the sandbox 
of pebble still does not support battery status object.


version 1.6
------------
Changed behaviour of shaking your wrist.
Instead of switching on or off the information according to the time table
I show an different layout with the following information
timetable things (if set)
bar for battery  (if set)
bar for phone battery (if set)
text percent for  phone battery / battery
Date 
Time not fuzzy but exact
shaking wrist can be switched off in configuration

version 1.5
-------------
On Android it is possible to show the battery level of the phone as second
bar. You need a service running on android.
The apk and the sourcecode you will find on github.
[AndroidServerJSON auf github](https://github.com/Roemke/AndroidServerJSON)

In the moment I check every minute if I have no information about 
phone battery level and check every 30 Minutes if I have the information
so the battery bar.

So don't enable this,  if you don't have a service running on your phone
which delivers the battery status cause you will send every minute a 
bluetoothquestion to your phone 

version 1.4.2 
-------------
bug in setting custom language strings
bug in storing the custom language
(both in the web-page configure-fuzzy...)

version 1.4.1
-------------
found a bug in timetable functionality - strange, it was working sometimes
don't know when I inserted this bug ... 

version 1.4
--------------
It's possible to customize an own language. In Settings on phone
you can use Language type custom and by using the CusLa Tab 
you can edit two textareas with your language entries. Added an example of 
bavarian language.

This version is not really tested, I'm still in the process of learning
(ok, will never stop it ...) and I need to handle with some dynamic memory
allocation which always can cause prolems. If you had problems with a 
custom language write an email to me.

version 1.3.1
--------------
Timetable persistant on the watch,
if mobile is connected there should be no difference.

version 1.3
--------------
Start version of "my app". Based heavily on the original work. I needed an
example to look how it works and after that I have some ideas to fit the 
app to my needs. So it becomes an extended version called TimeTable
edition. 

Modifications

- fixed bug in num2words.h (done)
- appinfo.json version is not ok (only two numbers not three), done, added version
  for debugging appinfo.json.watchapp (copy)
- Text in German is adapted ein / eins and fuenf vor halb instead of 25 nach
  changed German REL text from *$1 Uhr to es ist *$1
- changed positioning of rows slightly 
- fixed debugging option, remember to use different appinfo.json 
  use debug_time to avoid conflicts with the "normal time" (done)
- fixed problem of "cut off" zwanzig in german and maybe other languages
  by switching to smaller font if line is wider than 140 px
- rewritten configure-fuzzy-text.html to use jquery mobile and allow to configure timetable
  and new options 
- changed url-location


Additions (configurable)

- Bluetooth state (todo / think I will not do / need it)
- Battery state can be shown as thin line which visualize the percent of battery state
  (full line - 100 %) 
- Show delta minutes to exact time as number  (exact - fuzzy ).
  Number is positioned in the middle of the top line or at the right 
  a *  means no difference between exact and fuzzy  
- TimeTable. It's possible to define a table per day which represents the lessons.
  Eg: I have to hold a lesson on Monday from 9:45 to 11:15 am -> can put it into the timetable
  and you can mark it at your own lesson (see warnown)

  ATTENTION: I haven't put  the timetable into the persistant storage, so if you start the watchface
  and your mobile is not connected there will be now entries in the timetable (I think I will fix it, done 
  in version 1.3.1)

 - Option done: 
   If option done is set the watch will display how much time is done in the lesson (left upper corner)
   and how much time is left in the lesson upper right corner. 
   If your are not in a lessen, in the left upper corner is displayed when the next lesson begins
 - Option warnown: 5 minutes before next own lesson starts, the watch will vibrate. 

 - Weekend mode: tap_event is handled, so if you move the watch fast the done and warnown options will be
	 turned off or on 

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
