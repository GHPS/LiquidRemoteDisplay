/*  LiquidRemoteDisplay - Version 3.0
 A software emulator for Matrix Orbital LC display and keypad commands on Arduino.

 License: Open Source - GNU GPL v3

 You can use whatever screen size you want but you will
 have to make proper adjustments to both this file and at
 LCDSmartie's configuration menu (or whatever program you
 might use).

 Further information on setup, schematics, FAQs can be found in the LiquidRemoteDisplay Wiki
 http://github.com/GHPS/LiquidRemoteDisplay/wiki

 */

//#include "LiquidRemoteDisplay.h" // necessary only in Eclipse
#include "Wire.h"
#include "IRremote.h"

#define lcd_2 lcd_1   // use only one display
#define LCD_1_I2C_ByVac
/* Supported displays
 - Parallel display:         LCD_1_4BIT
 - Serial I2C display:       LCD_1_I2C
 - Serial I2C ByVac display: LCD_1_I2C_ByVac
 - Shift Register:           LCD_1_SR
 */

#ifdef LCD_1_4BIT
#include <LiquidCrystal.h>
#endif

#ifdef LCD_1_I2C
#include <LiquidCrystal_I2C.h>
#endif

#ifdef LCD_1_I2C_ByVac
#include <LiquidCrystal_I2C_ByVac.h>
#endif

#ifdef LCD_1_SR
#include <LiquidCrystal_SR.h>
#endif

//#include "SoftwareSerial.h"
//SoftwareSerial Serialport(0, 1); // Don't use SoftwareSerial on Pins 1/0 on Arduino Nano
#define Serialport Serial

#define version_number 3

#ifdef LCD_1_4BIT
#define          contrast_pin      6
#endif

#define		expander_address B0100000
#define		lcd_1_maxX       20
#define		lcd_1_maxY       4
#define		virtual_maxY     2              // With which line does the next screen in LCD Smartie -> 5 for just one screen
#define		IRin_pin         A3
// byte     IRout_pin       =  3;  // IRReceiver is hard-wired on 3, just to remember...

IRrecv irrecv(IRin_pin);
decode_results IRresults;

#ifdef LCD_1_4BIT
LiquidCrystal lcd_1(4, 7, 8, 9, 10, 11, 12,5,POSITIVE);
#endif

#ifdef LCD_1_I2C
LiquidCrystal_I2C lcd_1(0x27,2,1,0,4,5,6,7,3,POSITIVE);
#endif

#ifdef LCD_1_I2C_ByVac
LiquidCrystal_I2C_ByVac lcd_1(0x21);
#endif

#ifdef LCD_1_SR
LiquidCrystal_SR lcd_1(14,15,4);
#endif

byte x = 0, y = 0;
byte firstByte;
byte id;
byte GPO = 0;
byte currentBrightness,
currentContrast;
bool autoTransmit = true; // just is case the init sequence got lost...
unsigned long keyTimer;
char keyBuffer[8];

byte newCharacter[8] =
{ 0x0, 0x1b, 0xe, 0x4, 0xe, 0x1b, 0x0 }; // array for custom characters -> by default a cross to see any errors

uint8_t fullBlock[8] =
{ B00000, B11111, B11111, B11111, B11111, B11111, B00000, B00000 };

void appendChar(char* inputString, char appendCharacter)
{
	byte length = strlen(inputString);
	inputString[length] = appendCharacter;
	inputString[length + 1] = '\0';
}

void sendKeyBuffer()
{
	byte index = 0;
	Serialport.print(keyBuffer[0]);
	while ((keyBuffer[index] != 0) && (index < 8))
	{
		keyBuffer[index] = keyBuffer[index + 1];
		index++;
	}
}

void generalPurposeOut(byte address, byte mode)
{
	Wire.beginTransmission(expander_address);
	bitWrite(GPO, address - 1, mode); // The first bit is address number 1
	Wire.write(GPO xor B11111111);
	Wire.endTransmission();
}

void toggleGPO(byte address)
{
	generalPurposeOut(address, bitRead(GPO, address - 1) xor B1);
}

