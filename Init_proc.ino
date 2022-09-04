#include <ezButton.h>
#include <Wire.h>
#include <RunningMedian.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SimpleTimer.h>
#include <SPI.h>
#include <SD.h>

SimpleTimer displayTimer;                        //Define timer for display update interval
SimpleTimer minuteTimer;                         //Define timer for minute counter
SimpleTimer writeDataTimer;                      //Define timer to write data to SD Card
RunningMedian medianVoltage = RunningMedian(10);
RunningMedian medianCurrent = RunningMedian(10);

// Define push button pins
ezButton setButton(0);
ezButton modButton(2);

File myFile;

Adafruit_ADS1115 ads;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
 
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// long timeStart = 0;
 long minCount = 0;                         //Define minute counter for displaying duration of test
 long secCount = 0;                         //Define seconds counter for saving interval to SD card
 int16_t adc01, adc23;                      //Define counters for readings from ADS1115
 int starting = 0;                          //Define flag for init procedure, if running for the first time
 int menuNumber = 0;                        //Define menu item used for display screen info
 float minBatInfo[2] = {2.5, 2.75};         //Define min allowable voltage per battery type
 float maxBatInfo[2] = {3.8 , 4.3};        //Define max allowable voltage per battery type
 int numBat = 0;                            //Define counter to loop through batCount array
 int batCount[4] = {1, 4, 8, 16};           //Define array containing number of battery string options
 bool batType = false;                      //Define flag for selecting between different types of batteries
 float instVolt, instAmp, instPow, volts, amps, minV, maxV, tempCurrent, tempVoltage, tempPower;                         //Define calculated values for voltage and current
 bool voltageError = false;
 bool currentError = false;
 String errorMessage = "";
 
void setup(void) 
{
  Serial.begin(9600);
  Wire.begin();
  pinMode(3, OUTPUT);
  delay(1000);
  digitalWrite(3, LOW);

  // Serial.print("Initializing SD card...");

  // if (!SD.begin(15)) {
  //   Serial.println("initialization failed!");
  //   while (1);
  // }
  // Serial.println("initialization done.");


  //                                                                ADS1115
  //                                                                -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit =  0.1875mV (default)    // activate this if you are using a 5V sensor, this one should  be used with Arduino boards
//  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit =  0.125mV               // As the sensor is powered up using 3.3V, this one should be used with 3.3v controller boards
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit =  0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit =  0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit =  0.015625mV
   ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit =  0.0078125mV
  
  ads.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);

  setButton.setDebounceTime(20); // set debounce time to 50 milliseconds
  modButton.setDebounceTime(20); // set debounce time to 50 milliseconds

  displayTimer.setInterval(300L, Oled_Display);         //Adjust update interval of display Timer ro 500ms
  minuteTimer.setInterval(60000L, nextMin);             //Adjust interval of minute Timer to 1min
  writeDataTimer.setInterval(1000L, writeData);         //Adjust the interval to write data to the SD Card to 1sec
//  tenmsTimer.setInterval(500L, saveMedian);             //Adjust the interval for 10ms interval samples for median calculation
  delay(500);

}

void loop(void) {
  if (SD.begin(15)) {
    menuNumber = 1;
  }
  displayTimer.run();
  setButton.loop();
  modButton.loop();
  if(setButton.isReleased()){
    if(menuNumber >= 4){
      Serial.println("SET button pressed befond MENU 4. Do nothing!!!");
    } else {
    menuNumber ++;
    Serial.print("Menu changed to ");
    Serial.println(menuNumber);
    }
    }
  if (modButton.isReleased()){                                             //MOD button press functions Start
      if (menuNumber == 1){                                               //
        batType = !batType;                                               //
        if (batType){                                                     //
          Serial.println("BAT changed from LPF to MNC");                  //
        }                                                                 //
        else {                                                            //
          Serial.println("BAT changed from MNC to LPF");                  //
        }                                                                 //
      }                                                                   //
      if (menuNumber == 2){                                               //
        if (numBat == 3){                                                 //
          numBat = 0;                                                     //
        } 
        else {
          numBat ++;
        }
        Serial.print("BAT number changed to ");                           //
        Serial.println(batCount[numBat]);                                 //
      }                                                                   //
  }                                                                       //MOD button press functions End
  if (menuNumber == 4){ 
    if (starting == 0){
      initSdCard();
    }
    minuteTimer.run();
    writeDataTimer.run();
    }
  if (voltageError == true) {
    menuNumber = 6;
    }
 
  if (currentError == true){
  menuNumber = 6;
  }
}
void Oled_Display(){
  readData();
  if (menuNumber == 0){
    digitalWrite(3, LOW);
    displayMenu0();}
  else if (menuNumber == 1){
    digitalWrite(3, LOW);
    displayMenu1();}
  else if (menuNumber == 2){
    digitalWrite(3, LOW);
    displayMenu2();}  
  else if (menuNumber == 3){
    digitalWrite(3, LOW);
    displayMenu3();} 
  else if (menuNumber == 4){
    saveMedian();
    isInRange();  
    digitalWrite(3, HIGH);
    displayMenu4();}
  else if (menuNumber == 5){
    digitalWrite(3, LOW);
    displayMenu5();}    
  else if (menuNumber == 6){
    digitalWrite(3, LOW);
    displayMenu6();} 
}

