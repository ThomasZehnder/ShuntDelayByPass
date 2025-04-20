/*
 Einschalt Verzögerung um Shunt Widerstand zu überbrücken.

 Wird nach 5 Sekunden geschaltet, falls die Spannung über 22V ist, ansonsten nach 10 Sekunden.

Led Ausgang zeig den Status an.
* 200ms flash Warten auf Spannung
* 100ms flash Spannung erreicht, Relais geschaltet 5s
* 200ms ON, 2s OFF Relais geschaltet, spätestens nach 10 Sekunden

*/

/*
[ 30V ] ── R1 (100kΩ) ──┬──> To ADC pin (PB3)
                        |
                   R2 (20kΩ)
                        |
                      GND

 22V --> 750


*/

#define LED_PIN PB2
#define RELAY_PIN PB1
#define KEY_PIN PB0

#define UBATTTERY_PIN PB4
#define UREFERNCE_PIN PB3


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin as an output.
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // switch relais off, shunt activ
  pinMode(KEY_PIN, INPUT_PULLUP);
  analogReference(0);  // Only option on ATtiny13 == 1024 = VCC
}

int16_t pulsrateHigh = 200;
int16_t pulsrateLow = 200;

int vBat = 0;
int vRef = 0;

// UBAT > UREF (POTI)
bool fUbatOk(void) {
  analogRead((analog_pin_t)UBATTTERY_PIN); // dummy read
  delay(5);                   // stabilisieren
  int vBat = analogRead((analog_pin_t)UBATTTERY_PIN);

  analogRead((analog_pin_t)UREFERNCE_PIN); // dummy read
  delay(5);                   // stabilisieren
  int vRef = analogRead((analog_pin_t)UREFERNCE_PIN);

  return (vBat > vRef);
}


byte state = 0;
unsigned long resetTime = 0;

// the loop function runs over and over again forever
void loop() {

  unsigned long now = micros() / 1000;

  //reset for tests
  if (digitalRead(KEY_PIN) == 0) {
    state = 0;
    digitalWrite(RELAY_PIN, LOW);  // switch relais off, shunt activ
    resetTime = now;
  }

  if (state == 0) {
    pulsrateHigh = 250;
    pulsrateLow = 250;
    // Wait Time
    if (now - resetTime > 5000) {
      state = 1;
    }
  } else if (state == 1) {
    pulsrateHigh = 100;
    pulsrateLow = 100;
    // UBAT > UREF (POTI)
    if (fUbatOk()) {
      state = 3;
    }
    // Wait Time
    if (now - resetTime > 20000) {
      state = 3;
    }

  } else {
    // UBAT > UREF (POTI)
    // UBAT > UREF (POTI)
    if (fUbatOk()) {
      pulsrateHigh = 1950;
      pulsrateLow = 50;
    } else {
      pulsrateHigh = 450;
      pulsrateLow = 50;
    }

    digitalWrite(RELAY_PIN, HIGH);  // switch relais on, shunt is forced
  }

  //blink led
  digitalWrite(LED_PIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(pulsrateHigh);
  digitalWrite(LED_PIN, LOW);  // turn the LED off by making the voltage LOW
  delay(pulsrateLow);
}
