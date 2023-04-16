#include <MFRC522.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <SPI.h>

MFRC522 rfid(53, 5);
hd44780_I2Cexp lcd;

const int pushbuttonPin = 3;
const int buzzerPin = 2;
unsigned long prevEntranceTime = 0;

#define OBJECT 0xA0 // Object temperature command
#define SENSOR 0xA1 // Sensor temperature command

const int chipSelectPin = 10;
unsigned char Timer1_Flag = 0;
int iOBJECT, iSENSOR; // Signed 2byte temperature storage variables

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  lcd.begin(16, 2);
  pinMode(pushbuttonPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  /* Setting CS & SPI */
  digitalWrite(chipSelectPin, HIGH);     // SCE High Level
  pinMode(chipSelectPin, OUTPUT);        // SCE OUTPUT Mode
  SPI.setDataMode(SPI_MODE3);            // SPI Mode 
  SPI.setClockDivider(SPI_CLOCK_DIV16);  // 16MHz/16 = 1MHz
  SPI.setBitOrder(MSBFIRST);             // MSB First
  SPI.begin();                           // Initialize SPI

  delay(500);                            // Sensor initialization time 
  Timer1_Init();                         // Timer1 setup: 500ms(2Hz) interval
  interrupts();                          // enable all interrupts
}

void loop() {
  handleEmergencyButton();
  float temp = readTemperature();
  String uid = readNFCUid();
  checkAccessConditions(temp, uid);
}

void handleEmergencyButton() {
  int Emcall = digitalRead(pushbuttonPin);
  if (Emcall == 1) {
    tone(buzzerPin, 494, 1000);
    lcd.print("Emergency call");
    lcd.setCursor(0, 1);
    lcd.print("please stay here");
    delay(1000);
    lcd.clear();
  } else {
    noTone(buzzerPin);
  }
}

float readTemperature() {
  if (Timer1_Flag) {                                // Repeat every 500ms (Timer 1 Flag check)
    Timer1_Flag = 0;                                // Reset Flag
    iOBJECT = SPI_COMMAND(OBJECT);                  // Read object temperature
    delayMicroseconds(10);                          // 10us: Do not remove this line
    iSENSOR = SPI_COMMAND(SENSOR);                  // Read sensor temperature
  }
  return float(iOBJECT) / 100;
}

String readNFCUid() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return "";
  }

  String content = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    content.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(rfid.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  return content.substring(1);
}

void checkAccessConditions(float temp, String uid) {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - prevEntranceTime;
  bool tempCheckWithinTwoHours = elapsedTime <= 7200000;
  bool tempUnderLimit = temp < 37.5;

    if (((uid == "6F 05 FC 84" || uid == "A0 CB F2 32") && tempUnderLimit) || tempCheckWithinTwoHours) {
    lcd.clear();
    lcd.print("Access Granted");
    lcd.setCursor(0, 1);
    lcd.print("Temperature: ");
    lcd.print(temp);
    prevEntranceTime = currentTime;
  } else {
    lcd.clear();
    lcd.print("Access Denied");
    lcd.setCursor(0, 1);
    lcd.print("Temperature: ");
    lcd.print(temp);
  }
  delay(1000);
}

int SPI_COMMAND(unsigned char cCMD) {
  unsigned char T_high_byte, T_low_byte;
  digitalWrite(chipSelectPin, LOW); // SCE Low Level
  delayMicroseconds(10);            // delay(10us)
  SPI.transfer(cCMD);               // transfer  1st Byte
  delayMicroseconds(10);            // delay(10us)
  T_low_byte = SPI.transfer(0x22);  // transfer  2nd Byte
  delayMicroseconds(10);            // delay(10us)
  T_high_byte = SPI.transfer(0x22); // transfer  3rd Byte
  delayMicroseconds(10);            // delay(10us)
  digitalWrite(chipSelectPin, HIGH); // SCE High Level

  return (T_high_byte << 8 | T_low_byte); // Return temperature value
}

ISR(TIMER1_OVF_vect) { // interrupt service routine (Timer1 overflow)
  TCNT1 = 34286;       // preload timer: Do not change this value.
  Timer1_Flag = 1;     // Timer 1 Set Flag
}

void Timer1_Init(void) {
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 34286;            // preload timer 65536-16MHz/256/2Hz
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
}