void updateKeys()
{
	char sendKey = ' ';
	if (irrecv.decode(&IRresults))
	{
		if (abs(millis() - keyTimer) > 250)
		{
			keyTimer = millis();
			switch (IRresults.value)
			{
				case 0x870: // Arrow right
					sendKey = 'R';
					break;
				case 0x470: // Arrow left
					sendKey = 'L';
					break;
				case 0x2F0: // Arrow up
					sendKey = 'U';
					break;
				case 0xAF0: // Arrow down
					sendKey = 'D';
					break;
				case 0x90: // Program up
					sendKey = ' ';
					break;
				case 0x890: // Program down
					sendKey = ' ';
					break;
				case 0x10: // Key "One"
					sendKey = '1';
					break;
				case 0x810: // Key "Two"
					sendKey = '2';
					break;
				case 0x410: // Key "Three"
					sendKey = '3';
					break;
				case 0xC10: // Key "Four"
					sendKey = '4';
					break;
				case 0x210: // Key "Five"
					sendKey = '5';
					break;
				case 0xA10: // Key "Six"
					sendKey = '6';
					break;
				case 0x610: // Key "Seven"
					sendKey = '7';
					break;
				case 0xE10: // Key "Eight"
					sendKey = '8';
					break;
				case 0x110: // Key "Nine"
					sendKey = '9';
					break;
				case 0x910: // Key "Zero"
					sendKey = '0';
					break;
				case 0xA50: // Key "AV"
					sendKey = 'G';
					break;
				case 0xB9A: // Key "Record"
					sendKey = 'X';
					break;
				case 0x59A: // Key "Play"
					sendKey = 'P';
					break;
				case 0x19A: // Key "Stop"
					sendKey = 'S';
					break;
				case 0x39A: // Key "Fast Forward"
					sendKey = 'N';
					break;
				case 0xD9A: // Key "Fast Rewind"
					sendKey = 'V';
					break;
				case 0x99A: // Key "Pause/Play"
					sendKey = 'H';
					break;
				case 0xA90: // Key "Power Off"
					sendKey = 'O';
					break;
				case 0x290: // Key "Mute"
					sendKey = 'M';
					break;
				case 0x490: // Key "Volume +"
					sendKey = 'K';
					break;
				case 0xC90: // Key "Volume -"
					sendKey = 'J';
					break; // Key Red
				case 0x338:
					sendKey = 'a';
					break;
				case 0xB38: // Key Green
					sendKey = 'b';
					break;
					// direct actions without PC intervention
					// case 0x738: sendKey='c'; break;  // Key Yellow
				case 0x738: // Key Yellow
					toggleGPO(3);
					break;
					// case 0xF38: sendKey='d'; break;  // Key Blue
				case 0xF38: // Key Blue
					toggleGPO(4);
					break;
				case 0x138: // Key Orange
					toggleGPO(5);
					break;
			}
			if (sendKey != ' ') appendChar(keyBuffer, sendKey);
		}
		irrecv.resume();
	}
}

byte serial_getch()
{
	while (Serialport.available() == 0)
	{
		updateKeys();
		if (autoTransmit && (strlen(keyBuffer) > 0)) sendKeyBuffer();
	}
	return (Serialport.read());
}

