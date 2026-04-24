#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <time.h>
#include <Weather.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

byte maxScreenPointer = 3;
byte screenPointer = 0;
byte maxAlphabetPointer = 36;
byte flightEditPointer = 0;
byte alphabetPointer = 0;
byte editStage = 0;
LiquidCrystal_I2C lcd(0x27,16,2);
char pSkipLast;
char currentSkip;

struct tm offsetTime;
const byte offsetHourMax = 10;
const byte offsetMinuteMax = 55;

//Note Frequencies - may git rid of after decided on alarm tone to save memory
//=======================================================================
int c = 262;
int cs = 277;
int d = 294;
int ds = 311;
int e = 330;
int f = 349;
int fs = 370;
int g = 392;
int gs = 415;
int a = 440;
int as = 466;
int b = 494;

int c_high  = 523;
int cs_high = 554;
int d_high  = 587;
int ds_high = 622;
int e_high  = 659;
int f_high  = 698;
int fs_high = 740;
int g_high  = 784;
int gs_high = 831;
int a_high  = 880;
int as_high = 932;
int b_high  = 988;

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
const String apiKey   = "API_Key"; //API_Key

//API Setup
//=======================================================================
DynamicJsonDocument flightDoc(3072);
String apiEndpoint = "https://aviation-edge.com/v2/public/timetable?key=" + apiKey + "&flight_iata="; // + "&type=";
String flightIata = "                ";
String flightType;
int departureDelay;
int arrivalDelay;
String departureScheduledTime;
String arrivalScheduledTime;

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

//Other Boolean Variables
//=======================================================================
bool indicatorState = true;
bool noteState = false;
bool playSound = false;
bool smartAlarm = true;
bool validFlight = false;

//Constant Strings for LCD
//=======================================================================
const char alphabet[37] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
const char* prepScrn = "Set Prep Time:";
const char* prepScrnEdit = "Edit Prep Time ";
const char* flightScrn = "Set Flight Iata: ";
const char* alarmScrn = "Smart Alarm: ";

//Custom Characters for LCD
//=======================================================================
byte check[8] = { B00001, B00011, B00011, B00110, B10110, B11110, B11100, B01100 };
byte diamond[8] = { B00000, B00100, B01110, B11111, B01110, B00100, B00000, B00000 };
byte diamond2[8] = { B00000, B00000, B00100, B01110, B11111, B01110, B00100, B00000 };
byte cursor[8] = { B00001, B00011, B00111, B01111, B01111, B00111, B00011, B00001 };
byte cursor2[8] = { B10000, B10000, B10000, B10000, B10000, B10000, B10000, B10000 };
byte wall[8] = { B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111 };

//Timer Variables
//=======================================================================
unsigned long lastWeatherUpdate = 0;
unsigned long lastWeatherPrint = 0;
unsigned long selectionIndicatorTime = 0;
unsigned long selectionIndicator2Time = 0;
unsigned long noteTime = 0;

//Functions
//=======================================================================

void printAPIVariables(){
  lcd.println("Arrival\n==============");
  lcd.print("Scheduled Time: ");
  lcd.println(arrivalScheduledTime);
  lcd.print("Delay: ");
  lcd.println(arrivalDelay);

  lcd.println("\Departure\n==============");
  lcd.print("Scheduled Time: ");
  lcd.println(departureScheduledTime);
  lcd.print("Delay: ");
  lcd.println(departureDelay);
}

/**
  Combines the individual parts of the API endpoint
*/
String createFullEndpoint(){
  String trimmedFlightIata = flightIata;
  trimmedFlightIata.trim();
  return apiEndpoint + trimmedFlightIata + "&type=" + flightType;
}

/**
  Checks if the endpoint is returning valid data
*/
bool checkFullEndpoint(){
  HTTPClient http;
  http.begin(createFullEndpoint());
  int httpCode = http.GET();

  //Request failed
  if (httpCode <= 0) {
    http.end();
    return false;
  }
  //Bad response
  if (httpCode != 200) {
    http.end();
    return false; 
  }

  String result = http.getString();
  http.end();

  flightDoc.clear();
  DeserializationError error = deserializeJson(flightDoc, result);

  //Invalid JSON
  if (error) {
    Serial.println("Deserialization Error");
    return false; 
  }

  // check for expected fields
  // if (!flightDoc.containsKey("flight") ||
  //     !flightDoc.containsKey("status") ) {
  //   return false;
  // }


  //Extract response variables
  if (!flightDoc.isNull() && flightDoc.size() > 0) {

    JsonObject flight = flightDoc[0];

    //Departure Values
    if (!flight["departure"].isNull()){
      departureScheduledTime = flightDoc[0]["departure"]["scheduledTime"].as<String>();
      departureScheduledTime = flightDoc[0]["departure"]["delay"].as<String>();
    }

    //Arrival Values
    if (!flight["arrival"].isNull()){
      arrivalScheduledTime = flightDoc[0]["arrival"]["scheduledTime"].as<String>();
      arrivalScheduledTime = flightDoc[0]["arrival"]["delay"].as<String>();
    }
  }

  return true;
}

