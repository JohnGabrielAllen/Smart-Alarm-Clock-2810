#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <time.h>

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
const char* ssid     = "WiFi_Name";
const char* password = "WiFi_Password";

//PIN Setup
//=======================================================================
const byte buzzerPin = 14;
const byte buttonPins[] = {4, 5, 13, 18}; //{left=4, right=5, select=13, cycle=18}



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
        switch(buttonPin){
          case 4:
            //Left Button
            Serial.print("Red");
            break;

          case 5:
            //Right Button
            Serial.print("Yellow");
            break;

          case 13:
            //Select Button
            Serial.print("Green");
            break;

          case 18:
            //Cycle Button
            Serial.print("Blue");
            moveScreen(true);
            break;

          default:
            Serial.print("Error");
            break;
        }
        Serial.println(" = LOW");
      }
    }
}

/**
  Syncs ESP32 time.
*/
void syncTime(){
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

void clockScreen(){
  time_t now = getCurrentTime();
  struct tm *timeinfo = localtime(&now);
  char buffer[12];
  strftime(buffer, sizeof(buffer), "%I:%M %p %a", timeinfo);
  lcd.setCursor(0,0);
  lcd.print(buffer);
  Serial.println(buffer);
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
  while(WiFi.status() != WL_CONNECTED) { delay(500); }

  //Sync ESP32 Clock with NTP server
  configTime(gmtOffset, daylightOffset, ntpServer, ntpServer2);
  delay(2000);
  syncTime();

  //LCD Setup
  lcd.init();
  lcd.backlight();

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
      break;
    case 2:
      //TODO
      break;
    case 3:
      //TODO
      break;
    default:
      Serial.println("Invalid Screen Pointer - Error!");
      break;
  }
  readButtons();
}
