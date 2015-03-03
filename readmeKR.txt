personal notes cause it's my first modification to a pebble 
watchface and I have to refresh my c and git knowledge :-)

suche dauernd struct tm aufbau: 
http://www.cplusplus.com/reference/ctime/tm/

------------------------
2015-02-11
git repo geforked, s. dort fuer quelle (auf meinem git-Account online)
git nach buch konfiguriert (git-scm.com/book/de)
clonen: git clone https://github.com/Roemke/Fuzzy-Text-International.git
-> Verzeichnis wird erstellt und inhalte abgeglichen
Versuche das Teil erstmal zum laufen zu bringen
--------
entwickler verbindung auf pebble app eingestellt
ip setzen pebble install --phone 192.168.2.48
oder export PEBBLE_PHONE=192.168.2.48
habe bei der installation was uebersehen:
    cd ~/pebble-dev/PebbleSDK-2.8.1
    virtualenv --no-site-packages .env
    source .env/bin/activate
    pip install -r requirements.txt
    deactivate

pebble install --phone 192.168.2.48 
-> dennoch fehler
kann aber auch sein, dass die app erst fertig sein muss

make geht aber auch nicht - fehler
so läuft es leider nicht
evtl. ist makefile auch uralt und wscript ist das richtige

also pebble build -> fehler
sudo npm install -g jshint

dann korrektur   "versionLabel": "1.3",
in appinfo.json (statt 1.2.1 - nicht erlaubt)
in src/num2words.h fehlt ein , - strange (war pull request)

pebble install läuft - aber woher weiss er wohin?
(vermute hatte das phone noch in der environment variable)

wie dem auch sei, auf dem smartphone taucht die app auf
und lässt sich über einstellungen konfigurieren
(web-interface)

