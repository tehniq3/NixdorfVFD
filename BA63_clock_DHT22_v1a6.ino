/*
 * inspired by http://gorgusgfx.se/?page_id=62
 * kod för att styra en DLR3416-display från Siemens
 * Nicu FLORICA tests 5 module HDLO-2416 
 * for future projects see http://arduinotehniq.com
 * http://nicuflorica.blogspot.com/2016/11/afisaje-dhlo-2416-cu-4-caractere.html
 * http://nicuflorica.blogspot.com/2016/12/afisaje-hdlo-2416-cu-4-caractere.html
 * http://nicuflorica.blogspot.com/2016/12/afisaje-hdlo-2416-cu-4-caractere_23.html
 * https://github.com/tehniq3/HDLO-2416/
 * 
 * ver.0 - changed for Datecs DPD-201 (firmware 2.4/2004) - Craiova, 7.2021
 * ver.0a - add day name
 * ver.1 - add clock adjust (using 3 buttons: menu/bext, increase (+), decrease (-)
 * ver.1a - no flicker after long time, thanks to mihaiaurul from elforum for correction
 * ver.1.a1 - remove seconds to be stattic info on display for no any flickers
 * 
 * BA63_ver.1a1 - moved on Nixdorf PL-200/BA63 VFD customer display
 * Important: Add J3 jumper to the display to disable parity check, remove all other jumpers from the settings panel 
 * schematic is same as for DPD-201 serial display: https://nicuflorica.blogspot.com/2021/07/afisaj-vfd-modul-datecs-dpd-201.html
 */

#include <NixdorfVFD.h> // https://github.com/MrTransistorsChannel/NixdorfVFD
#include <SoftwareSerial.h>
#define DATAOUT 6 

NixdorfVFD VFD;
SoftwareSerial vfd(255, DATAOUT, true);  // This is required, to start an instance of an LCD (RX at pin D6), inverted logic

// Date and time functions using a DS1307/DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday    ", "Monday    ", "Tuesday   ", "Wednesday ", "Thursday  ", "Friday    ", "Saturday  "};


#include "DHT.h"
#define DHTPIN 12     // what pin we're connected to
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);
// NOTE: For working with a faster chip, like an Arduino Due or Teensy, you
// might need to increase the threshold for cycle counts considered a 1 or 0.
// You can do this by passing a 3rd parameter for this threshold.  It's a bit
// of fiddling to find the right value, but in general the faster the CPU the
// higher the value.  The default for a 16mhz AVR is a value of 6.  For an
// Arduino Due that runs at 84mhz a value of 30 works.
// Example to initialize DHT sensor for Arduino Due:
//DHT dht(DHTPIN, DHTTYPE, 30);

int timp = 250;
int timp2 = 1000;

float temperatura;
int umiditate;
int tzeci, tunit, tzecimi, trest;
int tsemn, ttot;
int hzeci, hunit;

int hh, mm, ss;
int yy, ll, dd, zz;

int i1,i2;
byte numar = 0;
unsigned long tprtc = 900;  // time between read RTC
unsigned long tpcitirertc = 0;
unsigned long tpcitire = 0;
unsigned long tppauza = 60000;  // time between read DHT 

unsigned long tpreglaj = 0; 
unsigned long tpreglajmax = 600000; // maximum time for adjust clock

#define meniu 4    // button MENU 
#define plus 2     // button +
#define minus 3     // button -
int nivel = 20;     // state level 
// 0 - normal clock
// 2 - change hour
// 4 - change minute
// 6 - chamge year
// 8 - change month
// 10 - change day
// 
// 20 - ready
int limba = 1;   // 1 for english, 2 for roumanian  // ToDo
int maxday;
int pornire = 1; // force display the clock at startup not just when changes the minutes 

void setup()
{
  delay(5000);                  // wait to be ready the VFD to receive the data
  vfd.begin(9600);              // Starting serial communication at 9600bod
  VFD.begin(vfd);               // Initialising the display
  VFD.clear();                  // Clearing screen
  VFD.home();                   // Returning cursor to the start position
  VFD.setCodePage(CP_866);      // Enabling cyrillic code page
  
 dht.begin();

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
   if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
//rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // uncomment the line to refresh tge clock, upload sketch, comment the line, re-upload the sketch
// rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

pinMode(meniu, INPUT);   // MENU button
pinMode(plus, INPUT);   // + button
pinMode(minus, INPUT);   // + button
digitalWrite(meniu, HIGH);  // pull-up the pin
digitalWrite(plus, HIGH);   // pull-up the pin
digitalWrite(minus, HIGH);  // pull-up the pin

VFD.clear();   
delay(500);

  VFD.setCursor(0, 0);          // Moving cursor to 1st column in the 1st row
  VFD.print(" niq_ro's clock on  ");    
  VFD.setCursor(0, 1);          // Moving cursor to 1st column in the 2nd row
  VFD.print("Nixdorf VFD display ");

delay(5000);
temperatura = 10*dht.readTemperature();
umiditate = dht.readHumidity();
VFD.clear();
}
 
