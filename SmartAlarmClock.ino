#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <time.h>
#include <Weather.h>

byte maxScreenPointer = 3;
byte screenPointer = 0;
LiquidCrystal_I2C lcd(0x27,16,2);


//Clock Setup
//=======================================================================
const char* ntpServer = "time.nist.gov";
const char* ntpServer2 = "pool.ntp.org";
const long gmtOffset = -7 * 3600; //In Seconds
const int daylightOffset = 3600; //In Seconds
time_t baseTime;
unsigned long baseMillis;

//WiFi Setup
//=======================================================================
const char* ssid     = "WiFi_Name"; //WiFi_Name
const char* password = "WiFi_Password"; //WiFi_Password

//PIN Setup
//=======================================================================
const byte buzzerPin = 14;
const byte buttonPins[] = {4, 5, 13, 18}; //{left=4, right=5, select=13, cycle=18}

//Button Variables
//=======================================================================
bool lastState4 = HIGH;
bool lastState5 = HIGH;
bool lastState13 = HIGH;
bool lastState18 = HIGH;

//Constant Strings for LCD
//=======================================================================
const char* prepScrn = "Set Prep Time:";
const char* flightScrn = "Set Flight Iata:";
const char* alarmScrn = "Set Alarm:";

//Timer Variables
//=======================================================================
unsigned long lastWeatherUpdate = 0;
unsigned long lastWeatherPrint = 0;


//Functions
//=======================================================================

/**
  Moves the screenPointer left if False or right if True.
*/
void moveScreen(bool direction){
  if(direction){
    screenPointer++;
    if(screenPointer > maxScreenPointer){ screenPointer = 0; }
  }
  else{
    if(screenPointer == 0){ screenPointer = maxScreenPointer; }
    else{ screenPointer--; }
  }
}

/**
  Button Listener:
  Performs Actions if buttons are pressed.
  NOTE: physical buttons are colored, RED, YELLOW, GREEN, & BLUE
*/
void readButtons() {
    for (byte i = 0; i < sizeof(buttonPins); i++) {
      byte buttonPin = buttonPins[i];
      if (digitalRead(buttonPin) == LOW) {
        //Button LOW State or On
        switch(buttonPin){
          case 4:
            //Left Button
            if(lastState4 == HIGH){
              Serial.println("Red");
            }
            lastState4 = LOW;
            break;

          case 5:
            //Right Button
            if(lastState5 == HIGH){
              Serial.println("Yellow");
            }
            lastState5 = LOW;
            break;

          case 13:
            //Select Button
            if(lastState13 == HIGH){
              Serial.println("Green");
            }
            lastState13 = LOW;
            break;

          case 18:
            //Cycle Button'
            if(lastState18 == HIGH){
              Serial.println("Blue");
              moveScreen(true);
              lcd.clear();
            }
            lastState18 = LOW;
            break;

          default:
            Serial.print("Button Pin Error");
            break;
        }
      }
      else{
        //Button HIGH State or Off
        switch(buttonPin){
          case 4:
            //Left Button
            //RED
            lastState4 = HIGH;
            break;

          case 5:
            //Right Button
            //Yellow
            lastState5 = HIGH;
            break;

          case 13:
            //Select Button
            //Green
            lastState13 = HIGH;
            break;

          case 18:
            //Cycle Button
            //Blue
            lastState18 = HIGH;
            break;

          default:
            Serial.print("Button Pin Error");
            break;
        }
      }
    }
}

/**
  Initializes ESP32 time.
*/
void initTime(){
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    time(&baseTime);
    baseMillis = millis();
  }
}

/**
  Returns current time.
*/
time_t getCurrentTime(){
  return baseTime + (millis() - baseMillis) / 1000;
}

/**
  Displays time on lcd screen.
*/
void clockScreen(){
  time_t now = getCurrentTime();
  struct tm *timeinfo = localtime(&now);
  char buffer[15];
  strftime(buffer, sizeof(buffer), "%I:%M %p %a", timeinfo);
  lcd.setCursor(2,0);
  lcd.print(buffer);

  // Serial.println(buffer);

  //Every 1.5 Seconds
  if(millis() - lastWeatherPrint > 1500) {
    lastWeatherPrint = millis();
    printWeather(lcd);
  }
  //Every 5 Minutes
  if(millis() - lastWeatherUpdate > 300000){
    lastWeatherUpdate = millis();
    getWeather(lcd);
  }
}

/**
  Allows the user to set the buffer time between when their flight arrives and when they wake up.
*/
void prepScreen(){

}

/**
  Allows the user to set the flight Iata and Arrival/Departure status of their flight.
*/
void flightScreen(){

}

/**
  Allows the user to toggle the smart alarm on and off.
*/
void alarmScreen(){

}

//Setup
//=======================================================================
void setup() {
  //Serial.print(String) is used for debugging purposes
  Serial.begin(115200);
  delay(1000);
  Serial.println("Setup Started");

  //Set Pins
  for (byte i = 0; i < sizeof(buttonPins); i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  //Connect WiFi, if connection fails wait 1/2 sec and try again
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.println("Connecting to WiFi...");
    }
  Serial.println("WiFi Connected!");

  //Sync ESP32 Clock with NTP server
  configTime(gmtOffset, daylightOffset, ntpServer, ntpServer2);
  delay(2000);
  initTime();

  //LCD Setup
  lcd.init();
  lcd.clear();
  lcd.backlight();

  //Get initial weather
  getWeather(lcd);

  Serial.println("Setup Finished!");
}

//MAIN - Loop
//=======================================================================
void loop() {

  switch(screenPointer){
    case 0:
      clockScreen();
      break;
    case 1:
      //TODO
      lcd.setCursor(0,0);
      lcd.println("Screen 2");
      break;
    case 2:
      //TODO
      lcd.setCursor(0,0);
      lcd.println("Screen 3");
      break;
    case 3:
      //TODO
      lcd.setCursor(0,0);
      lcd.println("Screen 4");
      break;
    default:
      lcd.setCursor(0,0);
      Serial.println("Invalid Screen Pointer - Error!");
      break;
  }
  readButtons();
}
