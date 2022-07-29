#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SimpleTimer.h>
#include <SPI.h>
#include <SD.h>

SimpleTimer secondTimer;
SimpleTimer minuteTimer;
SimpleTimer tenMinuteTimer;

File myFile;

Adafruit_ADS1115 ads;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
 
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 long timeStart = 0;
 long minCount = 0;
 int16_t adc0, adc1, adc2, adc3;
 int starting = 0;
 
void setup(void) 
{
  Serial.begin(9600);
  Wire.begin();

  Serial.print("Initializing SD card...");

  if (!SD.begin(15)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");


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

  secondTimer.setInterval(2000L, Oled_Display);
  minuteTimer.setInterval(60000L, nextMin);
  tenMinuteTimer.setInterval(60000L, writeData);
  timeStart = millis();

}

void loop(void) {

  if (starting == 0){
    starting = 1;
    //open file
    myFile=SD.open("DATA.csv", FILE_WRITE);

    // if the file opened ok, write to it:
    if (myFile) {
      Serial.println("File opened ok");
      // print the headings for our data
      myFile.println("Minutes Elapsed,Reading 1(mV),Reading 2(mV)");
    }
    myFile.close(); 
  }
  secondTimer.run();
  minuteTimer.run();
  tenMinuteTimer.run();
}

void Oled_Display()
{
  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0); // column row
  display.print("A0  d:");
  display.setTextSize(1);
  display.setCursor(45, 0);
  display.print(adc0);
  display.setTextSize(1);
  display.setCursor(0,10); // column row
  display.print("A0 mV:");
  display.setTextSize(1);
  display.setCursor(45, 10);
  display.print(adc0*0.125);

  display.setTextSize(1);
  display.setCursor(0,20); // column row
  display.print("A1  d:");
  display.setTextSize(1);
  display.setCursor(45, 20);
  display.print(adc1);
  display.setTextSize(1);
  display.setCursor(0,30); // column row
  display.print("A1 mV:");
  display.setTextSize(1);
  display.setCursor(45, 30);
  display.print(adc1*0.125);

  display.setTextSize(1);
  display.setCursor(0,40);
  display.print("Time:");
  display.setTextSize(1);
  display.setCursor(35, 40);
  display.print((millis()-timeStart)/1000);    

  display.setTextSize(1);
  display.setCursor(0,50);
  display.print("Min:");
  display.setTextSize(1);
  display.setCursor(35, 50);
  display.print(minCount); 

  display.display();
  Serial.print("A0  d: ");
  Serial.println(adc0);
  Serial.print("A0 mV: ");
  Serial.println(adc0*0.125); 
  Serial.print("A1  d: ");
  Serial.println(adc1);
  Serial.print("A1 mV: ");
  Serial.println(adc1*0.125);
  Serial.print("Time: ");
  Serial.println((millis()-timeStart)/1000);
  Serial.print("Min: ");
  Serial.println(minCount);
  Serial.println("............................"); 
}

void nextMin(){
  minCount ++;
}

void writeData() {
  Serial.println("--------------------------");
  Serial.println("Writing to SD Card");
  Serial.println("--------------------------");
  //open file
  myFile=SD.open("DATA.csv", FILE_WRITE);

  // if the file opened ok, write to it:
  if (myFile) {
    myFile.print((millis()-timeStart)/60000);
    myFile.print(",");
    myFile.print(adc0*0.125);
    myFile.print(",");
    myFile.println(adc1*0.125);
    // myFile.print(",");        
  }
  myFile.close();  
}