/**
  Returns the current letter of the alphabet.
*/
char getAlphabet(){
  return alphabet[alphabetPointer];
}

/**
  Moves the pointer for the alphabet.
*/
void moveAlphabet(bool right){
  if(right){
    //Increment
    alphabetPointer++;
    if(alphabetPointer > maxAlphabetPointer) {alphabetPointer = 0;}
  }
  else{
    //Decrement
    if(alphabetPointer == 0) {alphabetPointer = maxAlphabetPointer;}
    else{alphabetPointer--;}
  }
}

/**
  Helper method for incrementing and decrimenting offsetTime.
  right - increments if true decrements if false
  hour - changes hour if true minutes if false
*/
void changeOffset(bool right, bool hour){
  if(right){ //Increment
    if(hour){
      offsetTime.tm_hour++;
      if(offsetTime.tm_hour > offsetHourMax) {offsetTime.tm_hour = 0;}
    }
    else{ //Minutes
      offsetTime.tm_min+=5;
      if(offsetTime.tm_min > offsetMinuteMax) {offsetTime.tm_min = 0;}
    }
  }
  else{ //Decriment
    if(hour){
      offsetTime.tm_hour--;
      if(offsetTime.tm_hour < 0) {offsetTime.tm_hour = offsetHourMax;}
    }
    else{ //Minutes
      offsetTime.tm_min-=5;
      if(offsetTime.tm_min < 0) {offsetTime.tm_min = offsetMinuteMax;}
    }
  }
}

/**
  Work in progress may need different approach.
  Potentially play notes on the other core to prevent timing issues
  Large scope bool and run this in the main loop when true activate the bool remotely
*/
void playNote(int frequency, int duration) {
  if(!noteState){
    Serial.println("Note Begin");
    ledcWriteTone(buzzerPin, frequency);
    noteState = !noteState;
  }
  else if(millis() - noteTime > duration){
    ledcWriteTone(buzzerPin, 0);
    noteTime = millis();
    noteState = !noteState;
    playSound = false;
    Serial.println("Sound End");
  }
}