void executeCommand(byte commandCode)
{
	switch (commandCode)
	{
		case 0x23: // HEX 0x23 =DEC 35 Place large digit
			x = serial_getch() - 1; // Column
			y = serial_getch() - 1; // Digit
			break;
		case 0x36: // HEX 0x36 =DEC 54 Read version number
			Serialport.write(version_number);
			break;
		case 0x37: // HEX 0x37 =DEC 55 Read module type
			Serialport.print(0x09, HEX); // Matrix Display LK204-25 (20x4 LCD with keypad)
			break;
			/*	case 0x3b: // HEX 0x3b =DEC 59 Exit flow-control mode
			 break;*/
		case 0x3d: // HEX 0x3d =DEC 61 Draw vertical bar
			x = serial_getch() - 1; // Column
			y = serial_getch() - 1; // Height
			break;
		case 0x41: // HEX 0x41 =DEC 65 Auto transmit keypresses ON
			autoTransmit = true;
			break;
		case 0x42: // HEX 0x42 =DEC 66 Backlight ON
			x = serial_getch(); // get the time in minutes to stay on -> discarded
			lcd_1.setBacklight(currentBrightness);
			break;
		case 0x43: // HEX 0x43 =DEC 67 Auto line-wrap ON
			break;
		case 0x44: // HEX 0x44 =DEC 68 Auto line-wrap OFF
			break;
		case 0x45: // HEX 0x45 =DEC 69 Clear key buffer
			strcpy(keyBuffer, "");
			break;
		case 0x46: // HEX 0x46 =DEC 70 Backlight OFF
			lcd_1.noBacklight(); // currentBrightness unchanged
			break;
		case 0x47: //HEX 0x47 =DEC 71 Set cursor position
			x = serial_getch() - 1; //get column byte
			y = serial_getch() - 1; //get row byte
			if (y < virtual_maxY)
				lcd_1.setCursor(x, y);
			else
				lcd_2.setCursor(x, y);
			break;
		case 0x48: // HEX 0x48 =DEC 72 Cursor home (reset display position)
			lcd_1.home();
			break;
		case 0x4a: // HEX 0x4a =DEC 74 Underline cursor ON
			lcd_1.cursor();
			break;
		case 0x4b: // HEX 0x4b =DEC 75 Underline cursor OFF
			lcd_1.noCursor();
			break;
		case 0x4c: // HEX 0x4c =DEC 76 Move cursor left
			lcd_1.moveCursorLeft();
			x--;
			break;
		case 0x4d: // HEX 0x4d =DEC 77 Move cursor right
			lcd_1.moveCursorRight();
			x++;
			break;
		case 0x4e: // HEX 0x4E =DEC 78 Define custom character
			id = serial_getch();
			for (y = 0; y < 8; y++)
				newCharacter[y] = serial_getch(); // Get patterns byte from top to bottom
			lcd_1.createChar(id, newCharacter);
			break;
		case 0x4f: // HEX 0x4f =DEC 79 Auto transmit keypresses OFF
			autoTransmit = false;
			break;
		case 0x50: //HEX 0x50 =DEC 80 Set contrast
			currentContrast = serial_getch();
#ifdef LCD_1_4BIT
			analogWrite(contrast_pin, 255-currentContrast);
#endif
#ifdef LCD_1_I2C_ByVac
			lcd_1.setContrast(255 - currentContrast);
#endif
			break;
		case 0x51: // HEX 0x51 =DEC 81 Auto scroll ON
			lcd_1.autoscroll();
			break;
		case 0x52: // HEX 0x52 =DEC 82 Auto scroll OFF
			lcd_1.noAutoscroll();
			break;
		case 0x53: // HEX 0x53 =DEC 83 Block cursor ON
			lcd_1.blink();
			break;
		case 0x54: // HEX 0x54 =DEC 84 Block cursor OFF
			lcd_1.noBlink();
			break;
		case 0x56: //HEX 0x56 =DEC 86 General purpose output OFF
			id = serial_getch();
			generalPurposeOut(id, 0);
			break;
		case 0x57: // HEX 0x57 =DEC 87 General purpose output ON
			id = serial_getch();
			generalPurposeOut(id, 1); // The first bit is number 0
			break;
		case 0x58: // HEX 0x58 =DEC 88 Clear display and move cursor home
			lcd_1.clear();
			break;
		case 0x60: // HEX 0x96 =DEC 96 Auto-repeat mode OFF (keypad)
			break;
		case 0x68: // HEX 0x69 =DEC 104 Init horiz bar graph
			break;
		case 0x6e: // HEX 0x6e =DEC 110 Large digits
			break;
		case 0x6d: // HEX 0x6d=DEC 109  Init med size digits
			break;
		case 0x73: // HEX 0x73 =DEC 115 Init narrow vert bar graph
			break;
		case 0x76: // HEX 0x76 =DEC 118 Init wide vert bar graph
			break;
		case 0x7c: // HEX 0x7c =DEC 124 Draw horizontal bar graph
			x = serial_getch() - 1; // Column
			y = serial_getch() - 1; // Row
			x = serial_getch() - 1; // Direction
			y = serial_getch() - 1; // Length
			break;
		case 0x98: // HEX 0x98 =DEC 152 Set and remember backlight
			currentBrightness = serial_getch();
			lcd_1.setBacklight(currentBrightness);
			break;
		case 0x99: // HEX 0x99 =DEC 153 Set backlight brightness
			currentBrightness = serial_getch();
			lcd_1.setBacklight(currentBrightness);
			break;
		default:
			//all other commands ignored and parameter byte discarded
			x = serial_getch(); // Dump the command code
			break;
	}
}

