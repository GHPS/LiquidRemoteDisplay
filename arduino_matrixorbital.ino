/*
 arduino-matrixorbital.ino
 A software emulator for Matrix Orbital character display commands on Arduino.
 
 V1.5 28/12/2011
 Added fixes by yosoyzenitram, prettified code a little and added all Matrix Obrital commands for display model LK204-25.
 
 V1.0 6/2/2010
 Coded by giannoug <giannoug@gmail.com>
 Based on code by nuelectronics <http://www.nuelectronics.com/>
 
 You can use whatever screen size you want, but you will
 have to make proper adjustments to both this file and at
 LCDSmartie's configuration menu or whatever program you
 might use.
 
 Matrix Orbital LK204-25 manual (for command reference):
 http://www.matrixorbital.ca/manuals/LK_Series/LK204-25/LVK204-25%20%28Rev1.3%29.pdf
 
 The circuit (make sure to power the LCD):
 * LCD RS pin to digital pin 12
 * LCD E pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD Ve pin (contrast) to digital pin 6
 
 */

#include <LiquidCrystal.h>

// You can define your own pins here.
#define RS 12
#define E  11
#define D4 5
#define D5 4
#define D6 3
#define D7 2
#define BR 6

#define VERSION 1

#define GPO1 8 // Not yet implemented. (pg. 18 of the manual)

LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

void setup() {
  Serial.begin(19200); // Default baudrate.
  lcd.begin(16, 2); // Change this for other screen sizes.

  analogWrite(BR, 0); // Set maximum brightness.

  lcd.print("Arduino");
  lcd.setCursor(1, 1);
  lcd.print("Matrix Orbital!");
}

void loop() {
  byte rxbyte = serial_getch(); // Command
  byte temp; // Parameter

  if (rxbyte == 254) {
    switch (serial_getch()) {
    case 71: // Set cursor position (2 parameters, column, row)
      lcd.setCursor(serial_getch() - 1, serial_getch() - 1);
      break;
    case 72: // Cursor home (doesn't clear the screen!)
      //lcd.home();
      lcd.setCursor(0,0);
      break;
    case 74: // Underline cursor on
      lcd.cursor();
      break;
    case 75: // Underline cursor off
      lcd.noCursor();
      break;
    case 83: // Block cursor on
      lcd.blink();
      break;
    case 84: // Block cursor off
      lcd.noBlink();
      break;
    case 76: // Move cursor left
      lcd.command(16);
      break;
    case 77: // Move cursor right
      lcd.command(20);
      break;
    case 78: // Define custom character (2 parameters, id, data)
      lcd.command(64 + (serial_getch() * 8));
      for (temp = 7; temp != 0; temp--) {
        lcd.print(serial_getch());
      }
      break;
      //case 35: // Read serial number
      //case 36: // Read version number
    case 54: // Read version number
      Serial.print(VERSION);
      break;
    case 55: // Read module type
      Serial.print(9); // 9 for LK204-25
      break;
    case 80: // Set contrast (1 parameter, contrast)
      analogWrite(BR, 0xFF-serial_getch());
      break;
    case 81: // Auto scroll on
      lcd.autoscroll();
      break;
    case 82: // Auto scroll off
      lcd.noAutoscroll();
      break;
    case 86: // General Purpose Output off (1 parameter, number)
      //temp = serial_getch();
      //if (temp == 1) {
      //  digitalWrite(GPO1, LOW);
      //}
      break;
    case 87: // General Purpose Output on (1 parameter, number)
      //temp = serial_getch();
      //if (temp == 0) {
      //  digitalWrite(GPO1, HIGH);
      //}
      break;
    case 88: // Clear screen
      lcd.clear();
      break;
    case 35: // Place large number
    case 38: // Poll key presses
    case 51: // Change I2C slave address (1 parameter, address)
    case 57: // Change baud rate (1 parameter, baud rate)
    case 59: // Exit flow-control mode
    case 61: // Place vertical bar (2 parameters, column, length)
    case 64: // Change the startup screen
    case 65: // Auto transmit keypresses on
    case 67: // Auto line wrap on
    case 68: // Auto line wrap off
    case 66: // Backlight on (1 parameter, minutes)
    case 69: // Clear key buffer
    case 70: // Backlight off
    case 79: // Auto transmit keypress off
    case 85: // Set debounce time (1 paramater, time)
    case 96: // Auto repeat mode off
    case 152: // Set and save brightness (1 parameter, brightness)
    case 153: // Set backlight brightness (1 parameter, brightness)
    case 104: // Initialize horizontal bar
    case 109: // Initialize medium number
    case 110: // Initialize lange numbers
    case 111: // Place medium numbers
    case 115: // Initialize narrow vertical bar
    case 118: // Initialize wide vertical bar
    case 124: // Place horizontal bar graph (4 parameters, column, row, direction, length)
    case 126: // Set auto repeat mode (1 parameter, mode)
    case 145: // Set and save contrast (1 parameter, contrast)
    case 160: // Transmission protocol select (1 parameter, protocol)
    case 192: // Load custom characters (1 parameter, bank)
    case 164: // Setting a non-standart baudrate (1 parameter, speed)
    case 193: // Save custom character (3 parameters, bank, id, data)
    case 194: // Save startup screen custom characters (2 parameters, id, data)
    case 195: // Set startup GPO state (2 parameters, number, state)
    case 200: // Dallas 1-Wire
      switch (serial_getch()) {
      case 1: // Dallas 1-Wire transaction (6 parameters, flags, send, bits, receive, bits, data)
      case 2: // Search for a 1-Wire device
      default:
        break;
      }
      break;
    case 213: // Assign keypad codes
    default:
      /* All other commands are ignored 
       and parameter byte is discarded. */
      temp = serial_getch();
      break;
    }
    return; // Stop and start again.
  }

  // Otherwise its a plain char so we print it to the LCD.
  lcd.write(rxbyte);
}

/*
 * Waits for a byte to be available, reads and returns it.
 */
byte serial_getch() {
  while (Serial.available() == 0);

  return Serial.read();
}


