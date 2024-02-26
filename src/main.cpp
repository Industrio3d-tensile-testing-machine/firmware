#include <Arduino.h>
#include "HX711.h"

/*
  dit progama is geschreven door Bram Bijsterveld
  Versie 0.7 (alfa)
*/

#define bandsnelheid_Serial_to_PC 9600
#define HX711_DT 4                  // data pin voor loadcell
#define HX711_SCK 5                 // clock pin voor loadcell
#define pin_nulpunt_schakelaar 3    // deze pin moet een interupt kunnen hebben
#define pin_noodstop 2              // deze pin moet een interupt kunnen hebben
bool nulpunt_schakelaar_status = 0; // 0 = niet ingedrukt 1 = wel ingedrukt
bool noodstop_status = 0;

HX711 scale;

/*
  debounce word gedaan door dat niet meteen weer op 0 te zetten (zie ook kalibreren_positie())
*/
void nulpunt_schakelaar_interupt()
{
  nulpunt_schakelaar_status = 1;
};

/*
  debounce word gedaan door dat als het geactiveerd word permament aanstaat(in het programa) en moet je eerst de arduino herstarten
*/
void noodstop_interupt()
{
  noodstop_status = 1;
};

/*
  deze functie doet sensors instellen (de schakelaar voor kalibratie en de load cell versterker en de noodstop)
*/
void setup_input()
{
  pinMode(pin_nulpunt_schakelaar, INPUT_PULLUP);                                                        // instellen schakelaar met een Pullup weerstand zodat de knop alleen de verbinding naar 0V hoeft te verbinden
  attachInterrupt(digitalPinToInterrupt(pin_nulpunt_schakelaar), nulpunt_schakelaar_interupt, FALLING); // interupt instellen zodat de schakelaar nooit gemist kan worden (de functie word geroepen als het singiaal van 5v naar 0v gaat)
  pinMode(pin_noodstop, INPUT_PULLUP);                                                                  // instellen schakelaar met een Pullup weerstand zodat de knop alleen de verbinding naar 0V hoeft te verbinden
  attachInterrupt(digitalPinToInterrupt(pin_noodstop), noodstop_interupt, FALLING);                     // interupt instellen zodat de schakelaar nooit gemist kan worden (de functie word geroepen als het singiaal van 5v naar 0v gaat)

  scale.begin(HX711_DT, HX711_SCK);
};

/*
  Hier word de Seriale comucatie over de usb gestart en ingesteld de bandsnelheid ingesteld.
  De arduino wacht tot dat de communicatie verbonden is.
  bij "#define bandsnelheid_Serial_to_PC" kan je de bandsnelheid aanpassen
*/
void setup_commumicatie_PC()
{
  Serial.begin(bandsnelheid_Serial_to_PC); // De verbinding word gestart
  while (!Serial)
  {
    ; // de arduino wacht tot Serial verbinding gemaakt is
  }
  Serial.println(F("Verbinding gemaakt")); // stuurt een bericht naar de PC
  Serial.flush();                          // Wacht totdat de bericht aangekomen is
};

/*
  hier word er comuniceerd naar de stappen motor diver om een stap omhoog te gaan
*/
void stepper_omhoog(){};

/*
  hier word er comuniceerd naar de stappen motor diver om een stap omlaag te gaan
*/
void stepper_omlaag(){};

uint32_t stepper_postitie = 0; // dit is het aantal stappen die gemaakt zijn op de stappen motor
/*
  de functie zet het trekblok tegen de begin aan om te meten waar de nul positie is
*/
void Kalibreren_positie()
{
  for (uint8_t i = 0; i < 20; i++)
  { // hier word de stepper omlaag draait tot dat de schakelaar geactieveerd word, dit doet hij een aantal keren om er zeker van te zijn dat het nul punt is
    delay(1000);
    while (nulpunt_schakelaar_status == 0)
    {                   // herhaal tot dat de schakelaar geactieveerd word
      stepper_omlaag(); // zet de trekblok een stap omlaag
      delay(100);       // wacht om te voorkomen dat de motor te snel gaat (to do een goede delay uitkiezen)
    }
    delay(1000);
    nulpunt_schakelaar_status = 1;
  }
  stepper_postitie = 0;
};
void run_test(){};
void menu_opties(){};
// void manual_set_position(bool draai_richting){};
void manual_set_position(){};

char incomingByte = '?';
uint8_t incomingByte_2 = 0;
uint8_t incomingByte_3 = 0;
uint8_t modus = 254;
bool draai_setting = 0; // 1 = omhoog 2 = omlaag

void menu_opties()
{
  incomingByte_2 = Serial.read();
  if (incomingByte == 0 and incomingByte_2 != 0)
  {
    incomingByte = (char)incomingByte_2;
    incomingByte_2 = 0;
    incomingByte_3 = 0;
  }

  if (incomingByte != 0)
  {

    Serial.print("I received: ");
    Serial.println(incomingByte);

    switch (incomingByte)
    {
    case '?':
      Serial.println(F("T = begin test"));
      Serial.println(F("C = Calibratie"));
      Serial.println(F("M = zelf de positie veranderen"));

      incomingByte = 0;
      incomingByte_2 = 0;
      break;
    case 'T':
      modus = 0;

      incomingByte = 0;
      incomingByte_2 = 0;
      break;
    case 'C':
      modus = 1;

      incomingByte = 0;
      incomingByte_2 = 0;
      break;
    case 'M':
      modus = 2;
      Serial.println(F("omhoog(1) of omlaag(2) :"));
      while (incomingByte_3 == 0)
      {
        incomingByte_3 = Serial.read();
      }
      if (incomingByte_3 == 1)
      {
        draai_setting = 1;
        incomingByte_3 = 0;
      }
      else if (incomingByte_3 == 2)
      {
        draai_setting = 2;
        incomingByte_3 = 0;
      }
      else
      {
        Serial.println(F("legal input value"));
        modus = 254;
        incomingByte_3 = 0;
        break;
      }
      Serial.println(F("type aantal stappen (1-255):"));
      while (incomingByte_3 == 0)
      {
        incomingByte_3 = Serial.read();
      }
      break;
    }
  }
};

ISR(TIMER1_COMPA_vect)
{ // timer1 interrupt 1Hz check for the termal control
  menu_opties();
}

void setup()
{ // dit word een keer gedraaid
  setup_commumicatie_PC();
  Kalibreren_positie();

  cli(); // stop interrupts

  // set timer1 interrupt at 1Hz
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1 = 0;  // initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624; // = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei(); // allow interrupts
}

void loop()
{ // dit is waar de hoofd programa gaat en vooraltijd word gehaalt
  switch (modus)
  {
  case 0: // test loop
    run_test();
    break;
  case 1: // kalibratie loop
    Kalibreren_positie();
    break;
  case 2:
    manual_set_position();
    break;
  default: // do nothing

    break;
  }
}