also bis hier läuft es - schön, gleiche jetzt ab
-----------------------------------------------
2015-02-12 first commit
 also git add und dann git commit
 mein remote repo:
 roemke@kspace42:~/pebble-dev/Fuzzy-Text-International$ git remote -v 
 origin  https://github.com/Roemke/Fuzzy-Text-International.git (fetch)
 origin  https://github.com/Roemke/Fuzzy-Text-International.git (push)

 logging ist interessant - s APP_LOG in Doku (geht direkt auf notebook
 bei Aufruf von pebble logs, formatierung möglich
 formatierung von zahlen snprintf statt sprintf 
 typisch also 
 pebble build
 pebble install --phone 192.168.2.48
 pebble logs --phone 192.168.2.48

 breite ist laut angabe 144 - bei 142  (bei zwanzig erreicht)
 gibt es aber Stress - dumm gelaufen.
 animation scheint auch korrekt - hmm seltsam

 debug 1 hat fehler (click geschichte anscheinend nicht ok)
 außerdem wird appinfo.json geaendert werden muessen
 (hmm, nicht sicher, doch ich denke schon)
 geschehen, noch stress mit der Zeit, 
 
 delta zur realen Zeit eingebaut, aber noch icht verwendet.

2015-02-13
  einige kleinigkeiten bearbeitet, debug funktionalitaet funktioniert jetzt
  komplett, Analyse der Aufrufe um die Zeilen darzustellen:
doku:
function calls for updating lines
eingerückt -> call in der Funktion
static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
  static void display_time(struct tm *t)
    
    //(kr) delta is realTime - roundedTime (rounded: 5 minutes interval)
    int delta = time_to_lines(t->tm_hour, t->tm_min, t->tm_sec, textLine,format);
    
    //(kr) hier wird text übergeben, font gesetzt,  aber Text nicht gesetzt
    int nextNLines = configureLayersForText(textLine, format); 
    //(kr) warum funktionalität nicht in der Routine unten umgesetzt, nur
    //wenn update noetig?
    for each line which need to change (or if number of new lines is not equal
                                       number of lines)
        updateLineTo(&lines[i], textLine[i], delay);
           updateLayerText(line->nextLayer, value); //hier wird der text gesetzt
           makeAnimationsForLayer(line, delay);
           // then: Swap current/next layers

bevor ich aenderungen am ablauf vornehme mal ein commit
git add -A 
git commit

nochmal angeschaut: eine Aenderung am Ablauf ist nicht sinnvoll, da ich 
alle layer kleiner machen möchte, wenn ein Layer nicht passt, 
mit ausnahme der bold-face gesetzten layer
auch wenn man es vielleicht optimieren könnte - lasse den ablauf genau so,
versuche statt dessen einen temporaeren textlayer zu nutzen, um die 
groesse bzw. breite des textes zu finden und die layer entsprechend 
zu konfigurieren -> also erstmal 
git checkout -- src/TextWatch.c
(aenderungen weg)
bin diesen weg gegangen, laeuft so weit, habe noch zwei variablen eingebaut
um einen font-Wechsel zu detektieren
-------------------------------------------------
2015-02-14 anschauen, wie man optionen setzen kann 
  
  javascript teil gehört zur app, wird auf dem smartphone in einer sandbox
	ausgeführt. Beispiel nutzt jquery - nett, das das geht, Erweiterung um
  schaltbares delta ist trivial
  uups wohl doch nochmal schauen, es gibt noch eine js und die html wird
	nicht angezeigt, wenn ich einfach nur installiere, aenderungen fehlen
  sieht so aus, als wuerde die Seite nicht an die app auf dem phone
	uebergeben sondern in js steht, das eine Seite im Web gerufen wird
  hmm, strange, steht aber auch so in der doku
  zu hoffen ist dass die pebble app die Seite zwischenspeichert
  nee, ich glaube nicht
  evtl. bietet sich auch
	http://developer.getpebble.com/blog/2013/11/21/Using-PebbleKit-JS-Configuration/
  an - config selbst bauen. Da ich hier aber auf einem Projekt aufbaue 
  werde ich die gegebene anpassen

	auf c Seite scheint es die tuplet geschichte zu sein (AppMessages)
  aufruf von app_sync_init setzt die handler (hhm de_init unnötig? nein
  passiert in window_unload)
  gespeichert lokal in einem persist_storage, laden in handle_init
  - auch erledigt fuer delta

  textlayer fuer delta bauen (ohne Animation) positionierung und groesse
  mit ui designer in der pebble cloud
  
2015-02-17 ein wenig gespielt, einige optionen dazu viel an javascript fuer
  die timetable getan
  lange gebraucht: buffer probleme wenn zu viele Optionen, hatte
   app_messag_open übersehen, dort war der buffer auch auf 64 gesetzt, 
   der war ebenfalls zu klein, maximale groessen wurden mit 
   max outbox 656 max inbox 2026
   im log ausgegeben 

Pebble SDK provides a static minimum guaranteed size
(APP_MESSAGE_INBOX_SIZE_MINIMUM and APP_MESSAGE_OUTBOX_SIZE_MINIMUM).
Requesting a buffer of the minimum guaranteed size (or smaller) is always
guaranteed to succeed on all Pebbles in this SDK version or higher, and with
every phone.

In some context, Pebble might be able to provide your application with
larger inbox/outbox. You can call app_message_inbox_size_maximum() and
app_message_outbox_size_maximum() in your code to get the largest possible
value you can use.

hmmm, ich habe den Eindruck, dass ich von der einfachen variante mit 
AppSync wechseln muss auf AppMessage - kommunikation ist flexibler

vorher mal eine kopie einer halbwegs funktionierenden kommunikation machen
- geschehen, prinzipiell laeuft das watchface, nicht natuerlich die 
timetable geschichte

ok, kopie erstellt ohne app-synch, läuft ebenfalls problemlos
nehme jetzt die timetable wieder dazu, versuche als ganzes zu senden,
da von javascript aus ca. 2kB auf einen Rutsch gesendet werden können.
Alternative: 
http://forums.getpebble.com/discussion/11403/how-do-send-many-app-messages-in-a-loop-to-the-watch
nicht noetig klappt, speichere als linked list

ein paar verbesserungen, scheint jetzt zu laufen

2015-02-21
timetable so weit ok ohne persistenz 
Fehlerkorrekturen, weekend modus mit Hand schuetteln
optionen done und warnown umgesetzt

schiebe mal in's git und dann auch auf github
git add -A
git commit
git push origin master

screenshots mit pebble screenshot 
letzten commit aendern git --amend
-> vorsicht, damit gibt es manchmal stress, da
ein diverge von lokal und remote moeglich sind. 

etwas gespielt, dann einige commits "gemerged"
http://stackoverflow.com/questions/2563632/how-can-i-merge-two-commits-into-one

hatte dann wieder die Abweichung, da niemand an "meinem" remote etwas
aendern kann: 
git push -f origin master

2015-03-01 geht soweit alles, uhr laeuft, moechte die sprache
 konfigurierbar machen 
 bereits einige aenderungen vorgenommen
 git checkout -b workOnLanguage
 git commit -a 

 git branch zeigt die branches an


 umsetzung macht stress, bekomme die Hours und Rels der custom language
 nicht uebertragen anscheinend von configure zur js-App unklar
 Problem liegt im ' von o'clock Ich kann das durch ein x oder so ersetzen
 Vorschlag %27 klappt nicht (hatte ich gefunden)
 habe jetzt einfac |27 genommen - | verboten, anders nicht hinbekommen
 auf c-seite fehlt noch die Verwendung der Teile - ok, das ist relativ wenig
 Array von char * der groesse 24 bzw 12 genommen

 Aerger beim laufen lassen, falls ein ü im Text steht und der ausgegeben
 wird
  roemke@kspace42:~/pebble-dev/PebbleSDK-2.9$ joe tools/pebblecomm/pebble.py
 muss geaendert werden ?
 log.info("{} {} {}:{}> {}".format(str_level, timestamp_str,filename, linenumber, message.encode('utf-8')))
 .encode('utf-8') ergaenzt 
http://forums.getpebble.com/discussion/8582/bug-pebble-logs-fails-with-ascii-codec-cant-encode-character-u-xfc-in-position
 hat nichts genutzt ist aber auch nur ein debugging fehler
Noch ein fehler gefunden, falls sprache auf 0 ist, gebe standard-string
zurueck, auch wenn Sprachauswahl auf cl (custom language) steht
na, ob das jetzt fehlerfrei ist?

git geschichte
git checkout master
git checkout -b version1.3 (jetzt haben wir drei branches)
git checkout workOnLanguage (textdateien noch etwas anpassen)
git commit -a --amend
git branch
leads to 
roemke@kspace42:~/pebble-dev/Fuzzy-Text-International-TimeTable$ git branch
  master
  version1.3
* workOnLanguage

git checkout master
git merge workOnLanguage
sollte ein fast-forward geben, da nur der zeiger weiter bewegt werden muss
nein hat er nicht getan
 
