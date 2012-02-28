/*  Matrixino -
A software emulator for Matrix Orbital LC display commands on Arduino.

 V2.0 28/02/2012 by GHPS
   - adapted to new, unified LiquidCrystal library
      -> http://bitbucket.org/fmalpartida/new-liquidcrystal
      -> supports all kinds of connection (parallel, I2C, SR)
         and should support even multiple displays 
  - fixed startup and shutdown commands 
    -> all commands implemented now
  - supports contrast and brightness with parallel displays
  - supports the standard block character
  - fixed character redefinition
    -> all "higher" LCDSmartie plugins now work (BigNum, Winamp)
   - fixed GPO routines -> no more dumps to shutdown screen
 
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
 
*/

#include <Wire.h> 

#include <LiquidCrystal.h>
LiquidCrystal lcd_1(4, 7, 8, 9, 10, 11, 12);

//#include <SoftwareSerial.h>
//SoftwareSerial Serialport(0, 1);
#define Serialport Serial

#define version_number 1
#define          backlight_pin     5
#define          contrast_pin      6
#define          expander_address B0100000
#define	         lcd_1_maxX       20
#define		 lcd_1_maxY       4

byte x = 0;
byte y = 0;
byte firstByte;
byte temp;
byte id;
byte GPO       = 0;
byte currentBrightness;
byte currentContrast;
boolean sendKey      = true;
boolean autoTransmit =false;
unsigned long  keyTimer;

uint8_t newCharacter[8]  = {0x0,0x1b,0xe,0x4,0xe,0x1b,0x0}; // array for custom characters -> by default a cross to see any errors

uint8_t fullBlock[8] =
{
	B00000,
	B11111,
	B11111,
	B11111,
	B11111,
	B11111,
	B00000,
	B00000
};

void setup() {
  Wire.begin(); 
  keyTimer=millis();
  lcd_1.begin(lcd_1_maxX,lcd_1_maxY);
  lcd_1.clear();
  lcd_1.createChar(1,fullBlock);
  pinMode(backlight_pin, OUTPUT);
  pinMode(contrast_pin, OUTPUT);
  currentBrightness=255;
  currentContrast=205;
  analogWrite(backlight_pin, currentBrightness);
  analogWrite(contrast_pin, 255-currentContrast);
  generalPurposeOut(1,0); // reset all lines since GPO=0
  Serialport.begin(19200);
}

void updateKeys()
{ 		
}

byte serial_getch(){
  while (Serialport.available()==0) updateKeys();
  return (Serialport.read());
}

void generalPurposeOut(byte address, byte mode)
{
	Wire.beginTransmission(expander_address);
	bitWrite(GPO,address,mode);
	Wire.write(GPO xor B11111111);
	Wire.endTransmission();  
}

void executeCommand(byte commandCode)
{
	switch (commandCode)
	{
	case 0x23: // HEX 0x23 =DEC 35 Place large digit
		x=serial_getch()-1;   //column
		y=serial_getch()-1;   //digit
		break;
	case 0x36: // HEX 0x36 =DEC 54 Read version number
		Serialport.write(version_number);
		break;   
	case 0x37: // HEX 0x37 =DEC 55 Read module type
		Serialport.print(0x09, HEX); // Matrix Display LK204-25 (20x4 LCD with keypad)
		break;
/*	case 59: // HEX 0x =DEC 59 Exit flow-control mode
		break;*/
	case 0x3d: // HEX 0x3d =DEC 61 Draw vertical bar
		x=serial_getch()-1;   //column
		y=serial_getch()-1;   //height
		break;
	case 0x41: // HEX 0x41 =DEC 65 Auto transmit keypresses ON
                autoTransmit=true;
		break;
	case 0x42: // HEX 0x42 =DEC 66 Backlight ON
		temp=serial_getch(); // get the time in minutes to stay on -> discarded
		analogWrite(backlight_pin, currentBrightness);
		break;
	case 0x43: // HEX 0x43 =DEC 67 Auto line-wrap ON
		break;
	case 0x44: // HEX 0x44 =DEC 68 Auto line-wrap OFF    
		break;
	case 0x45: // HEX 0x45 =DEC 69 Clear key buffer
		break;
	case 0x46: // HEX 0x46 =DEC 70 Backlight OFF
		analogWrite(backlight_pin, 0); // currentBrightness unchanged
		break;
	case 0x47:  //HEX 0x47 =DEC 71 Set cursor position
		x=serial_getch()-1;   //get column byte
		y=serial_getch()-1;   //get row byte
                lcd_1.setCursor(x,y);
		break;
	case 0x48:  // HEX 0x48 =DEC 72 Cursor home (reset display position)
		lcd_1.home(); 
		break;
	case 0x4a:  // HEX 0x4a =DEC 74 Underline cursor ON
		lcd_1.cursor();
		break;
	case 0x4b:  // HEX 0x4b =DEC 75 Underline cursor OFF
		lcd_1.noCursor();
		break;
	case 0x4c:  // HEX 0x4c =DEC 76 Move cursor left
		lcd_1.moveCursorLeft();
		x--;
		break;
	case 0x4d:  // HEX 0x4d =DEC 77 Move cursor right
		lcd_1.moveCursorRight();
		x++;
		break;
	case 0x4e:  // HEX 0x4E =DEC 78 Define custom character
		id=serial_getch();
		for (temp=0; temp<8; temp++) newCharacter[temp]=serial_getch(); // Get patterns byte from top to bottom
		lcd_1.createChar(id,newCharacter);
		break;
	case 0x4f: // HEX 0x4f =DEC 79 Auto transmit keypresses OFF
                autoTransmit=false;
		break;
	case 0x50: //HEX 0x50 =DEC 80 Set contrast
		currentContrast=serial_getch();
		analogWrite(contrast_pin, 255-currentContrast);
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
	case 0x54:  // HEX 0x54 =DEC 84 Block cursor OFF
		lcd_1.noBlink();
		break;
	case 0x56:  //HEX 0x56 =DEC 86 General purpose output OFF
		id=serial_getch();
		generalPurposeOut(id-1,0);   // The first bit is number 0   
		break;
	case 0x57:  // HEX 0x57 =DEC 87 General purpose output ON
		id=serial_getch();
		generalPurposeOut(id-1,1);   // The first bit is number 0
		break;
	case 0x58:  // HEX 0x58 =DEC 88 Clear display and move cursor home
		lcd_1.clear();
		break;
	case 0x60: //HEX 0x96 =DEC Auto-repeat mode OFF (keypad)      
		break;
	case 0x68: // HEX 0x69 =DEC 104 Init horiz bar graph
		break;
	case 0x6e: // HEX 0x6e =DEC 110 Large digits      
		break;
	case 109: //HEX 0x =DEC   Init med size digits
		break;
	case 0x73: // HEX 0x =DEC 115 Init narrow vert bar graph
		break;
	case 0x76: // HEX 0x76 =DEC 118 Init wide vert bar graph
		break;
	case 0x7c: // HEX 0x7c =DEC 124 Draw horizontal bar graph
		x=serial_getch()-1;   //column
		y=serial_getch()-1;   //row
		x=serial_getch()-1;   //dir
		y=serial_getch()-1;   //lenght
		break;
	case 0x98: //HEX 0x =DEC  Set and remember backlight
		currentBrightness=serial_getch();
		analogWrite(backlight_pin, currentBrightness);
		break;
	case 0x99: //HEX 0x99 =DEC 153 Set backlight brightness
		currentBrightness=serial_getch();
		analogWrite(backlight_pin, currentBrightness);
		break;
	default:
		//all other commands ignored and parameter byte discarded
		temp=serial_getch();  //dump the command code
		break;
	}
	return;
}

