#include <SoftwareSerial.h>

/*여닫이문 모터*/
/*#include <Stepper.h>
const int steps = 512; //90도 개방
Stepper DoorOpen(steps,7,6,5,4); */

#include <Wire.h>

#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <LiquidCrystal.h> 
hd44780_I2Cexp lcd; //lcd 디스플레이 객체 설정 

#include <Wire.h>
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
 
//RFID 설정
#include <MFRC522.h>
#include <SPI.h>
#define SS_1_PIN 48
#define SS_2_PIN 49
#define RST_PIN 5
#define NR_OF_READERS   2
byte ssPins[] = {SS_1_PIN, SS_2_PIN};
MFRC522 mfrc522[NR_OF_READERS]; //rfid 객체 생성

//긴급호출 핀 설정 
int pushbuttonPin = 3;
int buzzerPin = 2;

//인적사항 관련 
String univNo;
String personname;
String phonenumber;



void setup() {

//DoorOpen.setSpeed(10);

Serial.begin(9600); //시리얼 모니터링 
while (!Serial);

SPI.begin();      // SPI 통신 시작

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    
  /*  Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": ")); */
    
    mfrc522[reader].PCD_DumpVersionToSerial();
    /*USB로 엑셀 DB 접속*/
    Serial.println("CLEARDATA");
    Serial.println("LABEL, TIME, UNIVNUMBER, NAME, TEMP, PHONENUMBER");
  }  
  


mlx.begin();  

lcd.begin(16,2); 


pinMode(pushbuttonPin, INPUT);
pinMode(buzzerPin, OUTPUT); 

}


void loop() {


  /* 긴급호출 버튼 누르면 부저 울림 */
  int Emcall = digitalRead(pushbuttonPin);
  if(Emcall == 1){
    tone(buzzerPin, 494, 1000);
    
  /*  Serial.println("Push!"); */
    
    lcd.print("Emergency call");
    lcd.setCursor(0,1);
    lcd.print("please stay here");
    delay(1000);
    lcd.clear();
    
  } 
  /* 긴급호출 누르지 않으면 부저 울리지 않음 */
  else {
    noTone(buzzerPin);
  }  

/* 온도 센서 변수 설정 */
 float temp;
 temp = mlx.readObjectTempC(); 

 /* 온도 센서 작동 안할 시 시리얼 메시지 */  
 if(isnan(temp)) {
  Serial.println("Failed");
  }  
  
  
/*nfc 확인*/
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    // Look for new cards

    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
     /* Serial.print(F("Reader "));
      Serial.print(reader); */
      // Show some details of the PICC (that is: the tag/card)
     /*  Serial.print(F(": Card UID:")); */
      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
     /* Serial.println();*/


      // Halt PICC
      mfrc522[reader].PICC_HaltA();
      // Stop encryption on PCD
      mfrc522[reader].PCD_StopCrypto1();
    } //if (mfrc522[reader].PICC_IsNewC
  } //for(uint8_t reader
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  } 

/*
//시리얼 모니터링용 온도 표현 
  Serial.print(" 온도 : ");
  Serial.print(temp);
  Serial.println("℃");
  delay(500);
  Serial.println();
  Serial.println();
  Serial.print("Message : ");
*/

  
uint8_t reader = 0;

 if ((dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size) == "6F 05 FC 84" || dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size) == "A0 CB F2 32") && temp < 37.5) // 등록된 사람이면서 37.5 미만인 사람 (정상)
  {
  /*  Serial.println("Authorized access"); //시리얼 창에 표시 
    Serial.println(); */
    
//uid 인적사항 출력
if(content.substring(1) == "6F 05 FC 84")
{
 univNo = "17101400";
 personname = "HwangJaeyeong";
 phonenumber = "01077034572";
}
if(content.substring(1) == "A0 CB F2 32")
{  
 univNo = "Valid_1";
 personname = "KimDeetee";
 phonenumber = "01012345678";
}
/* 엑셀 DB 시리얼 연동*/
    Serial.print("DATA,TIME," );
    Serial.print(univNo);
    Serial.print(",");
    Serial.print(personname);
    Serial.print(",");
    Serial.print(temp);
    Serial.print(",");
    Serial.print(phonenumber);
    Serial.println();
    
   /*   Serial.print(" 이름 : ");
      Serial.print(personname);
      Serial.println("\n");
      delay(500);
      Serial.print(" 학번 : ");
      Serial.print(univNo);
      Serial.println("\n");
      delay(500);
      Serial.print(" 연락처 : ");
      Serial.print(phonenumber);
      Serial.println("\n");
      delay(500);
      */
  
      lcd.print("Valid"); //디스플레이에 허가 표시 
      lcd.print(", OK");
      lcd.setCursor(0,1);
      lcd.print("temp : ");
      lcd.print(temp);
 
 /*Serial.println("문 개방");*/
 //DoorOpen.step(steps); // 시계 방향으로 스텝 수만큼 이동
 //delay(300);

 /*Serial.println("문 폐쇄");*/
 /* DoorOpen.step(-steps); // 반 시계 방향으로 스텝 수만큼 이동
 delay(300); // 모터가 이동할 시간만큼 딜레이 설정 */
 
  }

uint8_t reader = 0;
  else if((dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size) == "A0 CB F2 32" || dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size) == "6F 05 FC 84") && temp >= 37.5) //등록된 사람이면서 37.5 이상인 사람 (발열자) 
  {
    /* 시리얼에 결과 표시 */
  /*  Serial.println("Authorized Denied");
    Serial.println(); */

//uid 인적사항 출력
if(content.substring(1) == "6F 05 FC 84")
{
 univNo = "17101400";
 personname = "HwangJaeyeong";
 phonenumber = "01077034572";
}
if(content.substring(1) == "A0 CB F2 32")
{  
 univNo = "Valid_1";
 personname = "KimDeetee";
 phonenumber = "01012345678";
}

    /* 엑셀 DB 시리얼 연동*/
    Serial.print("DATA,TIME,");
    Serial.print(univNo);
    Serial.print(",");
    Serial.print(personname);
    Serial.print(",");
    Serial.print(temp);
    Serial.print(",");
    Serial.print(phonenumber);
    Serial.println();
    
/*      Serial.print(" 이름 : ");
      Serial.print(personname);
      Serial.println("\n");
      delay(500);
      Serial.print(" 학번 : ");
      Serial.print(univNo);
      Serial.println("\n");
      delay(500);
      Serial.print(" 연락처 : ");
      Serial.print(phonenumber);
      Serial.println("\n");
      delay(500); */

    /* 디스플레이에 비허가 문구 표시 */
    lcd.print("Invalid");
    lcd.print(", HIGH");
    lcd.setCursor(0,1);
    lcd.print("temp : ");
    lcd.print(temp);
  }

 else //등록되지 않은 사람
 {
   /* 시리얼에 결과 표시 */
   /* Serial.println("Authorized Denied");
   Serial.println(); */

    /* 엑셀 DB 시리얼 연동*/
    Serial.print("DATA,TIME,");
    Serial.print("Unknown");
    Serial.print(",");
    Serial.print("-");
    Serial.print(",");
    Serial.print(temp);
    Serial.print(",");
    Serial.print("-");
    Serial.println();

   /* 디스플레이에 비허가 문구 표시 */
   lcd.print("Invalid, N/A"); 
   lcd.setCursor(0,1);
   lcd.print("temp : ");
   lcd.print(temp);
   delay(500);
   lcd.clear(); 
   lcd.print("please applying"); 
  }



 delay(500);
  lcd.clear();

  
  }
  
