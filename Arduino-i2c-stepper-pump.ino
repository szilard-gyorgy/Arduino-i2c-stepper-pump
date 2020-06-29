#define STEPPERS 4
#define EN 8
#define ML_P_STEP 18820

#include <Wire.h>

struct stepper_pins {
  uint8_t stepPin;
  uint8_t dirPin;
};

stepper_pins pins[4] = {
  {2,5},
  {3,6},
  {4,7},
  {13,12}
};

//stepper_pins pins[4] = {
//  {2,5},
//  {3,6},
//  {4,7},
//  {13,12}
//};

int incomingByte = 0; 
String serial2_string = "";

void setup() {
 
  Serial.begin(115200);
  Serial.println("start");
  Wire.begin(0x8);
  Wire.onReceive(receiveEvent);
  
  for (uint8_t i=0;i<STEPPERS;i++){
    pinMode(pins[i].stepPin,OUTPUT); 
    pinMode(pins[i].dirPin,OUTPUT);
  }
  pinMode (EN, OUTPUT);
  digitalWrite (EN, HIGH);
}
void loop (){
  delay(100)  
}

void loop() {
  boolean serial2_string_complete = false;
  if (Serial.available() > 0) {
    char inchar = (char)Serial.read();
    serial2_string += inchar;
    if (inchar == '\r')
      serial2_string_complete = true;          
  }
    
  if (serial2_string_complete == true) {
    Serial.flush();
    Serial.print(">");
    Serial.print(serial2_string);
    Serial.print("<");
    if (serial2_string.startsWith("pump")) {
      uint8_t pump = serial2_string.substring(5,6).toInt();
      bool dir = (boolean)serial2_string.substring(7,8).toInt();
      float quantity = serial2_string.substring(9).toFloat();
      digitalWrite(pins[pump].dirPin,dir);
      Serial.print("Pump:");
      Serial.print(pump);
      Serial.print("dir:");
      Serial.print(dir);
      Serial.print("steps:");
      Serial.print(quantity);
      Serial.println();
      digitalWrite (EN, LOW);
      for(uint32_t x = 0; x < quantity*ML_P_STEP; x++) {
        digitalWrite(pins[pump].stepPin,HIGH); 
        delayMicroseconds(500); 
        digitalWrite(pins[pump].stepPin,LOW); 
        delayMicroseconds(500); 
      }
      digitalWrite (EN, HIGH);
    }
    serial2_string = "";    
  }
}