byte characterSubstitution(byte character) // Substitution for a HD44780 with standard ROM Code A00 (the limited foreign character set)
{
	byte returnCharacter = character;
	if (character >= 0x5C) switch (character)
	{
		case 0x5C:
			returnCharacter = 0x60;
			break; // Backslash
	case 0xA3:
		returnCharacter = 0xED;
		break; // British pound-sterling
	case 0xB5:
		returnCharacter = 0xE4;
		break; // Greek "mu"
		// A-like characters mapped to plain A
	case 0xC0:
	case 0xC1:
	case 0xC2:
	case 0xC3:
	case 0xC4:
	case 0xC5:
		returnCharacter = 0x41;
		break;
		// E-like characters mapped to plain E
	case 0xC8:
	case 0xC9:
	case 0xCA:
	case 0xCB:
		returnCharacter = 0x45;
		break;
		// I-like characters mapped to plain I
	case 0xCC:
	case 0xCD:
	case 0xCE:
	case 0xCF:
		returnCharacter = 0x49;
		break;
	case 0xD1:
		returnCharacter = 0x43;
		break; // Spanish N+~ (tilde) -> plain "N"
		// O-like characters mapped to plain O
	case 0xD2:
	case 0xD3:
	case 0xD4:
	case 0xD5:
	case 0xD6:
	case 0xD8:
		returnCharacter = 0x4F;
		break;
		// U-like characters mapped to plain U
	case 0xD9:
	case 0xDA:
	case 0xDB:
	case 0xDC:
		returnCharacter = 0x55;
		break;
	case 0xDD:
		returnCharacter = 0x59;
		break; //"Y" acute -> "Y"
	case 0xDF:
		returnCharacter = 0xE2;
		break; // German sharp-s -> beta sign "ß"
		// a-like characters mapped to plain a
	case 0xE0:
	case 0xE1:
	case 0xE2:
	case 0xE3:
	case 0xE5:
		returnCharacter = 0x61;
		break;
	case 0xE4:
		returnCharacter = 0xE1;
		break; // German ä(a+") (Umlaut)
	case 0xE7:
		returnCharacter = 0x63;
		break; //"c" cedilla -> "c"
		// e-like characters mapped to plain e
	case 0xE8:
	case 0xE9:
	case 0xEA:
	case 0xEB:
		returnCharacter = 0x65;
		break;
		// i-like characters mapped to plain i
	case 0xEC:
	case 0xED:
	case 0xEE:
	case 0xEF:
		returnCharacter = 0x69;
		break;
	case 0xF1:
		returnCharacter = 0xEE;
		break; // Spanish n+~ (tilde)
		// o-like characters mapped to plain o
	case 0xF2:
	case 0xF3:
	case 0xF4:
	case 0xF5:
	case 0xF8:
		returnCharacter = 0x6F;
		break;
	case 0xF6:
		returnCharacter = 0xEF;
		break; // German ö(o+") (Umlaut)
	case 0xF7:
		returnCharacter = 0xFD;
		break; // Division symbol
		// u-like characters mapped to plain u
	case 0xF9:
	case 0xFA:
	case 0xFB:
		returnCharacter = 0x75;
		break;
	case 0xFC:
		returnCharacter = 0xF5;
		break; // German ü(u+") (Umlaut)
}
	return returnCharacter;
}

void printCharacter(byte character)
{
	if (y < virtual_maxY)
		lcd_1.write(characterSubstitution(character));
	else
		lcd_2.write(characterSubstitution(character));
	x++;
}

void setup()
{
	irrecv.enableIRIn();
	irrecv.blink13(true);
	Wire.begin();
	lcd_1.begin(lcd_1_maxX, lcd_1_maxY);
	lcd_1.clear();
	lcd_1.createChar(1, fullBlock);
	keyTimer = millis();
	strcpy(keyBuffer, "");
#ifdef LCD_1_4BIT
	pinMode(contrast_pin, OUTPUT);
	currentContrast=205;
	currentBrightness=255;
	analogWrite(contrast_pin, 255-currentContrast);
	lcd_1.setBacklight(currentBrightness);
#endif
	generalPurposeOut(1, 0); // reset all lines since GPO=0
	Serialport.begin(19200);
	strcpy(keyBuffer,"2    1"); // switch the screen twice to restore the screen after auto-reset from bootloader -> set LCD Smartie actions accordingly
}

void loop()
{
	firstByte = serial_getch();
	if (firstByte != 0xFE)
		printCharacter(firstByte); // Matrix Orbital displays use 'FE' as prefix for commands
	else
		executeCommand(serial_getch());
}
