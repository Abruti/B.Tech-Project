/*------------------------------------------------------------------------------ 
Program:      ask_transmitter and  ask_receiver
 
Description:  Slight variation of the original example code (included in the 
              RadioHead-1.74 library) for the ASK transmitter and receiver:
              simple example of how to use RadioHead to transmit messages
              with a simple ASK transmitter in a very simple way.
              Implements a simplex (one-way) transmitter with an TX-C1 module.
 
------------------------------------------------------------------------------*/
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile
#include <TinyGPS++.h>
#include <RH_ASK.h>

TinyGPSPlus gps;
TinyGPSTime gpsTime;

String uartDataTxStr;
String rfDataTxStr;

float yaw = 0;

double flat, flon;
int timeSlice;
int txOrRx = 0;

int id=2;   //<----------------------------------------------DEFINE THIS FOR EVERY NEW SYSTEM-----------------------------------------

String str/*="!$17.3456,13.2349#"*/;
char *msg;

// tx/rx configuration
const int txSpeed = 2000;
const int txOrRxPin = 48; //8     // For Rx, 0 and for tx, ,COONECT TO PIN 40
//const int selectPin = 40;// connect to pin 48
const int rxPin = 50;//5
const int txPin = 52;//6
const int pttPin = 7;
 
RH_ASK driver(txSpeed, rxPin, txPin, pttPin);

void rfSend(double slat,double slon)
{
  String s1=String(slat,8);
  String s2=String(slon,8);
  String s3=String(yaw,2);
  rfDataTxStr="!"+s1+","+s2+":"+s3+"#"+String(id);
  Serial.println(rfDataTxStr);
}

void uartSend(double slat,double slon, float this_yaw)
{
  String s1=String(slat,8);
  String s2=String(slon,8);
 String s3=String(this_yaw,2);
  uartDataTxStr=String("!"+s1+","+s2+":"+s3+"#@");
  Serial.println(uartDataTxStr);
  Serial2.println(uartDataTxStr);
}

void setup()
{
  pinMode(txOrRxPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);  // PC Debugging only
  Serial1.begin(9600);  // gps
  Serial2.begin(9600);  // uno tft
  Serial3.begin(4800);  // nano gyro
  if (!driver.init())
    Serial.println("rf driver init failed");
 
}
 
void loop()
{
  
 // txOrRx = digitalRead(txOrRxPin);
  uint8_t buf[56] = {0};
  uint8_t buflen = sizeof(buf);
  
  if(Serial3.available())
  {
    yaw=Serial3.parseFloat();
    Serial.print("Yaw: ");
    Serial.println(yaw);
  }
    
  while (Serial1.available())   //try making this while as if
  {
    int c = Serial1.read();
    if (gps.encode(c))
    {
      flat=(double)gps.location.lat();
      flon=(double)gps.location.lng();
    }

    gpsTime=gps.time;
    if(gpsTime.isValid())
      {
      int currTime=gpsTime.second();
      timeSlice=(currTime%3)+1;

      if(id==timeSlice)
        txOrRx=1;         
      else
        txOrRx=0;    
      }
     //lat 18.53139000
    //lon 73.85505000
  } 
  //uartSend(18.53139000,73.85505000);
  
//--------------------------------------------------
  uartSend(flat,flon,yaw);
//---------------------------------------------------    
  if(txOrRx)          //Tx
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);      //<--------------------------------------commented for timing purposes
    rfSend(flat,flon);
    //rfSend(18.53139000,73.85505000);
    driver.send((uint8_t *)(rfDataTxStr.c_str()), strlen(rfDataTxStr.c_str()));
    driver.waitPacketSent(); 
    delay(100);      //<-------------------------------------commented for timing purposes
  }

  else
  {
    if (driver.recv(buf, &buflen)) // if message received, save it
    {
      str = String((char*)buf);
      Serial.println(str);
      Serial2.println(str);
    }
  }
}
