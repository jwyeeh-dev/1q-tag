
/*****************************************************************************
*
* Copyright (C) 2016 Diwell Electronics Co.,Ltd.
* Project Name : (DTS 시리즈) SPI Master Code
* Version : 1.1
* SYSTEM CLOCK : 16Mhz 
* BOARD : Arduino UNO. 5V operation 
* 지원 모델명 : DTS-SIL300-B, DTS-L300-V2
* 주의 : 본 예제는 DTS-M300 구동이 불가합니다.
 PORT Description

1. ChipSelectPin : 10           
2. MOSI(Master Output) : 11
3. MISO(Master Input) : 12
4. SCK : 13
  온도센서모듈 입력전원은 3.3V로 하셔야 하며 포트 연결 방법은 회로도를 참고하십시오.
  온도센서 통신포트의 논리 레벨은 3.3V 이기 때문에 반드시 회로도를 참고하시기 바랍니다.

 Revision history.

1. 2016.5.4  : First version is released.
2. 2020.1.2 : 데이터 시트 통신 지연시간 변경에 따른 코드 수정.
****************************************************************************/


#include<SPI.h>

#define OBJECT	0xA0			// 대상 온도 커맨드
#define SENSOR	0xA1			// 센서 온도 커맨드

const int chipSelectPin  = 10;
unsigned char Timer1_Flag = 0;
int  iOBJECT, iSENSOR;	// 부호 2byte 온도 저장 변수 

void setup() {
  /* Setting CS & SPI */
  digitalWrite(chipSelectPin , HIGH);    // SCE High Level
  pinMode(chipSelectPin , OUTPUT);        // SCE OUTPUT Mode
  SPI.setDataMode(SPI_MODE3);            // SPI Mode 
  SPI.setClockDivider(SPI_CLOCK_DIV16);  // 16MHz/16 = 1MHz
  SPI.setBitOrder(MSBFIRST);             // MSB First
  SPI.begin();                           // Initialize SPI
    
  delay(500);                             // Sensor initialization time 
  Timer1_Init();                          // Timer1 setup : 500ms(2Hz) interval
  Serial.begin(9600);
  interrupts();                           // enable all interrupts
}
int SPI_COMMAND(unsigned char cCMD)
{
    unsigned char T_high_byte, T_low_byte;
    digitalWrite(chipSelectPin , LOW);  // SCE Low Level
    delayMicroseconds(10);              // delay(10us)
    SPI.transfer(cCMD);                // transfer  1st Byte
    delayMicroseconds(10);              // delay(10us)        
    T_low_byte = SPI.transfer(0x22);   // transfer  2nd Byte
    delayMicroseconds(10);              // delay(10us) 
    T_high_byte = SPI.transfer(0x22);  // transfer  3rd Byte
    delayMicroseconds(10);              // delay(10us)
    digitalWrite(chipSelectPin , HIGH); // SCE High Level 
    
    return (T_high_byte<<8 | T_low_byte);	// 온도값 return 
}
ISR(TIMER1_OVF_vect){        // interrupt service routine (Timer1 overflow)
  TCNT1 = 34286;            // preload timer : 이 값을 바꾸지 마세요.
  Timer1_Flag = 1;          // Timer 1 Set Flag
}
void Timer1_Init(void){
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 34286;            // preload timer 65536-16MHz/256/2Hz
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
}
void loop() {
  // put your main code here, to run repeatedly:
 
  if(Timer1_Flag){                                       // 500ms마다 반복 실행(Timer 1 Flag check)
    Timer1_Flag = 0;                                    // Reset Flag
    iOBJECT = SPI_COMMAND(OBJECT);			// 대상 온도 Read 
    delayMicroseconds(10);                              // 10us : 이 라인을 지우지 마세요 
    iSENSOR = SPI_COMMAND(SENSOR);			// 센서 온도 Read
    
    Serial.print("Object Temp : ");                    // 하이퍼터미널 출력
    Serial.print(float(iOBJECT)/100,2);
    Serial.print("     Sensor Temp : ");
    Serial.println(float(iSENSOR)/100,2);
   }
}
