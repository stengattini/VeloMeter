


//||||\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\||||\\
//||||****BICYCLE FLOW MEASUREMENTS. SPEED and HEADWAYAS****|||\\
//||||/////////////////////////////////////////////////////||||\\
// Simone Tengattini
//v1_26SEP16

///set analogRead to return much faster
// 16 prescaler
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


////////////////////////////////////////////
//CONSTANT VARIABLE DEFINITION & libraries//
////////////////////////////////////////////

//for SD library
//#include <MemoryFree.h>
#include <SPI.h>
#include <SD.h>

//for LCD library
#include <LiquidCrystal.h>

//for RTC - Real time clock
#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 RTC;

//Others
#define FILE_BASE_NAME "BkSp" // Base name must be six characters or less for short file names 8.3fat. maybe change it to include SITE# (e.g. A), DATE OF TEST (mmdd), etc...e.g. 1506A000.csv
Sd2Card card;
File file; //name of the file'entity' needed e.g. for file.print command
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); //pin for LCD communication in order as (RS, Enable, D4, D5, D6, D7)

////////////////////
//GLOBAL VARIABLES//
///////////////////

// Light Beam Sensors PINS
const unsigned char LBSensorPins[] = {33, 31};
const unsigned char LBSensorPinsTot = 2;

//BUTTON PINS
const unsigned char SelectButtonPin = A0; //button select on the LCD screen

//SD CARD PINS
const uint8_t CS_PIN = 53; //communication Pin between card and Arduino

//threshold HIGH-LOW
const int TSH = 1; // deigital read


//decleared here because arduino varibales have a SCOPE. So if I define them in the setup() they are not recallable in the loop()
const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
char fileName[] = FILE_BASE_NAME "000.csv";

//|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