void nextMin(){
  minCount ++;
  Serial.println("nextMin function run");
}

void writeData() {
  secCount ++;

  Serial.println("Writing to SD Card");
  //open file
  myFile=SD.open("DATA.csv", FILE_WRITE);

  // if the file opened ok, write to it:
  if (myFile) {
    myFile.print(secCount);
    myFile.print(",");
    myFile.print(amps);
    myFile.print(",");
    myFile.print(volts);
    myFile.print(",");
    myFile.println(amps*volts);    
    // myFile.print(",");        
  }
  myFile.close();  
}

void initSdCard(){
      minV = minBatInfo[batType];
      maxV = maxBatInfo[batType];
      minV = minV * batCount[numBat];
      maxV = maxV * batCount[numBat];
      if (SD.exists("DATA.csv")) {
      Serial.println("DATA.csv exists.");
      SD.remove("DATA.csv");
      Serial.println("DATA.csv removed!");
    }
    myFile=SD.open("DATA.csv", FILE_WRITE);

    // if the file opened ok, write to it:
    if (myFile) {
      Serial.println("File opened ok");
      // print the headings for our data
      myFile.println("Time Elapsed(s),Amps(A),Volts(V), Power(W)");
    }
    myFile.close(); 
    minCount = 0;
    starting = 1;
}

void displayMenu0(){
  display.clearDisplay();
  display.setCursor(40,0);
  display.print("ERROR!!!");
  display.setCursor(0,10);
  display.print("SD Card Error!!!");
  display.setCursor(0,20);
  display.print("Please check SD Card");
  display.setCursor(0, 30);
  display.print("and re-insert");
  display.display();
  }

void displayMenu1(){
  display.clearDisplay();
  display.setCursor(40,0);
  display.print("PAGE 1");
  display.setCursor(0,10);
  display.print("Cell type Selected:");
  display.setCursor(0,20);
  if (batType){
    display.print("NMC");
  }
  else {
    display.print("LPF");
  }
  
  display.setCursor(0,30);
  display.print("Is this CORRECT?");    
  display.setCursor(0,40);
  display.print("MOD -> Change"); 
  display.setCursor(0,50);
  display.print("SET -> Next");   
  display.display();
}

void displayMenu2() {
  display.clearDisplay();
  display.setCursor(40,0);
  display.print("PAGE 2");
  display.setCursor(0,10);
  display.print("Number of Cells:");
  display.setCursor(40,20);
  display.print(batCount[numBat]);
  display.setCursor(0,30);
  display.print("MOD -> Change"); 
  display.setCursor(0,40);
  display.print("SET -> Next"); 
  display.display();  
}

void displayMenu3() {
  display.clearDisplay();
  display.setCursor(40,0);
  display.print("PAGE 3");
  display.setCursor(0,10);
  display.print("You have selected:");
  display.setCursor(0,20);
  display.print(batCount[numBat]);
  display.setCursor(15,20);
  display.print("x");    
  display.setCursor(25,20);
  if (batType == 0){display.print("LFP");}
  else {display.print("MNC");} 
  String tempVoltString = String(instVolt);
  tempVoltString.concat("V");
  String tempAmpString = String(instAmp);
  tempAmpString.concat("A");
  display.setCursor(0,30);
  display.print("RESTART -> Change"); 
  display.setCursor(0,40);
  display.print("SET -> Start"); 
  display.setCursor(10,50);
  display.print(tempVoltString);
  display.setCursor(70,50);
  display.print(tempAmpString);  
  display.display();  
}

