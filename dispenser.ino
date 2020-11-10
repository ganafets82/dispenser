/*
 
*/
#include <HX711_ADC.h>
#include<Servo.h>

Servo myservo;

//pins:
const int HX711_dout = 9; //mcu > HX711 dout pin
const int HX711_sck = 6; //mcu > HX711 sck pin
const int servo1 = 5; //mcu > servo PWM signal

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

long t; //for timing the serial output
boolean fill = false;
boolean newDataReady = 0;
float weight;
int closed = 152; //degrees of servo for closed pinch valve 
int milliliters = 5040; //desired ammount to dispense in gramm
int slow = 800; //gramms which need to be left when valve starts to close slowly
int drip = 50; //gramm which need to be left for dripping in the exact ammount
int drip_pos = 145; //degrees of servo for dripping short before reaching final weight
boolean tared = true; //for tare checking

void setup() {
  myservo.attach(servo1, 500, 2500);
  myservo.write(closed);
  Serial.begin(57600); delay(10);
  Serial.println();
  Serial.println("Starting...");

  LoadCell.begin();
  float calibrationValue; // calibration value (see example file "Calibration.ino")
  calibrationValue = -14.80; // uncomment this if you want to set the calibration value in the sketch
  long stabilizingtime = 200; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Startup is complete");
  }
  pinMode (A1,INPUT); //button for starting the filling
  pinMode (A2,INPUT); //abort filling
  pinMode (A3,INPUT); //tare the scale

  
}


void loop() {
  
  const int serialPrintInterval = 200; //increase value to slow down serial print activity
  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;
  if (newDataReady) weight = LoadCell.getData();

  //Read Buttons
  boolean S1 = digitalRead(A1);
  boolean S2 = digitalRead(A2);
  boolean S3 = digitalRead(A3);

 //Serial output:
     if (millis() > t + serialPrintInterval) {
      Serial.print("Servo: ");
      Serial.print(myservo.read());
      Serial.write(" | ");
      Serial.print("Weight: ");
      Serial.print(weight,0);
      Serial.write(" | ");
      Serial.print(S1);
      Serial.write(" | ");
      Serial.print(S2);
      Serial.write(" | ");
      Serial.print(S3);
      Serial.write(" | ");
      if (fill == false) Serial.print ("ready | ");
      if (fill == true) Serial.print ("fill | ");
      if (tared == false) Serial.print ("wait for taring");
      if  (tared == true) Serial.print ("tared");
      Serial.println();
      newDataReady = 0;
      t = millis();
    }
  
//start filling
  if (S1 == LOW && fill == false && weight < milliliters){ //cant start during filling or if last amount is still on the scale           
    if (weight >= 5 || weight <= -5) { //tare only if weight is far of
      LoadCell.tareNoDelay();
      tared = false;
    }
    fill = true;
  }
  
//stop filling
  if (S2 == LOW) fill = false;  
  
//tare manually, will stop filling also
  if (S3 == LOW){         
    fill = false;
    LoadCell.tareNoDelay();
    tared = false;
  }    

  //filling procedure
  if (fill == true) {
    if (tared == true) {
      if (milliliters-weight >= drip) myservo.write(constrain(((weight-milliliters+slow+drip)/slow*drip_pos),0,drip_pos)); //valve closes smootly
        else myservo.write(drip_pos); //dripping for exact dosing or removing foam at bottle filling
      if (weight > milliliters) fill = false; //if desires weight is reached filling is finished
    }
  }
  
  if (fill == false) myservo.write(closed); //close valve always if no filling is going on


 if (LoadCell.getTareStatus() == true) tared = true;  //check if taring is finished
  
   
}
