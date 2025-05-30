/*
 * Important: Add J3 jumper to the display to disable parity check, remove all other jumpers from the settings panel 
 * 
 * Nicu Florica (niq_ro) easy changed the original sketch
 * v.0 - changed to be real pin D2 to send data to VFD display and added delay to be sure init the firmware from display
*/

#include <NixdorfVFD.h>  // https://github.com/MrTransistorsChannel/NixdorfVFD
#include <SoftwareSerial.h>

NixdorfVFD VFD;
SoftwareSerial vfd(3, 2, true); // SoftwareSerial port on pin #2, inverted logic

void setup() {
  delay(5000);
  vfd.begin(9600);              // Starting serial communication at 9600bod
  VFD.begin(vfd);               // Initialising the display
  VFD.clear();                  // Clearing screen
  VFD.home();                   // Returning cursor to the start position
  VFD.setCodePage(CP_866);      // Enabling cyrillic code page

  VFD.setCursor(4, 0);          // Moving cursor to 5 column in the first row
  VFD.print("Hello world!");    // Printing text
  VFD.setCursor(4, 1);
  VFD.print("Привет, мир!");

}

void loop() {
  // Nothing here
}

#include <SoftwareSerial.h>

NixdorfVFD VFD;
SoftwareSerial vfd(3, 2, true); // SoftwareSerial port on pins #2 and #3, inverted logic

void setup() {
  delay(5000);
  vfd.begin(9600);              // Starting serial communication at 9600bod
  VFD.begin(vfd);               // Initialising the display
  VFD.clear();                  // Clearing screen
  VFD.home();                   // Returning cursor to the start position
  VFD.setCodePage(CP_866);      // Enabling cyrillic code page

  VFD.setCursor(4, 0);          // Moving cursor to 5 column in the first row
  VFD.print("Hello world!");    // Printing text
  VFD.setCursor(4, 1);
  VFD.print("Привет, мир!");

}

void loop() {
  // Nothing here
}