/**
  Moves the screenPointer left if False or right if True.
*/
void moveScreen(bool direction = true){
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
        //Button LOW State or On ------------------------------------------------------------------------------------------------------------------
        switch(buttonPin){
          case 4:
            //Left Button
            if(lastState4 == HIGH){
              // Serial.println("Red");
              switch(screenPointer){
                case 0:
                  //Do nothing
                  break;
                case 1:
                  //Prep Time Screen
                  switch(editStage){
                    case 1:
                      //Hours
                      changeOffset(false, true);
                      break;
                    case 2:
                      //Minutes
                      changeOffset(false, false);
                      break;
                  }
                  break;
                case 2:
                  //Flight Edit Screen
                  switch(editStage){
                    case 0:
                      if(editStage == 0){
                        moveAlphabet(false);
                        // Serial.println(getAlphabet());
                      }
                      break;
                    case 1:
                      editStage++;
                      lcd.clear();
                      break;
                    case 2:
                      editStage--;
                      lcd.clear();
                      break;
                  }
                  break;
                case 3:
                  //Not used, may implement further functionality here later
                  break;
                default:
                  Serial.println("Screen Pointer Error");
                  break;
              }
            }
            lastState4 = LOW;
            break;

          case 5:
            //Right Button
            if(lastState5 == HIGH){
              // Serial.println("Yellow");
              switch(screenPointer){
                case 0:
                  //Do nothing
                  break;
                case 1:
                  //Prep Time Screen
                  switch(editStage){
                    case 1:
                      //Hours
                      changeOffset(true, true);
                      break;
                    case 2:
                      //Minutes
                      changeOffset(true, false);
                      break;
                  }
                  break;
                case 2:
                  //Flight Edit Screen
                  switch(editStage){
                    case 0:
                      if(editStage == 0){
                        moveAlphabet(true);
                        // Serial.println(getAlphabet());
                      }
                      break;
                    case 1:
                      editStage++;
                      lcd.clear();
                      break;
                    case 2:
                      editStage--;
                      lcd.clear();
                      break;
                  }
                  break;
                case 3:
                  //TODO
                  break;
                default:
                  Serial.println("Screen Pointer Error");
                  break;
              }
            }
            lastState5 = LOW;
            break;

          case 13:
            //Select Button
            if(lastState13 == HIGH){
              // Serial.println("Green");
              switch(screenPointer){
                case 0:
                  //Do Nothing - clock screen
                  break;
                case 1:
                  //Prep Time Screen
                  lcd.clear();
                  editStage++;
                  if(editStage > 2) {editStage = 0;}
                  break;
                case 2:
                  //Flight Edit Screen
                  switch(editStage){
                    case 0:
                      if(alphabetPointer == 0){ //Condition user enters an empty character
                        lcd.clear();
                        editStage++;
                        flightEditPointer = 0;
                        Serial.println(flightIata);
                        break;
                      }
                      flightEditPointer++;
                      alphabetPointer = 0;
                      if(flightEditPointer > 15){ //Condition user enters a character on the last position on the lcd
                        lcd.clear();
                        flightEditPointer = 0;
                        editStage++;
                        Serial.println(flightIata);
                      }
                      break;
                    case 1:
                      flightType = "arrival";
                      editStage = 3;
                      Serial.println(flightType);
                      lcd.clear();
                      Serial.print("API: ");
                      Serial.println(createFullEndpoint());

                      Serial.print("Valid Endpoint: ");
                      Serial.println(checkFullEndpoint());
                      printAPIVariables();
                      break;
                    case 2:
                      flightType = "departure";
                      editStage = 3;
                      Serial.println(flightType);
                      lcd.clear();
                      Serial.print("API: ");
                      Serial.println(createFullEndpoint());

                      Serial.print("Valid Endpoint: ");
                      Serial.println(checkFullEndpoint());
                      printAPIVariables();
                      break;
                    case 3:
                      //TODO
                      break;
                  }
                  break;
                case 3:
                  //Alarm Screen
                  smartAlarm = !smartAlarm;
                  break;
                default:
                  Serial.println("Screen Pointer Error");
                  break;
              }
            }
            lastState13 = LOW;
            break;

          case 18:
            //Cycle Button
            if(lastState18 == HIGH){
              // Serial.println("Blue");
              // playSound = true;
              editStage = 0;
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
        //Button HIGH State or Off ------------------------------------------------------------------------------------------------------------------
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
  Displays a selection indicator icon at the given position on the lcd.
  Intended to be used in conjunction with constantly writing what you want beneath the cursor.
*/
void selectionIndicator(LiquidCrystal_I2C &lcd, byte col = 0, byte row = 0, byte indicator = 0, int offTime = 1000, int onTime = 300){
  //Don't write for offTime
  if(!indicatorState){
    if(millis() - selectionIndicatorTime > offTime){
      indicatorState = true;
      selectionIndicatorTime = millis();
    }
  }
  else{
    //Write constantly for onTime
    lcd.setCursor(col, row);
    lcd.write(indicator);
    if(millis() - selectionIndicatorTime > onTime){
      indicatorState = false;
      selectionIndicatorTime = millis();
    }
  }
}

/**
  Displays a blinking selection indicator icon at the given position on the lcd.
  strTime - how long String is there before indicator overwrites it
  indTime - how long indicator is there before string overwrites it
*/
void blinkIndicator(LiquidCrystal_I2C &lcd, byte strCol, byte strRow, byte indCol, byte indRow, String replaceString = " ", byte indicator = 0, int strTime = 1000, int indTime = 1000){
  if(indicatorState){
    if(millis() - selectionIndicatorTime > strTime){
      indicatorState = false;
      selectionIndicatorTime = millis();
      lcd.setCursor(indCol, indRow);
      lcd.write(indicator);
    }
  }
  else{
    if(millis() - selectionIndicatorTime > indTime){
      indicatorState = true;
      selectionIndicatorTime = millis();
      lcd.setCursor(strCol, strRow);
      lcd.print(replaceString);
    }
  }
}

/**
  Displays a blinking selection indicator icon at the given position on the lcd.
*/
void blinkIndicator(LiquidCrystal_I2C &lcd, byte col, byte row, char replaceChar = ' ', byte indicator = 0, int charTime = 1000, int indicatorTime = 1000){
  if(indicatorState){
    if(millis() - selectionIndicatorTime > charTime){
      indicatorState = false;
      selectionIndicatorTime = millis();
      lcd.setCursor(col, row);
      lcd.write(indicator);
    }
  }
  else{
    if(millis() - selectionIndicatorTime > indicatorTime){
      indicatorState = true;
      selectionIndicatorTime = millis();
      lcd.setCursor(col, row);
      lcd.print(replaceChar);
    }
  }
}

/**
  Method prints a string and skips a specific char in the print to allow selectionIndicator to work on existing text.
*/
char printSkip(LiquidCrystal_I2C &lcd, String text, int skipIndex, int col = 0, int row = 0){
  lcd.setCursor(col, row);
  for(int i = 0; i < text.length(); i++){
    if(i == skipIndex) {
      col++;
      continue;
    }
    lcd.setCursor(col++, row);
    lcd.print(text[i]);
  }
  return text[skipIndex];
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
  //Format PrepTime variables
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", offsetTime.tm_hour, offsetTime.tm_min);

  switch(editStage){
    case 0:
      //Row 1
      lcd.setCursor(0,0);
      lcd.print("Prep Time: ");
      lcd.print(timeStr);
      //Row 2
      lcd.setCursor(0,1);
      lcd.print(prepScrnEdit);
      blinkIndicator(lcd, 15, 1);
      break;
    case 1:
      //Row 1
      lcd.setCursor(0,0);
      lcd.print(prepScrn);
      //Row 2
      lcd.setCursor(0,1);
      lcd.print(timeStr);
      selectionIndicator(lcd, 1, 1, 1, 1000, 333);
      break;
    case 2:
      //Row 1
      lcd.setCursor(0,0);
      lcd.print(prepScrn);
      //Row 2
      lcd.setCursor(0,1);
      lcd.print(timeStr);
      selectionIndicator(lcd, 4, 1, 1, 1000, 333);
      break;
  }
  
}

/**
  Allows the user to set the flight Iata and Arrival/Departure status of their flight.
*/
void flightScreen(){
  switch(editStage){
    case 0:
      //Row 1
      lcd.setCursor(0,0);
      lcd.print(flightScrn);
      //Row 2
      flightIata[flightEditPointer] = getAlphabet();
      if(!indicatorState){
        lcd.setCursor(0,1);
        lcd.print(flightIata);
      }   
      selectionIndicator(lcd, flightEditPointer, 1, 1, 1000, 333);
      //TODO
      break;
    case 1:
      //ROW 1
      lcd.setCursor(0,0);
      lcd.print("Arrival");
      blinkIndicator(lcd, 10, 0, ' ', 0, 1000, 1000);
      //ROW 2
      lcd.setCursor(0,1);
      lcd.print("Departure");
      break;
    case 2:
      //ROW 1
      lcd.setCursor(0,0);
      lcd.print("Arrival");
      //ROW 2
      lcd.setCursor(0,1);
      lcd.print("Departure");
      blinkIndicator(lcd, 10, 1, ' ', 0, 1000, 1000);
      break;
    case 3:
      //display flightIata
      lcd.setCursor(0,0);
      lcd.print(flightIata);
      //show if api found flight
      lcd.setCursor(0,1);
      lcd.print("Flight: "); //Found or Missing
      //TODO

  }
}

/**
  Allows the user to toggle the smart alarm on and off.
  //Possible TODO when toggled off functionality to instead manually set an alarm
*/
void alarmScreen(){
  switch(editStage){
    case 0:
      lcd.setCursor(0,0);
      lcd.print(alarmScrn);
      if(smartAlarm){
        lcd.print("On ");
      }
      else{
        lcd.print("Off");
      }
      selectionIndicator(lcd, 13, 0, 1, 1000, 300);
      break;
    case 1:
      //Would be for if manual alarm functionality is added
      break;
  }
  
  

}

//Setup
//=======================================================================
void setup() {
  //Setup Buzzer
  ledcAttach(buzzerPin, 2000, 8);

  //tm objects setup
  memset(&offsetTime, 0, sizeof(offsetTime));

  //Serial.print(String) is used for debugging purposes
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nSetup Started");

  //Set Pins
  for (byte i = 0; i < sizeof(buttonPins); i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  //Connect WiFi, if connection fails wait 1.5 sec and try again
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) { 
    delay(1500); 
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
  lcd.createChar(0, check);
  lcd.createChar(1, wall);
  lcd.createChar(2, diamond2);
  lcd.createChar(3, cursor);
  lcd.createChar(4, cursor2);

  //Get initial weather
  getWeather(lcd);

  // ledcWriteTone(14, 262);
  // delay(300);
  // ledcWriteTone(14, 0);
  Serial.println("Setup Finished!\n=========================\n");
}

//MAIN - Loop
//=======================================================================
void loop() {
  if(playSound){
    Serial.println("Playing Sound");
    playNote(c, 1000);
  }

  // screenPointer = 7; //Test Case - Debugging

  switch(screenPointer){
    case 0:
      clockScreen();
      break;
    case 1:
      prepScreen();
      break;
    case 2:
      flightScreen();
      break;
    case 3:
      alarmScreen();
      break;
    case 7:
      //Test Case - Debugging
      // lcd.setCursor(0,0);
      lcd.setCursor(1,0);
      lcd.print("3");
      delay(2000);
      Serial.println("Delay OVER");
      currentSkip = printSkip(lcd, "Hello World", 1, 0, 0);
      delay(2000);
      lcd.print(currentSkip);
      Serial.println(currentSkip);
    default:
      lcd.setCursor(0,0);
      Serial.println("Invalid Screen Pointer - Error!");
      break;
  }
  readButtons();
}
