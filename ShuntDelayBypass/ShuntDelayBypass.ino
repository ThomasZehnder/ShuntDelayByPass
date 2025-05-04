/*
Git HUB Repo
https://github.com/ThomasZehnder/ShuntDelayByPass

# Funktionsbeschreibung
 Einschalt Verzögerung um den Shunt Widerstand von 4Ω zu überbrücken.
 Der Shunt begrenzt den Strom auf ca. 8A bei 24V

##  ✅ Ablauf
### Laden
 * Nache dem Einschalten wird 1 Sekunden geladen. --> langsam blinken
### Warten auf Spannung
 * Danach wird 3 Sekunden lang überprüft, ob die Spannund erreicht wurde. Falls ja, wird der Schunt überbrückt, das Relais zieht an. --> schnelles blinken mit 200ms Periode.
 * Falls nach 3 Sekunden die Spannung nicht erreicht wurde, wird das Relais auch geschaltet.
### Relais ein
* Die Relaisspannung wird nach 500ms auf 50% Haltespannung  gesenkt werden.
### Relai auf Haltespannung
 * Falls die Spannung unter der Referenzspannung liegt blinkt die LED mit 500ms Perione, sonst mit 2s.
 * Falls die Spannung auf unter 80% der Haltespannung fällt, wird wieder versucht das Relais voll einzuschalten.


## 💡  Programmieren der Referenzspannung
Den Schalter betätigen. Beim Loslassen wird die gemessene Spannung intern gespeichert und von dann an als Referenzspannung verwendet.

Die abgespeicherte Referenzspannung entspricht 90% der gemessenen Spannung.

(Bei gedrückter Taste wird das Relais ausschaltet. Es blinkt schnell. )

*/

/*
Spannungsteiler

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

//define USE_PWM for RElais, to reduce powerconsumtion
//#define USE_PWM

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

uint16_t scaleInt16Percent(uint16_t val, uint16_t factor)
{
  return (uint16_t)((uint32_t)val * factor / 100);
}

// global defined
uint16_t vBat = 0;
uint16_t vRef = 700;

// UBAT > UREF (POTI)
bool fUbatOk(void)
{
  vBat = analogRead((analog_pin_t)UBATTTERY_PIN);
  return (vBat > vRef);
}

bool fUbatCritical(void)
{
  vBat = analogRead((analog_pin_t)UBATTTERY_PIN);
  return (vBat < scaleInt16Percent(vRef, 88)); // 80% = 90% * 88%, weil vRef bereits um 90% reduziert wurde
}

bool fUbatRestart(void)
{
  vBat = analogRead((analog_pin_t)UBATTTERY_PIN);
  return (vBat < scaleInt16Percent(vRef, 50)); 
}

// used for debugging built in led works inverse :-)
#define POLARITY_LED 0

void blink(int16_t pulsrateHigh, int16_t pulsrateLow)
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
    blink(250, 250);
    // Wait Time
    if (getNow() > 1000)
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
    blink(100, 100);
    // UBAT > UREF (POTI)
    if (fUbatOk())
    {
      state = 3;
    }
    // Wait Time
    if (getNow() > 3000)
    {
      state = 3;
    }
  }
  else if (state == 2)
  { // Warten auf loslassen der Programmiertaste
    blink(200, 50);

    if (digitalRead(KEY_PIN) == HIGH)
    {
      resetStateMachine();
      // call function to get aktual vBat
      fUbatOk();
      // flash voltage as new reference
      saveReference(scaleInt16Percent(vBat, 90)); // save limit on 90% of messured value
    }
  }
  else if (state == 3)
  {                              // Realay zieht an, nach einer 500ms reduzieren auf 50%
    analogWrite(RELAY_PIN, 255); // switch relais on, shunt is forced off
    blink(250, 250);             // blink sequens for -Relais ON Time  Delay
    #ifdef USE_PWM
    analogWrite(RELAY_PIN, 128); // Haltespannung Reduzieren auf 50% = 128 auf der sicheren Seite  (75 von 255 = 3.5V für 12V Relais reicht nicht, mit 50% zieht das Relais aber auch nicht mehr an, und es prifft leicht mit der PWM Frequenz)
    #endif
    state = 4;
  }
  else
  { // Relais geschaltet, Anzeigen der Spannung mit Frequenz des Blinkens

    // UBAT > UREF (POTI & Programmiert)
    if (fUbatOk())
    {
      blink(1950, 50);
    }
    else
    {
      blink(450, 50);
    }

    // Spannung Programmieren oder Unterspannung
    if ((digitalRead(KEY_PIN) == LOW) || (fUbatRestart()))
    {
      resetStateMachine();
      analogWrite(RELAY_PIN, 0); // switch relais off
    }

    // Haltespannung kritisch, Relais voll einschalten
    if (fUbatCritical())
    {
      state = 3;
    }
  }
}
