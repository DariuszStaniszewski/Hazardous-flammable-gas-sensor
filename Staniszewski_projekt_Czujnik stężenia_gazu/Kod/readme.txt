Projekt powstały na płytce FRDM-KL05Z z użyciem czujnika stężenia gazów palnych lub dymu modMQ-2.

Wszystkie wartości napięć jakim odpowiada dany próg ppm dla butanu oraz reakcje układu.
poniżej 0,49 V Poziom Normalny zielony led, brak syreny
od 0,49 do 1,18 V Poziom > 100ppm żółty led, brak syreny
od 1,18 do 2,1 V Poziom > 1 000 ppm czerwony led, syrena co 400ms
od 2,1 do 5 V Poziom > 10 000 ppm żółty i czerwony led, syrena co 200ms

Połączenia:
LCD przez I2C: SDA (I2C ) – SDA (płytka)
SCL (I2C ) – SCL (płytka)
Czujnik: A0 Output (czujnik) – PTA0 (płytka)
LED R (czerwony) PTB 11
LED Y (żółty) PTB 6
LED G (zielony) PTB 7
BUZZER PTA 11

Działanie układu:
https://youtu.be/rxTESSsUlcw
