#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SimpleTimer.h>

SimpleTimer timer1;

Adafruit_ADS1115 ads;


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
 
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 long timeStart = 0;
 long lastMinCount = 0;
 long minCount = 0;
 long lastSecCount = 0;
 int16_t adc0, adc1, adc2, adc3;
 
void setup(void) 
{
  Serial.begin(9600);
  Wire.begin();

  //                                                                ADS1115
  //                                                                -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit =  0.1875mV (default)    // activate this if you are using a 5V sensor, this one should  be used with Arduino boards
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit =  0.125mV               // As the sensor is powered up using 3.3V, this one should be used with 3.3v controller boards
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit =  0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit =  0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit =  0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit =  0.0078125mV
  
  ads.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);

  timer1.setInterval(1000L, Oled_Display);
  timeStart = millis();
}

void loop(void) 
{
if (lastMinCount == 0) {                          //Init. Only runs after first boot
  lastMinCount = millis() + 60000;                //Set timer for 1min delay
  lastSecCount = millis() + 1000;                 //Set timer for 1sec delay for display update
}

if (millis() > lastMinCount) {                    //Check if minute timer has expired
  lastMinCount = millis() + 60000;                //Set timer for 1min delay
  minCount ++;                                    //Increment minute counter

}

if (millis() > lastSecCount) {                    //Check if second timer has expired
  lastSecCount = millis() + 1000;                 //Set timer for 1sec delay
  timer1.run();                                   
  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  Serial.print("A0: ");
  Serial.println(ads.computeVolts(adc0 + 2));
  Serial.print("A1: ");
  Serial.println(ads.computeVolts(adc1));
  Serial.print("Time: ");
  Serial.println((millis()-timeStart)/1000);
  Serial.print("Min: ");
  Serial.println(minCount);
  Serial.println("............................"); 

}


  
}

void Oled_Display()
{
     // display on Oled display

   // Oled display
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0); // column row
  display.print("A0:");
  display.setTextSize(1);
  display.setCursor(25, 0);
  display.print(ads.computeVolts(adc0 + 2));

  display.setTextSize(1);
  display.setCursor(0,10);
  display.print("A1:");
  display.setTextSize(1);
  display.setCursor(25, 10);
  display.print(ads.computeVolts(adc1));

  display.setTextSize(1);
  display.setCursor(0,20);
  display.print("Time:");
  display.setTextSize(1);
  display.setCursor(35, 20);
  display.print((millis()-timeStart)/1000);    

  display.setTextSize(1);
  display.setCursor(0,30);
  display.print("Min:");
  display.setTextSize(1);
  display.setCursor(35, 30);
  display.print(minCount); 

 display.display();
}