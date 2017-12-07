/* README
 *  - Use Arduino Uno to view Serial messages for debugging
 *  - Use the board manager to ad adafruit if they don't exist
 *  - If Adafruit not available in Board Manager, add URL to preferences on additional board locations
 *  - Arduino is C++. Easier to search for C++ solutions
 * Trinket: 
 *   Board - Trinket 5V/16Hz (USB)
 *   Programmer - USBTinyISP
 * 
*/
#include <Adafruit_NeoPixel.h>
#include "LightPattern.h"

#define PIN 4
#define LEDS 36

Pattern strand = Pattern(LEDS, PIN, NEO_GRB + NEO_KHZ800, &PatternComplete);

void setup() {
  Serial.begin(9600);
  strand.begin();
  strand.show();
  strand.Init(CHRISTMAS, 50);

  test();
}

void test() {
//  Test Patterns
//  strand.lockPattern = true;

//  Test Clap
//  strand.Clap(strand.Color(255, 0, 0), strand.Color(0,255,0), 50, 3);

//  Testing CircleFade pattern
//  strand.CircleFade(strand.Color(255, 0, 0), strand.Color(0,255,0), 50, 8, true);
}

void loop() {
  strand.Update();
}

void PatternComplete() {
  
}

