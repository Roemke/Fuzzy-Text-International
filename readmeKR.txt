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
 

 breite ist laut angabe 144 - bei 142  (bei zwanzig erreicht)
 gibt es aber Stress - dumm gelaufen.
 animation scheint auch korrekt - hmm seltsam

 debug 1 hat fehler (click geschichte anscheinend nicht ok)
 außerdem wird appinfo.json geaendert werden muessen
 (hmm, nicht sicher, doch ich denke schon)
 geschehen, noch stress mit der Zeit, 
 

2015-02-13
  einige kleinigkeiten bearbeitet, debug funktionalitaet funktioniert jetzt
  komplett, Analyse der Aufrufe um die Zeilen darzustellen:
    IW   tmp (Geändert)
Row 14   Col 3    2:03  Ctrl-K H for help
function calls for updating lines
eingerückt -> call in der Funktion
static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
  static void display_time(struct tm *t)
    
    //(kr) delta is realTime - roundedTime (rounded: 5 minutes interval)
    int delta = time_to_lines(t->tm_hour, t->tm_min, t->tm_sec, textLine,format);
    
    //(kr) hier wird übergeben, font gesetzt,  aber Text nicht gesetzt
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
 
 