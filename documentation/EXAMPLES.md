# Examples

Explanations currently only in Finnish.

## starttime and endtime

http://smartmet.fmi.fi/avi?&format=json&time=202007010000&param=message,icao,messagetime&icao=EFHK&messagetype=TAF → time = 1.7.2020 00 UTC → palauttaa vain 1.7.2020 00UTC tafin

http://smartmet.fmi.fi/avi?&format=json&time=202007010100&param=message,icao,messagetime&icao=EFHK&messagetype=TAF → time = 1.7.2020 01 UTC → palauttaa vain 1.7.2020 00UTC tafin

http://smartmet.fmi.fi/avi?&format=json&starttime=202007010000&endtime=202007010300&param=message,icao,messagetime&icao=EFHK&messagetype=TAF → starttime = 1.7.2020 00UTC, ja endtime = 03 UTC → palauttaa kaikki tafit, jotka ovat voimassa tuolla välillä, eli tekoaika 30.6. 03 UTC - 1.7. 03 UTC

http://smartmet.fmi.fi/avi?&format=json&starttime=202007010000&endtime=202007010300&validrangemessages=0&param=message,icao,messagetime&icao=EFHK&messagetype=TAF → sama aikaväli kuin edellisessä, mutta lisämääre validrangemessages=0, jolloin palautuu kaikki tafit, jotka on luotu tuolla välillä → palauttaa vain 1.7.2020 03 UTC tafin

http://smartmet.fmi.fi/avi?&format=json&starttime=202006302300&endtime=202007010300&validrangemessages=0&param=message,icao,messagetime&icao=EFHK&messagetype=TAF → sama kuin edellä, mutta starttime on jo 30.6.2020 23 UTC, jolloin mukaan tulee myös 1.7.2020 00 UTC taf

http://smartmet.fmi.fi/avi?bbox=24,60,25,61&messagetype=TAF,GAFOR,METAR&format=debug&param=stationid,name,icao,messagetype,message&maxdistance=50

http://smartmet.fmi.fi/avi?wkt=LINESTRING(24.906+60.315,25.822+66.562,27.413+68.607)&messagetype=METAR,TAF&format=json&param=icao,longitude,latitude,messagetype,route,message,messagetime,messirheading,messageversion&maxdistance=10&precision=3&starttime=201512031000&endtime=201512031200

http://smartmet.fmi.fi/avi?lonlat=26,62&param=icao,latlon&messagetype=METAR&maxdistance=300&numberofstations=3&starttime=2015-12-01T09:00:00&endtime=2015-12-01T12:00:00

## param

Tässä kerrotaan mitä parametreja halutaan palauttaa. Täydellinen lista näistä löytyy avipluginin ohjeesta (sivun yläosassa). Yleisimmin käytettyjä icao, name, lonlat, elevation, message, messagetime, messagevalidfrom, messagevalidto, messagecreated.

http://smartmet.fmi.fi/avi?&format=json&starttime=202007010600&endtime=202007011200&param=name,message,messagecreated&icaos=EFHK&messagetype=METAR

## Sijainnin valinta

### icao, icaos

Jos haet sanomia monesta paikasta kerralla, käytä icaos. Jos tarvitset vain yhden aseman tietoja, voit käyttää kumpaa vaan (icao tai icaos).

http://smartmet.fmi.fi/avi?&format=json&starttime=202007010000&endtime=202007011200&param=message&icaos=EFHK&messagetype=METAR

http://smartmet.fmi.fi/avi?&format=json&starttime=202007010000&endtime=202007011200&param=message&icaos=EFHK,EFTU&messagetype=METAR

### place,places

