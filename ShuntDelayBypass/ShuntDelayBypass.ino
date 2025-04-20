/*
# Funktionsbeschreibung
 Einschalt Verzögerung um den Shunt Widerstand von 5Ω zu überbrücken.
 Der Shunt begrenzt den Strom auf ca. 5A

##  ✅ Ablauf
 * Nache dem Einschalten wird 5 Sekunden geladen. --> langsam blinken 
 * Danach wird 15 Sekunden lang überprüft, ob die Spannund erreicht wurde. Falls ja, wird der Schunt überbrückt, das Relais zieht an. --> schnelles blinken
 * Falls nach 15 Sekunden die Spannung nicht erreicht wurde, wird das Relais auch geschaltet. --> Puls mit langer Pause (2s)


## ✅ Programmieren
Vor dem Einschalten der Batterie  oder während den ersten 5 Sekunden den Schalter betätigen. Beim Loslassen wird die gemessene Spannung intern gespeichert und von dann an als Referenzspannung verwendet. (Bei gedrückter Taste wird das Relais nicht geschaltet. Es blinkt mit kurzen "Blitzen")

*/

/*
    [ 30V ] ── R1 (100kΩ) ──┬──> To ADC pin (PB4)
                            |
                           R2 (20kΩ)
                            |
                           GND

*/

#define LED_PIN PB2
#define RELAY_PIN PB1
#define KEY_PIN PB0

#define UBATTTERY_PIN PB3

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin as an output.
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);    // switch relais off, shunt activ
  pinMode(KEY_PIN, INPUT_PULLUP);  // programmiertaste
  analogReference(DEFAULT);        // Only option on ATtiny13 == 1024 = VCC
}

int16_t pulsrateHigh = 20;
int16_t pulsrateLow = 20;

//global defined
int vBat = 0;
int vRef = 700;

// UBAT > UREF (POTI)
bool fUbatOk(void) {
  vBat = analogRead((analog_pin_t)UBATTTERY_PIN);
  return (vBat > vRef);
}

#define POLARITY_LED 1

void blink(void) {
  //blink led
  digitalWrite(LED_PIN, HIGH ^ POLARITY_LED);  // turn the LED on
  delay(pulsrateHigh);
  digitalWrite(LED_PIN, LOW ^ POLARITY_LED);  // turn the LED off
  delay(pulsrateLow);
}


byte state = 0;
unsigned long resetTime = 0;

// the loop function runs over and over again forever
void loop() {

  unsigned long now = (micros() / 1000) - resetTime;

  if (state == 0) {  //Warten auf Zeit oder "Programmierttaste"
    pulsrateHigh = 250;
    pulsrateLow = 250;
    blink();
    // Wait Time
    if (now > 5000) {
      state = 1;
    }
    if (digitalRead(KEY_PIN) == LOW) {
      state = 2;
    }

  } else if (state == 1) {  //Warten auf Zeit oder Spannung
    pulsrateHigh = 100;
    pulsrateLow = 100;
    blink();
    // UBAT > UREF (POTI)
    if (fUbatOk()) {
      state = 3;
    }
    // Wait Time
    if (now > 20000) {
      state = 3;
    }

  } else if (state == 2) {  //Warten auf loslassen der Programmiertaste
    pulsrateHigh = 200;
    pulsrateLow = 50;
    blink();

    if (digitalRead(KEY_PIN) == HIGH) {
      state = 0;
      resetTime = now;  //reset start time
    }

  } else {  //Relais geschaltet, anzeigen der Spannung mit Frequenz des Blinkens

    // UBAT > UREF (POTI)
    if (fUbatOk()) {
      pulsrateHigh = 1950;
      pulsrateLow = 50;
    } else {
      pulsrateHigh = 450;
      pulsrateLow = 50;
    }

    digitalWrite(RELAY_PIN, HIGH);  // switch relais on, shunt is forced off

    blink();

    if (digitalRead(KEY_PIN) == LOW) {
      state = 0;
      resetTime = now;  //reset start time
       digitalWrite(RELAY_PIN, LOW);  // switch relais off
    }
  }
}