void loop()
{

if (nivel == 0)  // clock
{

if (digitalRead(meniu) == LOW)  // go to hour set
  {
  nivel = 1;
  tpreglaj = millis();
  VFD.clear();
  delay(500);
  }
  
//if ((millis() - tpcitirertc > tprtc) or (pornire == 1))
if (millis() - tpcitirertc > tprtc)
{
//pornire = 0;
DateTime now = rtc.now();
   /* 
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
  */
    
hh = now.hour();
mm = now.minute();
ss = now.second();
yy = now.year()-2000;
ll = now.month();
dd = now.day();
zz = now.dayOfTheWeek();

String text = "";
text = text + dd/10 + dd%10 + "/" + ll/10 + ll%10 + "/" + "20" + yy + "  ";
text = text + hh/10 + hh%10 + ":" + mm/10 + mm%10 + ":" + ss/10 + ss%10;

String text3 = "";
text3 = text3 + daysOfTheWeek[zz] + "  ";
text3 = text3 + hh/10 + hh%10 + ":" + mm/10 + mm%10 + ":" + ss/10 + ss%10;
//text3 = text3 + hh/10 + hh%10 + ":" + mm/10 + mm%10;

if (temperatura >= 0) 
{
  tsemn = 1;
  ttot = temperatura;
}
if (temperatura < 0) 
{
  tsemn = -1;
  ttot = -temperatura;
}

tzeci = ttot/100;
trest = ttot%100;
tunit = trest/10;
tzecimi = trest%10;

hzeci = umiditate/10;
hunit = umiditate%10;


String text1 = "";
text1 = text1 + "Temperature: ";
  if (tsemn == -1)
  {
   if (tzeci > 0)
   text1 = text1 + "-" + tzeci;
   else
   text1 = text1 + " -";
  }
  else
  if (tsemn == 1)
  {
   if (tzeci > 0)
   text1 = text1 + "+" + tzeci;
   else
   text1 = text1 + " +";
  }
  else
   text1 = text1 + "  ";

text1 = text1 + tunit + "." + tzecimi + "`C";

String text2 = "";
text2 = text2 + "Air Humidity:  ";
 if (hzeci == 0)
text2 = text2 + " "+ hunit + "%RH";
 else
text2 = text2 + hzeci + hunit + "%RH";

if (numar < 5)
{
VFD.setCursor(0, 0); 
VFD.print(text);
VFD.setCursor(0, 1); 
VFD.print(text1);
}
else
{
VFD.setCursor(0, 0); 
VFD.print(text3);
VFD.setCursor(0, 1); 
VFD.print(text2);
}

numar = numar + 1;
if (numar > 10)
    numar = 0;
tpcitirertc = millis();
}  //

if (millis() - tpcitire > tppauza) 
{  
temperatura = 10*dht.readTemperature();
umiditate = dht.readHumidity();
//temperatura = random(-150, 350);  // for test
//umiditate = random (10, 90);  // for test
tpcitire = millis();
}
} // end nivel = 0


if (nivel == 1)
{
  VFD.clear();
  VFD.setCursor(0, 0);          
  VFD.print("Hour set:           ");  
  nivel = 11;
  }


if (nivel == 11)  // hour adjust
{
  
if (digitalRead(meniu) == LOW)  // jump to minute set
  {
  nivel = 2;
  VFD.clear();
  delay(500);
  }

  VFD.setCursor(12, 0);          
  VFD.print(hh/10); 
  VFD.print(hh%10);

if (digitalRead(plus) == LOW) 
  {
  hh = hh + 1;
  delay(250);
  }
if (digitalRead(minus) == LOW) 
  {
  hh = hh - 1;
  delay(250);
  }
  
if (hh > 23) hh = 0; 
if (hh < 0) hh = 23;  
//VFD.clear();
}  // end nivel = 1;   

 if (nivel == 2)
 {
  VFD.clear();
  VFD.setCursor(0, 0);          
  VFD.print("Minute set:         "); 
  nivel = 22; 
  }

if (nivel == 22)  // minute adjust
{
  
if (digitalRead(meniu) == LOW)  // jump to year set
  {
  nivel = 3;
  VFD.clear();
  delay(500);
  }

  VFD.setCursor(12, 0);          
  VFD.print(mm/10); 
  VFD.print(mm%10);

if (digitalRead(plus) == LOW) 
  {
  mm = mm + 1;
  delay(250);
  }
if (digitalRead(minus) == LOW) 
  {
  mm = mm - 1;
  delay(250);
  }
  
if (mm > 59) mm = 0; 
if (mm < 0) mm = 59;  

}  // end nivel = 2;  


 if (nivel == 3)
 {
  VFD.clear();
  VFD.setCursor(0, 0);          
  VFD.print("Year set: 20        ");
  nivel = 32;  
  }

if (nivel == 32)  // year adjust
{
  
if (digitalRead(meniu) == LOW) // jump to month set
  {
  nivel = 4;
  VFD.clear();
  delay(500);
  }

  VFD.setCursor(12, 0);          
  VFD.print(yy/10); 
  VFD.print(yy%10);

if (digitalRead(plus) == LOW) 
  {
  yy = yy + 1;
  delay(250);
  }
if (digitalRead(minus) == LOW) 
  {
  yy = yy - 1;
  delay(250);
  }
  
if (yy > 50) yy = 25; 
if (yy < 25) yy = 50;  
//VFD.clear();
}  // end nivel = 3;  

 if (nivel == 4)
 {
  VFD.clear();
  VFD.setCursor(0, 0);          
  VFD.print("Month set:          ");  
  nivel = 42;
  }


if (nivel == 42)  // month adjust
{
  
if (digitalRead(meniu) == LOW)
  {
  nivel = 5;
  VFD.clear();
  delay(500);
  }

  VFD.setCursor(12, 0);          
  VFD.print(ll/10); 
  VFD.print(ll%10);

if (digitalRead(plus) == LOW) 
  {
  ll = ll + 1;
  delay(250);
  }
if (digitalRead(minus) == LOW) 
  {
  ll = ll - 1;
  delay(250);
  }
  
if (ll > 12) ll = 1; 
if (ll < 1) ll = 12;  
//VFD.clear();
}  // end nivel = 4; 


  if (nivel == 5)
  {
  VFD.clear();
  VFD.setCursor(0, 0);          
  VFD.print("Day set:             ");  
  nivel = 52;
  }

if (nivel == 52)  // day adjust
{
  
if (digitalRead(meniu) == LOW)  // jump to day in week
  {
  nivel = 6;
  VFD.clear();
  delay(500);
  }

  VFD.setCursor(12, 0);          
  VFD.print(dd/10); 
  VFD.print(dd%10);

if (digitalRead(plus) == LOW) 
  {
  dd = dd + 1;
  delay(250);
  }
if (digitalRead(minus) == LOW) 
  {
  dd = dd - 1;
  delay(250);
  }

// Dawn & Dusk controller. http://andydoz.blogspot.ro/2014_08_01_archive.html
 if (ll == 4 || ll == 5 || ll == 9 || ll == 11) { //30 days hath September, April June and November
    maxday = 30;
  }
  else {
  maxday = 31; //... all the others have 31
  }
  if (ll ==2 && yy % 4 ==0) { //... Except February alone, and that has 28 days clear, and 29 in a leap year.
    maxday = 29;
  }
  if (ll ==2 && ll % 4 !=0) {
    maxday = 28;
  }

if (dd > maxday) dd = 1;   
if (dd < 1) dd = maxday;  
//VFD.clear();
}  // end nivel = 5; 


  if (nivel == 6)
  {
  VFD.clear();
  VFD.setCursor(0, 0);          
  VFD.print("Day in week:        ");  
  nivel = 62;
  }

if (nivel == 62)  // day in week adjust
{
  
if (digitalRead(meniu) == LOW) // jump to store the chamged data
  {
  nivel = 7;
  VFD.clear();
  delay(500);
  }

  VFD.setCursor(12, 0);          
  VFD.print(zz); 
  VFD.setCursor(0, 1);          
  VFD.print(daysOfTheWeek[zz]); 

if (digitalRead(plus) == LOW) 
  {
  zz = zz + 1;
  delay(250);
  }
if (digitalRead(minus) == LOW) 
  {
  zz = zz - 1;
  delay(250);
  }

if (zz > 6) zz = 0;   
if (zz < 0) zz = 6;  
//VFD.clear();
}  // end nivel = 6; 

if (nivel == 7)
{ 
 // January 21, 2014 at 3am you would call:
 rtc.adjust(DateTime(2000+yy, ll, dd, hh, mm, 0));
 //setDS3231time(0, mm, hh, zz, dd, ll, yy);
 nivel = 20;   
}
  
  
if (nivel == 20)
{
VFD.clear();
temperatura = 10*dht.readTemperature();
umiditate = dht.readHumidity();
delay(1000);
nivel = 0;
}  // end last level & go to clock level


if(nivel != 0 &&  millis() - tpreglaj > tpreglajmax)  // va intra aici doar dacă nu este in functionare normala
//if (millis() - tpreglaj > tpreglajmax)
{
VFD.clear();
nivel = 0; 
}

}  // end main program
 
