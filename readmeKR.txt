personal notes cause it's my first modification to a pebble 
watchface and I have to refresh my c and git knowledge :-)
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
(vielleicht hat der befehl oben doch etwas gebracht)

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



 