#include "Weather.h"

uint8_t displayState = 0;

DynamicJsonDocument weatherDoc(2048);

const String endpoint = "https://api.open-meteo.com/v1/forecast?latitude=40.758478&longitude=-111.888142&timezone=America%2FDenver&current_weather=true";

const uint8_t weatherCodes [28] = {
  0, 1, 2, 3, 45, 48, 51, 53, 55, 56, 57, 61, 63, 65, 66,
  67, 71, 73, 75, 77, 80, 81, 82, 85, 86, 95, 96, 99
};

const char* weatherDescriptions [28] = {
  "Clear Sky", "Mainly Clear", "Partly Cloudy", "Overcast", "Fog"
  "Rime", "Light Drizzle", "Moderate Drizzle", "Heavy Drizzle", "Freezing Drizzle",
  "Freezing Drizzle", "Slight Rain", "Moderate Rain", "Heavy Rain", "Freezing Rain",
  "Freezing Rain", "Light Snow", "Moderate Snow", "Heavy Snow", "Snow Grains", 
  "Light Showers", "Moderate Showers", "Heavy Showers", "Snow Showers", "Snow Storm",
  "Thunderstorms", "Slight Hail", "Heavy Hail"
};

const char* codeToDescription(uint8_t code)
{
  for (int i = 0; i < 28; ++i)
  {
    if (weatherCodes[i] == code)
    {
      return weatherDescriptions[i];
    }
  }
  return "No weather info.";
}

void printTemperature(LiquidCrystal_I2C &lcd)
{
  float tempC = weatherDoc["current_weather"]["temperature"];
  float tempF = (tempC * 1.8) + 32;
  lcd.setCursor(0, 1);
  lcd.print("    ");
  lcd.print(tempF);
  lcd.print((char)223);
  lcd.print(" F          ");
}

void printConditions(LiquidCrystal_I2C &lcd)
{
  uint8_t weatherCode = weatherDoc["current_weather"]["weathercode"];
  const char* weatherDescription = codeToDescription(weatherCode);
  uint8_t buffer = (16 - strlen(weatherDescription)) / 2;
  lcd.setCursor(0, 1);
  for (int i = 0; i < buffer; ++i)
  {
    lcd.print(' ');
  }
  lcd.print(weatherDescription);
  lcd.print("                ");
}

void printDate(LiquidCrystal_I2C &lcd)
{
  String dateStr = weatherDoc["current_weather"]["time"].as<String>();
  dateStr = dateStr.substring(0, 10);
  uint8_t buffer = (16 - dateStr.length()) / 2;
  lcd.setCursor(0, 1);
  for (int i = 0; i < buffer; ++i)
  {
    lcd.print(' ');
  }
  lcd.print(dateStr);
  lcd.print("                ");
}

void printWeather(LiquidCrystal_I2C &lcd)
{
  switch(displayState)
  {
    case 0:
      printTemperature(lcd);
      break;
    case 1:
      printConditions(lcd);
      break;
    case 2:
      printDate(lcd);
      break;
  }
  displayState = displayState < 2 ? displayState + 1 : 0;
}

void getWeather(LiquidCrystal_I2C &lcd)
{
  if ((WiFi.status() == WL_CONNECTED))
  {
    HTTPClient http;
    http.begin(endpoint);
    int httpCode = http.GET();
    if (httpCode > 0)
    { 
      String result = http.getString();
      DeserializationError error = deserializeJson(weatherDoc, result);
      if (error)
      {
        lcd.setCursor(4, 1);
        lcd.print("--json--");
      }
    }
    else
    {
      lcd.setCursor(4, 1);
      lcd.print("--http--");
    }
    http.end();
  }
  else
  {
    lcd.setCursor(4, 1);
    lcd.print("--wifi--");
  }
}