byte characterSubstitution(byte character)
{
  byte returnCharacter=character;
  if (character>=0x80) 
      switch (character)
	{   
	case 0x80: returnCharacter=0xFF; break; // Full block
	case 0xA3: returnCharacter=0xED; break; // British pound-sterling
        case 0xB5: returnCharacter=0xE4; break; // Greek "mu"
	case 0xC0: // A-like characters
	case 0xC1:
	case 0xC2:
	case 0xC3:
	case 0xC4:
	case 0xC5: returnCharacter=0x41; break;
	case 0xC8: // E-like characters
	case 0xC9:
	case 0xCA:
	case 0xCB: returnCharacter=0x45; break;
	case 0xCC: // I-like characters
	case 0xCD:
	case 0xCE:
	case 0xCF: returnCharacter=0x49; break;
	case 0xD1: returnCharacter=0x43; break; // Spanish N+~ (tilde) -> plain "N"
	case 0xD2: // O-like characters
	case 0xD3:
	case 0xD4:
	case 0xD5:
	case 0xD6:
	case 0xD8: returnCharacter=0x4F; break;
	case 0xD9: // U-like characters
	case 0xDA:
	case 0xDB:
	case 0xDC: returnCharacter=0x55; break;
	case 0xDD: returnCharacter=0x59; break; //"Y" acute -> "Y"
	case 0xDF: returnCharacter=0xE2; break; // German sharp-s -> beta sign "ß" 
        case 0xE0: // a-like characters
	case 0xE1:
	case 0xE2:
	case 0xE3:
        case 0xE4: returnCharacter=0xE1; break; // German ä(a+") (Umlaut)
	case 0xE5: returnCharacter=0x61; break;
	case 0xE7: returnCharacter=0x63; break; //"c" cedilla -> "c"
	case 0xE8: // e-like characters
	case 0xE9:
	case 0xEA:
	case 0xEB: returnCharacter=0x65; break;
	case 0xEC: // i-like characters
	case 0xED:
	case 0xEE:
	case 0xEF: returnCharacter=0x69; break;
	case 0xF1: returnCharacter=0xEE; break; // Spanish n+~ (tilde)
	case 0xF2: // o-like characters
	case 0xF3:
	case 0xF4:
	case 0xF5:
	case 0xF6: returnCharacter=0xEF; break; // German ö(o+") (Umlaut)
	case 0xF8: returnCharacter=0x6F; break;
	case 0xF7: returnCharacter=0xFD; break; // Division symbol
	case 0xF9: // u-like characters
	case 0xFA:
	case 0xFB: returnCharacter=0x75; break;
	case 0xFC: returnCharacter=0xF5; break; // German ü(u+") (Umlaut)
	}
   return returnCharacter;
}

void printCharacter(byte character)
{
  lcd_1.write(characterSubstitution(character));
  x++;
}

void loop()
{ 
  firstByte=serial_getch();
  if (firstByte!=0xFE) printCharacter(firstByte);
    else executeCommand(serial_getch());                 // Matrix Orbital displays use 'FE' as prefix for commands
}