void displayMenu3Error() {
  display.clearDisplay();
  String tempVoltString = String(instVolt);
  tempVoltString.concat("V");
  String tempAmpString = String(instAmp);
  tempAmpString.concat("A");  
  display.setCursor(0,0);
  display.print("Voltage reading=");
  display.setCursor(20,10);
  display.print(tempVoltString);   
  display.setCursor(80,10);
  display.print(tempAmpString);   
  display.setCursor(0,20);
  display.print(batCount[numBat]);
  display.setCursor(15,20);
  display.print("x");    
  display.setCursor(25,20);
  if (batType == 0){display.print("LFP");}
  else {display.print("MNC");} 
  display.setCursor(50,20);
  display.print("should read"); 
  display.setCursor(0,30);
  float minV = minBatInfo[batType];
  float maxV = maxBatInfo[batType];
  minV = minV * batCount[numBat];
  maxV = maxV * batCount[numBat];
  String rangeV = String(minV);
  rangeV.concat(" - ");
  rangeV.concat(String(maxV));
  rangeV.concat(" V");
  display.print(rangeV);   
  display.setCursor(0,40);
  display.print("RESTART AND");   
  display.setCursor(0,50);
  display.print("TRY AGAIN!!!");    
  display.display();  
}

void displayMenu4() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("RUNNING!!!:");
  display.setCursor(0,10);
  display.print("Voltage:");
  display.setCursor(55,10);
  String voltString = String(instVolt);
  voltString.concat(" V");
  display.print(voltString);
  display.setCursor(0,20);
  display.print("Current:");    
  display.setCursor(55,20);
  String ampString = String(instAmp);
  ampString.concat(" A");  
  display.print(ampString);
  display.setCursor(0,30);
  display.print("Power:");    
  display.setCursor(55,30);
  String powString = String(instPow);
  powString.concat(" W");  
  display.print(powString);   
  display.setCursor(0,40);
  display.print("Time:"); 
  display.setCursor(55,40);
  String timeString = String(minCount);
  timeString.concat(" min");    
  display.print(timeString);
  display.setCursor(0,50);
  display.print("SET -> Emergency Stop");   
  display.display();    
}

void displayMenu5() {
  display.clearDisplay();
  display.setCursor(40,0);
  display.print("DONE!!!");
  display.setCursor(0,10);
  display.print("Discharge complete.");
  display.setCursor(0,20);
  display.print("Total Time:");
  String timeString = String(minCount);
  timeString.concat(" min");   
  display.setCursor(70,20);
  display.print(timeString);    
  display.setCursor(0,30);
  display.print("Remove SD Card"); 
  display.setCursor(0,40);
  display.print("and save data"); 
  display.display();    
}

void displayMenu6() {
  display.clearDisplay();
  display.setCursor(40,0);
  display.print("FAIL!!!");
  display.setCursor(0,10);
  display.print("Discharge Terminated");
  display.setCursor(0,20);
  display.print("due to safety reasons");
  display.setCursor(0,30);
  display.print(errorMessage); 
  display.display();    
}

void readData(){
  instAmp = float(ads.readADC_Differential_0_1())/96;
  instVolt = float(ads.readADC_Differential_2_3())/96;
  instPow = instAmp*instVolt;
}

void saveMedian() {
  medianCurrent.add(instAmp);
  medianVoltage.add(instVolt);
}

void isInRange() {
  tempCurrent = medianCurrent.getMedian();
  tempVoltage = medianVoltage.getMedian();
  tempPower = tempCurrent*tempVoltage;
  if (tempVoltage < minV){
    if (menuNumber == 4){
      Serial.println("Test Complete");
      menuNumber = 5;
    } else{
    Serial.println("Voltage below limit");
    errorMessage = "Voltage ";
    errorMessage.concat(String(tempVoltage));
    errorMessage.concat(" < ");
    errorMessage.concat(minV);
    voltageError = true;
    }
  }
  if (tempVoltage > maxV){
    Serial.println("Voltage above limit");
    errorMessage = "Voltage ";
    errorMessage.concat(String(tempVoltage));
    errorMessage.concat(" > ");
    errorMessage.concat(maxV);
    voltageError = true;
  }  
  if (tempPower > 400){
    Serial.println("Power above limit");
    errorMessage = "Power ";
    errorMessage.concat(String(tempPower));
    errorMessage.concat(" > ");
    errorMessage.concat("400");
    currentError = true;
}
  if (tempCurrent > 100){
    Serial.println("Current above limit");
    errorMessage = "Current ";
    errorMessage.concat(String(tempCurrent));
    errorMessage.concat(" > ");
    errorMessage.concat("100");
    currentError = true;
}
}