void setup() {

  ///set analogRead to return much faster
#if FASTADC
  // set prescale to 16
  sbi(ADCSRA, ADPS2) ;
  cbi(ADCSRA, ADPS1) ;
  cbi(ADCSRA, ADPS0) ;
#endif

  pinMode(CS_PIN, OUTPUT); //for ethernet shield through ICSP pins

  //set up communication with PC , LCD shield, SD
  // start laptop communication
  Serial.begin(115200);

  //start LCD communication
  lcd.begin(16, 2); // begin communication with an LCD of 16-by-2 size

  //start RTC communications
  Wire.begin();
  RTC.begin();

  if (! RTC.isrunning()) {
    Serial.println(F("(!) RTC non working"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("(!) RTC is" ));
    lcd.setCursor(0, 1);
    lcd.print(F("out of order"));
    delay(2000);
  }
  // following line sets the RTC to the date & time this sketch was compiled
  // uncomment it & upload to set the time, date and start run the RTC!
  //RTC.adjust(DateTime(__DATE__, __TIME__));


START: //milestone for 'goto' if card not working


  ////////////////////////////
  //SET PINS INPUT or OUTPUT//
  ////////////////////////////


  //cycle to set LBsensors as input
  for (unsigned char pin = 0; pin < LBSensorPinsTot; pin++)
  {
    pinMode(LBSensorPins[pin], INPUT_PULLUP);
  }


  //BUTTON PINS

  pinMode(SelectButtonPin, INPUT); //button select on the LCD screen

  //SD CARD PINS

  //check SD card is OK
  if (!SD.begin(CS_PIN))
  {
    Serial.println(F("(!) SD.begin failed. Fix card & PRESS RESET"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("(!)SD.begin failed"));
    lcd.setCursor(0, 1);
    lcd.print(F("ed. Fix card & PRESS RESET"));
    delay(2000);
    goto START;
  }
}

//|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

void loop() {


  /////////////////////////////////////////////////////////////////////////////////////////
  //In this MAIN LOOP, while cycles are used to establish "checkpoints" where
  //authorization by the operator (i.e. who is pressing "RIGHT") is
  //needed in order for the MAIN LOOP to proceed to the next stage. In particoular,
  //there are 5 stages:

  //1. Pre-test (pre-paration);
  //2. Preparation (give name to test);
  //3. Preparation (measurement vectors back to default stage);
  //4. Measurement Phase;
  //5. Data Logging phase.
  ////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////
  //1. Pre-test (pre-paration)//
  /////////////////////////////

  // check SD is ok at beggining of every test
  if (!card.init(SPI_HALF_SPEED, CS_PIN))
  {
    Serial.println(F("(!) SD Initialization failed. Is a card inserted?"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("(!) SD Initialization"));
    lcd.setCursor(0, 1);
    lcd.print(F("failed."));
    return;
  }
  else
  {
    Serial.println(F("() SD working properly"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("() SD working"));
    lcd.setCursor(0, 1);
    lcd.print(F("properly"));
  }

  Serial.println(F("(1/5) Select to authorize TEST PREPARATION"));
  lcd.print(F("(1/5) Select to authorize TEST PREPARATION"));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("(1/5)'RIGHT' for"));
  lcd.setCursor(0, 1);
  lcd.print(F("TEST PREPARATION"));

  while (digitalRead(SelectButtonPin) == HIGH)
  {
    //do nothing, just wait for button to be pushed so that break out of 'while' cycle
  }
  delay(2000); //delay of 2 secs beacuse humans press buttons too slowly: arduino thinks it has 'authorization' to proceed through downstream checkpoints.



  ////////////////////////////////////////
  //2. Preparation (give name to test) //
  ///////////////////////////////////////
  //check in the SD card the last file name
  while (SD.exists(fileName))
  {
    if (fileName[BASE_NAME_SIZE + 2] != '9') //increment one unit until unit=9 then go to next condition
    {
      fileName[BASE_NAME_SIZE + 2]++;
    }
    else if (fileName[BASE_NAME_SIZE + 1] != '9') //where unit posed to zero, but decade incremented by one
    {
      fileName[BASE_NAME_SIZE + 2] = '0';
      fileName[BASE_NAME_SIZE + 1]++;
    }
    else if (fileName[BASE_NAME_SIZE] != '9') //for when need to go from a century to another, e.g. 099 to 100
    {
      fileName[BASE_NAME_SIZE + 2] = '0';
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    }
    else
    {
      Serial.println(F("(!) Cannot create file name")); //print on LCD!! so modify
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("(!)CAN'T CREATE"));
      lcd.setCursor(0, 1);
      lcd.print("file name");
      return;
    } //now the next file name is found, and it is stored in "fileName"

  }

  Serial.println(F("(2/5) TEST PREPARATION UNDERGOING")); //print to LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("(2/5)TEST PREP"));
  lcd.setCursor(0, 1);
  lcd.print(F("UNDERGOING"));
  delay(2000);
  // Check where BEAM SENSORS ARE read HIGH to check alignment!

BEAM_ALIGN://check point for beam alignment. if not present, maybe 2 beams are misalgined and one is ok but the other not, arduino thinks everything is ok, because of the sequential check of beams of the for() loop
  //as a consequence in the field check them in order, i.e. fix beam 0, then 1, then 2, etc.
  for (unsigned char pin = 0; pin < LBSensorPinsTot; pin++)
  {
    while (digitalRead(LBSensorPins[pin]) < TSH )
    {

      Serial.print(F("(!) BEAM NOT ALIGNED:"));
      Serial.println(pin);
      Serial.print(F("(!) ALIGN and press RESET"));
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("(!)UNALIGNMENT"));
      lcd.setCursor(0, 1);
      lcd.print(F("#"));
      lcd.setCursor(1, 1);
      lcd.print(pin + 1);
      lcd.setCursor(4, 1);
      lcd.print(F("ALIGN&RESET"));
      delay(50);
      goto BEAM_ALIGN;

    }
  }

  Serial.println(F("() BEAMS ARE ALIGNED"));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("()BEAMS"));
  lcd.setCursor(0, 1);
  lcd.print(F("ARE ALIGNED"));
  delay(2000);

  Serial.print(F("THIS is DATA SET#   "));
  Serial.println(fileName); //print on LCD!! so modify..perhaps write 'press select to continue'
  Serial.println(F("(?) HAVE YOU TAKEN NOTE OF TEST #? - PRESS SELECT to CONTINUE"));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("DATA SET #"));
  lcd.setCursor(0, 1);
  lcd.print(fileName);

  while (digitalRead(SelectButtonPin) == HIGH)
  {
    //do nothing, just wait for button to be pushed so that break out of 'while' cycle
    //time to take note of the TEST NUMBER
  }
  delay(2000); //delay of 2 secs beacuse humans press buttons too slowly: arduino thinks it has 'authorization' to proceed through downstream checkpoints.

  //////////////////////////////////////////////////////////////
  //3. Preparation (measurement vectors back to default stage)//
  //////////////////////////////////////////////////////////////

  Serial.println(F("(3/5) VECTORS PREPARATION UNDERGOING"));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("(3/5) VECTORS"));
  lcd.setCursor(0, 1);
  lcd.print(F("PREP UNDERGOING"));
  delay(1000);

  //Time variables
  unsigned long ReadTime;

  // DateTime
  DateTime now;

  Serial.println(F("(4/5) MEASUREMENT PHASE. Press Select to START")); //print to LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("(4/5)MSRMT PHASE"));
  lcd.setCursor(0, 1);
  lcd.print(F("'RIGHT' to START"));

  while (digitalRead(SelectButtonPin) == HIGH)
  {
    //do nothing, just wait for button to be pushed so that break out of 'while' cycle
    //time to check bicyclist is ready to start
  }
  delay(2000); //delay of 2 secs beacuse humans press buttons too slowly: arduino thinks it has 'authorization' to proceed through downstream checkpoints.

  Serial.println(F("(4/5) NOW UNDER TESTING"));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("(4/5) NOW UNDER"));
  lcd.setCursor(0, 1);
  lcd.print(F("TESTING..."));
  delay(1500);
  lcd.clear();

  Serial.println(F("PRESS RIGHT to stop recording"));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("PRESS RIGHT"));
  lcd.setCursor(0, 1);
  lcd.print(F("TO SAVE&QUIT"));


  ///////////////////////////
  // 4. Measurement Phase///
  //////////////////////////

  file = SD.open(fileName, FILE_WRITE);
  now = RTC.now();
  file.print(fileName);
  file.print(F(","));
  file.print(now.day());
  file.print(F("/"));
  file.print(now.month());
  file.print(F("/"));
  file.print(now.year());
  file.print(F(","));
  file.print(now.hour());
  file.print(F(":"));
  file.print(now.minute());
  file.print(F(":"));
  file.println(now.second());
  file.print("TIME_ms");
  file.print(",");
  file.println("#BeamBroken");

  while ( digitalRead(SelectButtonPin) > 0 )
  {
    for (unsigned char pin = 0; pin < LBSensorPinsTot; pin++) //check SELECT so that if someone is gong to slow to reach last sensor, data can be saved anyways
    {
      
      while (digitalRead(LBSensorPins[pin]) > 0)
      {
        if (digitalRead(SelectButtonPin) < 1)
        {
          goto SAVE; //exit the loop
        }
      }
      //do nothing, waut for beam to break
    
    ReadTime = millis();
    lcd.setCursor(15, 0);
    lcd.print(pin + 1);
    file.print(ReadTime);
    file.print(",");
    file.println(pin + 1);
    }
  }

SAVE: //used to exit the loop
  now = RTC.now(); //finish time
  file.print(fileName);
  file.print(F(","));
  file.print(now.day());
  file.print(F("/"));
  file.print(now.month());
  file.print(F("/"));
  file.print(now.year());
  file.print(F(","));
  file.print(now.hour());
  file.print(F(":"));
  file.print(now.minute());
  file.print(F(":"));
  file.println(now.second());
  
  file.close();

  delay(2000); //delay in case SELECT is pressed in the WHILE() to skip last beams, and not to think authorization is grated for downstream checkpoints

  Serial.println(F("(4/5) END OF DATA COLL")); //print to LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("(4/5) END OF"));
  lcd.setCursor(0, 1);
  lcd.print(F("DATA COLL."));
  delay(2000);

}

