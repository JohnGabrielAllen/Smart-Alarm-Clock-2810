//Function
//=======================================
//TODO selectionIndicator where instead of using replaceChar uses replaceString and seperate positioning for it, I believe this will fix my previous visual lag issues.

/**
  Displays a blinking selection indicator icon at the given position on the lcd.
  TODO Consider different delay times for indicator and replaceChar
  strTime - how long String is there before indicator overwrites it
  indTime - how long indicator is there before string overwrites it
*/
void selectionIndicator(LiquidCrystal_I2C &lcd, byte strCol, byte strRow, String replaceString = " ", byte indCol, byte indRow, byte indicator = 0, int strTime = 1000, int indTime = 1000){
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


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
