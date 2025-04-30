/*
Git HUB Repo
https://github.com/ThomasZehnder/ShuntDelayByPass

# Funktionsbeschreibung
 Einschalt Verzögerung um den Shunt Widerstand von 5Ω zu überbrücken.
 Der Shunt begrenzt den Strom auf ca. 5A

##  ✅ Ablauf
 * Nache dem Einschalten wird 5 Sekunden geladen. --> langsam blinken
 * Danach wird 15 Sekunden lang überprüft, ob die Spannund erreicht wurde. Falls ja, wird der Schunt überbrückt, das Relais zieht an. --> schnelles blinken mit 200ms Periode.
 * Falls nach 15 Sekunden die Spannung nicht erreicht wurde, wird das Relais auch geschaltet.
 * Falls die Spannung unter der Referenzspannung liegt blinkt die LED mit 500ms Perione, sonst mit 2s.
 * Die Relaisspannung wird nach 500ms auf 50% Haltespannung gesenkt werden.


## ✅ Programmieren der Referenzspannung
Den Schalter betätigen. Beim Loslassen wird die gemessene Spannung intern gespeichert und von dann an als Referenzspannung verwendet. (Bei gedrückter Taste wird das Relais nicht geschaltet. Es blinkt schnell. )
*/

/*
    [ 30V ] ── R1 (100kΩ) ──┬──> To ADC pin (PB4)
                            |
                           R2 (20kΩ)
                            |
                           GND

*/

#include <EEPROM.h>

#define LED_PIN PB2
#define RELAY_PIN PB1
#define KEY_PIN PB0

#define UBATTTERY_PIN PB3

// the setup function runs once when you press reset or power the board
void setup()
{
  // initialize digital pin as an output.
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);   // switch relais off, shunt activ
  pinMode(KEY_PIN, INPUT_PULLUP); // programmiertaste
  analogReference(DEFAULT);       // Only option on ATtiny13 == 1024 = VCC
}

int16_t pulsrateHigh = 20;
int16_t pulsrateLow = 20;

// global defined
uint16_t vBat = 0;
uint16_t vRef = 700;

// UBAT > UREF (POTI)
bool fUbatOk(void)
{
  vBat = analogRead((analog_pin_t)UBATTTERY_PIN);
  return (vBat > vRef);
}

bool fUbatLow(void)
{
  vBat = analogRead((analog_pin_t)UBATTTERY_PIN);
  return (vBat < vRef / 2);
}

// used for debugging built in led works inverse :-9
#define POLARITY_LED 0

void blink(void)
{
  // blink led
  digitalWrite(LED_PIN, HIGH ^ POLARITY_LED); // turn the LED on
  delay(pulsrateHigh);
  digitalWrite(LED_PIN, LOW ^ POLARITY_LED); // turn the LED off
  delay(pulsrateLow);
}

void saveReference(uint16_t value)
{
  EEPROM.write(0, lowByte(value));
  EEPROM.write(1, highByte(value));
}

uint16_t loadReference()
{
  return ((uint16_t)EEPROM.read(1) << 8) | EEPROM.read(0);
}

byte state = 0;
unsigned long resetTime = 0;

void resetStateMachine(void)
{
  state = 0;
  resetTime = (micros() / 1000);
}

unsigned long getNow(void)
{
  return (micros() / 1000) - resetTime;
}

// the loop function runs over and over again forever
void loop()
{

  if (state == 0)
  { // Warten auf Zeit oder "Programmierttaste"
    pulsrateHigh = 250;
    pulsrateLow = 250;
    blink();
    // Wait Time
    if (getNow() > 5000)
    {
      state = 1;
      vRef = loadReference();
    }
    if (digitalRead(KEY_PIN) == LOW)
    {
      state = 2;
    }
  }
  else if (state == 1)
  { // Warten auf Zeit oder Spannung
    pulsrateHigh = 100;
    pulsrateLow = 100;
    blink();
    // UBAT > UREF (POTI)
    if (fUbatOk())
    {
      state = 3;
    }
    // Wait Time
    if (getNow() > 10000)
    {
      state = 3;
    }
  }
  else if (state == 2)
  { // Warten auf loslassen der Programmiertaste
    pulsrateHigh = 200;
    pulsrateLow = 50;
    blink();

    if (digitalRead(KEY_PIN) == HIGH)
    {
      resetStateMachine();
      // call function to get aktual vBat
      fUbatOk();
      // flash voltage as new reference
      saveReference(vBat);
    }
  }
  else if (state == 3)
  { // Realay zieht an, nach einer 500ms reduzieren auf 50%
    pulsrateHigh = 250;
    pulsrateLow = 250;
    analogWrite(RELAY_PIN, 255); // switch relais on, shunt is forced off
    blink();                     // blink sequens for -Relais ON Time  Delay
    analogWrite(RELAY_PIN, 128); // Haltespannung Reduzieren auf 50% = 128 auf der sicheren seite  (75 von 255 = 3.5V für 12V Relais reicht nicht)
    state = 4;
  }
  else
  { // Relais geschaltet, Anzeigen der Spannung mit Frequenz des Blinkens

    // UBAT > UREF (POTI & Programmiert)
    if (fUbatOk())
    {
      pulsrateHigh = 1950;
      pulsrateLow = 50;
    }
    else
    {
      pulsrateHigh = 450;
      pulsrateLow = 50;
    }

    blink();

    // Spannung Programmieren oder Unterspannung
    if ((digitalRead(KEY_PIN) == LOW) || (fUbatLow()))
    {
      resetStateMachine();
      analogWrite(RELAY_PIN, 0); // switch relais off
    }
  }
}
