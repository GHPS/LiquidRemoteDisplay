#include <Wire.h> 

#define LCD1_SR
/* Supported displays
//   - Parallel display:         LCD1_4BIT
     - Serial I2C display:       LCD1_I2C
     - Serial I2C ByVac display: LCD1_I2C_ByVac
*/

#include <IRremote.h>
#ifdef LCD1_4BIT
  #include <LiquidCrystal.h>
#endif

#ifdef LCD1_I2C
  #include <LiquidCrystal_I2C.h>
#endif

#ifdef LCD1_I2C_ByVac
  #include <LiquidCrystal_I2C_ByVac.h>
#endif

#ifdef LCD1_SR
  #include <LiquidCrystal_SR.h>
#endif


#define          backlight_pin     5
#define          contrast_pin      6
#define	         lcd_1_maxX       20
#define		 lcd_1_maxY       4

int RECV_PIN = 17;

IRrecv irrecv(RECV_PIN);
decode_results results;

#ifdef LCD1_4BIT
  LiquidCrystal lcd_1(4, 7, 8, 9, 10, 11, 12,5,POSITIVE);
#endif

#ifdef LCD1_I2C
  LiquidCrystal_I2C lcd_1(0x27,2,1,0,4,5,6,7,3,POSITIVE);
#endif

#ifdef LCD1_I2C_ByVac
  LiquidCrystal_I2C_ByVac lcd_1(0x21); 
#endif

#ifdef LCD1_SR
  LiquidCrystal_SR lcd_1(14,15,4);
#endif


long int lastKeycode =0;
byte currentBrightness=255;
byte currentContrast=205;

void setup()
{
  lcd_1.begin(lcd_1_maxX,lcd_1_maxY);
  lcd_1.clear();
  irrecv.enableIRIn(); // Start the receiver
  pinMode(contrast_pin, OUTPUT);
  pinMode(backlight_pin, OUTPUT);
  lcd_1.setBacklight(currentBrightness);
  analogWrite(backlight_pin, currentBrightness);
  analogWrite(contrast_pin, 255-currentContrast);
  lcd_1.print("Sketch 'getKeycode'");
  lcd_1.setCursor(0,1); lcd_1.print("Press remote key!");
}

void loop() {
  if (irrecv.decode(&results)) {
    lastKeycode=results.value;
    if (lastKeycode!=0) {
      lcd_1.setCursor(0,1);
      lcd_1.print("Keycode:0x"); lcd_1.print(lastKeycode, HEX); lcd_1.print("    ");
    }
    irrecv.resume();
  }
}
