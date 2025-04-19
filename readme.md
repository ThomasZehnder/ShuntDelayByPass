# Funktionsbeschreibung
 Einschalt Verzögerung um den Shunt Widerstand von 5Ω zu überbrücken.
 Der Shunt begrenzt den Strom auf ca. 5A

 ## Ablauf
 * Nache dem Einschalten wird 5 Sekunden geladen. --> langsam blinken 
 * Danach wird 15 Sekunden lang überprüft, ob die Spannund erreicht wurde. Falls ja, wird der Schunt überbrückt, das Relais zieht an. --> schnelles blinken
 * Falls nach 15 Sekunden die Spannung nicht erreicht wurde, wird das Relais auch geschaltet. --> Puls mit langer Pause (2s)


# 🧷 ATtiny13 Pinbelegung

| Pin Nr. | AVR Pin | Deine Belegung      | Funktion                                       |
|--------:|:--------|:--------------------|:-----------------------------------------------|
| 1       | PB5     | Reset (Standard)    | Standardmäßig Reset                            |
| 2       | PB3     | `UREFERNCE_PIN`     | Analog Input (ADC3) – Referenz Spannung  (POTI)| 
| 3       | PB4     | `UBATTTERY_PIN`     | Analog Input (ADC2) – Batteriespannung messen  |            
| 4       | GND     | -                   | Masse                                          |
| 5       | PB0     | `KEY_PIN`           | Digital Input (Taster) optional nur für Tests schaltet PullDown auf Null. |
| 6       | PB1     | `RELAY_PIN`         | Digital Output (Relais)                        | 
| 7       | PB2     | `LED_PIN`           | Digital Output (LED) --> schaltet auf Null     |
| 8       | Vcc     | +                   | Betriebsspannung (5 V)                         |



# Pinlayout
                   +---\/---+
    RESET         –|1      8|– Vcc (5V)
    UREFERENCE_PIN–|2      7|– LED_PIN
    UBATTERY_PIN  –|3      6|– RELAY_PIN
    GND           –|4      5|– KEY_PIN (unbenutzt / PB0)
                   +--------+

    
    #define LED_PIN PB2
    #define RELAY_PIN PB1
    #define KEY_PIN PB0

    #define UBATTTERY_PIN PB4
    #define UREFERNCE_PIN PB3

# Spannungsteiler Spannungsmessung 
    [ 30V ] ── R1 (100kΩ) ──┬──> To ADC pin (PB4)
                            |
                           R2 (20kΩ)
                            |
                           GND

Wir nehmen deinen Spannungsteiler:

    R1 = 100 kΩ
    R2 = 20 kΩ

Vcc = 5 V → also ADC-Werte von 0 bis 1023

### 🧮 Schritt 1: Ausgangsspannung berechnen
Die Spannung am ADC-Pin ist:

𝑉out = 22𝑉×(𝑅1/(𝑅1+𝑅2)) = 22𝑉×(20𝑘Ω/120𝑘Ω)= 22𝑉×1/6 ≈ 3.67𝑉

### 🧮 Schritt 2: Potentiometer einstellen
 Auf eine Spannung von 3.67V stellen.



### ✅ Antwort:
Wenn 22 V anliegen, wird der ATtiny13 einen analogRead()-Wert von etwa 751 liefern.

## 🔧 IRLB8721-Anschluss am ATtiny13 (PB1)

**Ansicht von vorne (Pins unten):**
   IRLB8721 (Frontansicht, Pins unten)

    Gate   Drain   Source
     |       |       |
     |       |       +--→ GND
     |       |
     |       +--→ Relais → +24 V
     |
     +--→ 150 Ω in Serie mit PB1
     |
     +--→ 100 kΩ nach GND
     
**Beschreibung:**
- **Gate**: Wird über einen **150 Ω** Vorwiderstand von **PB1** angesteuert  
  – zusätzlich ein **100 kΩ Pull-Down**-Widerstand nach Masse  
- **Drain**: Schaltet das **Relais** gegen **+24 V Versorgung**  
- **Source**: Direkt mit **GND** verbunden  

Freilaufdione nicht vergessen