Oikean nimen lentoasemalle voi tarkistaa palauttamalla param=name (esim. http://smartmet.fmi.fi/avi?&format=json&starttime=202007010600&endtime=202007011200&param=icao,name&icaos=EFTU,EFTP&messagetype=METAR).

Kun sitten uuteen hakuun laittaa places=[äsken selvitetty name], haku toimii. Alla olevassa linkissä välilyönnit on korvattu "%20" -merkinnällä, mutta ne voivat selaimen osoitekentässä olla myös välilyönteinä.

http://smartmet.fmi.fi/avi?&format=json&starttime=202007010600&endtime=202007011200&param=message,icao,name&places=Turku%20lentoasema,Pirkkala%20Tampere-Pirkkala%20lentoasema&messagetype=METAR

Kätevämpää on kyllä käyttää icao-tunnuksia icaos-kentässä.

### lonlat, latlon, lonlats, latlons

http://smartmet.fmi.fi/avi?&format=json&starttime=202006220600&endtime=202007011200&latlon=60,25&maxdistance=50&param=latlon,message&messagetype=SIGMET

Huom! Tämä esimerkki palauttaa myös sigmetin koordinaatit, jolloin huomataan, että kaikki ovat Helsingissä. Sigmetin latlon-arvo ei siis ole se, mihin sigmet on annettu, vaan paikka, jossa se on tehty. Tällä ei siis voi hakea tietylle alueelle annettuja varoituksia. Koordinaattien kanssa käytetään yhdessä maxdistance-parametriä, joka kertoo etäisyyden annetusta pisteestä kilometreinä.

### stationid,stationids

Aseman stationid:n saa palautettua, kun hakee param=stationid. Helpompi kyllä käyttää icao-koodia.

http://smartmet.fmi.fi/avi?&format=json&starttime=202006220600&endtime=202007011200&param=stationid,icao&stationids=7,26&messagetype=METAR

### bbox

bbox määrittää koordinaattilaatikon, jonka sisältä sanomat haetaan. Tässä esimerkissä 23E, 60N - 25E, 62N.

Tämän kanssa käytetään yhdessä maxdistance-parametriä, joka kertoo etäisyyden annetusta alueesta kilometreinä. Alla on esimerkkihakuja samalla laatikolla, mutta eri etäisyyksillä.

http://smartmet.fmi.fi/avi?&format=json&starttime=202007010800&endtime=202007011200&param=icao,message&bbox=23,60,25,62&messagetype=METAR&maxdistance=10 → EFHA,EFHK,EFTP

http://smartmet.fmi.fi/avi?&format=json&starttime=202007010800&endtime=202007011200&param=icao,message&bbox=23,60,25,62&messagetype=METAR&maxdistance=50 → EFHA,EFHK,EFTP,EFTU

http://smartmet.fmi.fi/avi?&format=json&starttime=202007010800&endtime=202007011200&param=icao,message&bbox=23,60,25,62&messagetype=METAR&maxdistance=100 → EETN, EFHA,EFHK,EFJY,EFPO,EFSI,EFTP,EFTU

### country,countries

esim. FI, SE, EE, LT, LV, RU,DK,NO

http://smartmet.fmi.fi/avi?&format=json&starttime=202006200000&endtime=202007011200&param=message&country=FI&messagetype=SIGMET

http://smartmet.fmi.fi/avi?&format=json&starttime=202006200000&endtime=202007011200&param=message&countries=NO,DK,DE&messagetype=SIGMET

### maxdistance

Käytetään yhdessä koordinaattien tai bboxin kanssa.

### numberofstations

Määrittää kuinka monen (lähimmän) aseman tiedot näytetään, tässä kaksi esimerkkiä, ensimmäisessä on 3 asemaa, toisessa 5. Haetut koordinaatit 65,25 osuvat Hailuodon itäpuolelle, merkattu kuvaan tähdellä.

http://smartmet.fmi.fi/avi?&format=json&starttime=202007010800&endtime=202007011200&param=icao,message&latlon=65,25&messagetype=METAR&maxdistance=500&numberofstations=3 → EFKE. EFOU,ESPA

http://smartmet.fmi.fi/avi?&format=json&starttime=202007010800&endtime=202007011200&param=icao,message&latlon=65,25&messagetype=METAR&maxdistance=500&numberofstations=5 → EFKE, EFKI, EFKK, EFOU, ESPA

## messagetype

### METAR/SPECI

http://smartmet.fmi.fi/avi?&format=json&starttime=202007010800&endtime=202007011200&param=icao,message&icao=EFHK&messagetype=METAR

### AWSMETAR

http://smartmet.fmi.fi/avi?&format=json&starttime=202007010800&endtime=202007011200&param=icao,message&icao=ILZD&messagetype=AWSMETAR

### TAF

http://smartmet.fmi.fi/avi?&format=json&starttime=202007010800&endtime=202007011200&param=icao,message&icao=EFHK&messagetype=TAF

### GAFOR

http://smartmet.fmi.fi/avi?&format=json&starttime=201807010800&endtime=201807011200&param=message&country=FI&messagetype=GAFOR

### METREP/SPECIAL

http://smartmet.fmi.fi/avi?&format=json&starttime=202007010800&endtime=202007011200&param=message&icao=EFHK&messagetype=SPECIAL

http://smartmet.fmi.fi/avi?&format=json&starttime=202007010800&endtime=202007011200&param=message&icao=EFRO&messagetype=METREP

### WRNG

Huom! icao tai name = EFKL

http://smartmet.fmi.fi/avi?&format=json&starttime=202002010000&endtime=202002011200&param=message,name,icao&country=FI&messagetype=WRNG

### WXREP

Huom! icao tai name = EFKL

http://smartmet.fmi.fi/avi?&format=json&starttime=202002060000&endtime=202002070000&param=message,name,icao,messagetime&country=FI&messagetype=WXREP

### ARS

Huom! icao tai name = EFKL

http://smartmet.fmi.fi/avi?&format=json&starttime=202006060000&endtime=202006300000&param=message,name,icao,messagetime&country=FI&messagetype=ARS

### SIGMET

Huom! icao tai name = EFKL

http://smartmet.fmi.fi/avi?&format=json&starttime=202005060000&endtime=202005300000&param=message,name,icao,messagetime&country=FI&messagetype=SIGMET

### VA-SIGMET

### VAA
