#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SimpleTimer.h>

SimpleTimer timer1;

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
// Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
 
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

 int16_t adc0, adc1, adc2, adc3;
 
void setup(void) 
{
  Serial.begin(9600);
  Wire.begin();
  
  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)    // activate this if you are using a 5V sensor, this one should  be used with Arduino boards
    //  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV               // As the sensor is powered up using 3.3V, this one should be used with 3.3v controller boards
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  
  ads.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);

  timer1.setInterval(1000L, Oled_Display);
}

void loop(void) 
{
 
timer1.run();
  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);
  adc3 = ads.readADC_SingleEnded(3);
  //adc3 = ads.readADC_SingleEnded(3);
  Serial.print("AIN0: "); Serial.println(adc0);
  Serial.print("AIN1: "); Serial.println(adc1);
  Serial.print("AIN2: "); Serial.println(adc2);
 Serial.print("AIN3: "); Serial.println(adc3);
  Serial.println(" ");
  
  delay(1000);
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
  display.setCursor(20, 0);
  display.print(ads.computeVolts(adc0));


    display.setTextSize(1);
  display.setCursor(0,10);
  display.print("A1:");

  display.setTextSize(1);
  display.setCursor(20, 10);
  display.print(ads.computeVolts(adc1));
 

    display.setTextSize(1);
  display.setCursor(0,40);
  display.print("A2:");

  display.setTextSize(1);
  display.setCursor(60, 40);
  display.print(ads.computeVolts(adc2));
 
    display.setTextSize(1);
  display.setCursor(0,60);
  display.print("A3:");

  display.setTextSize(1);
  display.setCursor(60, 60);
  display.print(ads.computeVolts(adc3));  

 display.display();
}