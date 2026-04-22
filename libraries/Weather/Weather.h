#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

void getWeather(LiquidCrystal_I2C &lcd);
void printWeather(LiquidCrystal_I2C &lcd);

#endif