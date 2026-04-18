#include <Wire.h>
#include <LiquidCrystal_I2C.h>

byte maxScreenPointer = 3;
byte screenPointer = 0;
LiquidCrystal_I2C lcd(0x27,16,2);

//True is right, False is left
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

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
