

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// For the Adafruit shield, these are the default.
#define cs 22
#define dc 21
#define rst 18
#define mosi 23
#define sck 19
#define miso 25
// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(cs, dc, mosi, sck, rst, miso);

void setup() {
  // Initialize TFT display
  tft.begin();
}

void loop() {
  // Call the functions here
  tft.setFont(NULL); // Set the font
  tft.fillRoundRect(0, 0, 50, 50, 10, ILI9341_RED); // Fill a rounded rectangle
  tft.setCursor(0, 0); // Set the cursor position
  tft.setTextColor(ILI9341_WHITE); // Set the text color
  tft.setTextSize(2); // Set the text size
  tft.printf("Hello, World!"); // Print formatted text
  tft.print("Hello, World!"); // Print text
  uint16_t bitmap[] = {0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000};
  tft.drawRGBBitmap(0, 0, bitmap, 8, 8); // Draw an RGB bitmap
}




Activity 