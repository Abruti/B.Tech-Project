#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

/* 
 * Code to run UART GPS and TFT together.
 * Download the following libraries: TinyGPS(not available in IDE), AdafruitGFX, MCUFriend_kbv
 * This code maps GPS coordinates to a circle on the tft screen and provides UART functionality
 * CONNECTIONS: 
    * Connect respective arduino pins to shield using jumpers as they would be connected if the shield was plugged into the arduino board  
    * Pin 13-> GPS Tx
    * PIN 12-> GPS Rx
    * Connect 12V adapter to GPS module to power it up GND of GPS connects to GND of arduino
    * 5V of TFT to arduino 5V and GND to GND
    */

    /*
    Free Pins:
   GPIO: 6,7,A1,A2 (Provided touchscreen is not used)
   Other IOs: 0,1 (UART pins)
    NOTE: FLOAT HAS BEEN CHANGED TO DOUBLE IN ALL HEADER FILES OF TINYGPS using find and replace, if error/problem occurs, redownload TinyGPS for arduino
    */
#include <MCUFRIEND_kbv.h>
#include <TinyGPS++.h>
#include<math.h>
/***************************************** MACROS***********************************************************************/
#define PI 3.14159265358979
#define R 6371.8
#define pixelX 320  
#define pixelY 480
#define centre_x pixelX/2
#define centre_y pixelY/2
/*COLOURS AND HEX VALUES*/                         
#define         BLACK    0x0000
#define         BLUE     0x001F
#define         RED      0xF800
#define         GREEN    0x07E0
#define         CYAN     0x07FF
#define         MAGENTA  0xF81F
#define         YELLOW   0xFFE0
#define         WHITE    0xFFFF
#define         BGCLR    BLACK
 /***************************************** FUNCTION DECLARATIONS***********************************************************************/
double toRadians(double);
void show_Serial(void);
void show_tft(void);
struct cartesian get_xy(double this_lat,double this_lon);
int sign(double num);
//void uartSend(double lat,double lon);
void uartReceive();
void drawCar(int id);
/***************************************** INSTANTIATIONS******************************************************************************/
MCUFRIEND_kbv tft;       // hard-wired for UNO shields anyway.
TinyGPSPlus gps;

/*******************************************GLOBALS************************************************************************************/
uint8_t Orientation = 2;    // ulta PORTRAIT
uint16_t ID;
/*double slat=18.2236;
double slon=17.2195;
double slat_dummy= 18.2236;
double slon_dummy=17.2190;*/
//for xy mapping on tft
struct cartesian
{
  double x;
  double y;
  double dist;
  double latitude;
  double longitude;
};
struct cartesian c1,c2,c3,c_self;
/*****************************************MAIN CODE***********************************************************************************/
void setup(void)
{
    uint16_t tmp;
    tft.reset();
    ID = tft.readID();
    tft.begin(ID);
    tft.setRotation(Orientation);
    tft.fillScreen(BGCLR);
    show_tft();
    Serial.begin(9600);
    c_self.x=centre_x; c_self.y=centre_y; /*CENTRE POINT OF TFT SELF COORDINATES*/
    tft.drawCircle(c_self.x,c_self.y,150,CYAN);
    tft.fillCircle(c_self.x,c_self.y,2,YELLOW);
   
}



void loop()
{
 while(!Serial.available());
      uartReceive();      
  // delay(100);
 }

 
/***********************************************FUNCTION DEFINITIONS**************************************************/
double toRadians(double myDeg)
{
 return (myDeg*PI/180);
}

void show_tft(void)
{
    tft.setCursor(0, 0);
    tft.setTextSize(1);
}

void set_xy(struct cartesian *this_car)
{ 
  //5% error roughly. find correction formula if possible
  double xc,yc;
  yc=R*toRadians(this_car->latitude-c_self.latitude); 
  xc=R*toRadians(this_car->longitude-c_self.longitude);
  this_car->dist= gps.distanceBetween(c_self.latitude,c_self.longitude,this_car->latitude,this_car->longitude);
  //Relative to centre of circle
  this_car->x=   centre_x + xc*1500;  ///sign  of xc and yc should be taken into account
  this_car->y=   centre_y + yc*1500;  ///sign  of xc and yc should be taken into account
 


}

int sign(double num)
{
   if(num>0)
    return 1;

   else if(num<0)
    return -1;

   else if(num==0)
    return 0;
}

void drawCar(int id)
{
  switch(id)
  {
    case 1:
    {
      set_xy(&c1);
      tft.fillCircle(c1.x,c1.y,2,RED);
      break;
    }
    case 2:
    {
      set_xy(&c2);
      tft.fillCircle(c2.x,c2.y,2,GREEN);
      break;
    }
    case 3:
    {
      set_xy(&c3);
      tft.fillCircle(c3.x,c3.y,2,BLUE);
      break;
    }
  }
  
}

void eraseCar(int id)
{
  switch(id)
  {
    case 1:
    {
      tft.fillCircle(c1.x,c1.y,2,BGCLR);
      break;
    }
    case 2:
    {
      tft.fillCircle(c2.x,c2.y,2,BGCLR);
      break;
    }
    case 3:
    {
      tft.fillCircle(c3.x,c3.y,2,BGCLR);
      break;
    }
  }
}

/*void uartSend(double lat,double lon)
{
String s1=String(lat,8);
String s2=String(lon,8);
String uartDataTxStr=String("!"+s1+","+s2+"#");
Serial.println(uartDataTxStr);
}*/


void uartReceive()
{
  char chArr1[12]={0};
  char chArr2[12]={0};
  char* ch;
  Serial.readStringUntil('!');
  String uartDataRxStr=Serial.readStringUntil(',');
    uartDataRxStr.toCharArray(chArr1,11);
    double this_slat=strtod(chArr1,&ch);
    delay(10);
    uartDataRxStr=Serial.readStringUntil('#');
    uartDataRxStr.toCharArray(chArr2,11);
    double this_slon=strtod(chArr2,&ch);
    delay(10);
    char selfOrRx = Serial.read();
  if(selfOrRx == '@')
  {
    c_self.latitude= this_slat;
    c_self.longitude=this_slon;
  }
  else if(selfOrRx=='1')
  {
    eraseCar(1);
    c1.latitude= this_slat;
    c1.longitude=this_slon;
    drawCar(1);
  }
  else if(selfOrRx=='2')
  {
    eraseCar(2);
    c2.latitude= this_slat;
    c2.longitude=this_slon;
    drawCar(2);
  }
  else if(selfOrRx=='3')
  {
    eraseCar(3);
    c3.latitude= this_slat;
    c3.longitude=this_slon;
    drawCar(3);
  }
}
