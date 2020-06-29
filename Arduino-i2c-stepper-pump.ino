#include <Wire.h>

#define EN 8              // Enable PIN
#define STEPPERS 3        // Number of steppers configured
#define ML_P_STEP 18820   // how many stepts to pump 1ml
#define I2C_MAXLEN 32     // maximum response size for I2C

//define struct types
struct stepper_pins {
  uint8_t stepPin;
  uint8_t dirPin;
};

struct pump_struct{
  uint16_t pump;
  uint16_t dir;
  float quantity;
  uint32_t msg_time;
  uint16_t check;
};

// init variables

String directions[] = {"forward", "backwards"};
String message = "";
char response[I2C_MAXLEN];
uint8_t msg_len;

pump_struct data;
stepper_pins pins[3] = { // stepper pins Nano
  {5,2},
  {6,3},
  {7,4}
};
//stepper_pins pins[3] = { // stepper pins Uno
//  {2,5},
//  {3,6},
//  {4,7}
//};



void setup() { 
  Serial.begin(115200);
  Wire.begin(0x9);
  Wire.onRequest(sendData);
  Wire.onReceive(receiveEvent);
  
  for (uint8_t i=0;i<STEPPERS;i++){
    pinMode(pins[i].stepPin,OUTPUT); 
    pinMode(pins[i].dirPin,OUTPUT);
  }
  pinMode (EN, OUTPUT);
  digitalWrite (EN, HIGH);
  Serial.println("started");
}

void loop (){
  delay(1000);
}

void sendData(){
  Wire.write(response); 
}

void send_message(String msg){
  message += msg + '\n';
  Serial.println(msg);
}

String dump_wire(){
  String wrong_data = "";
  while(Wire.available())
    wrong_data += ' '+String(Wire.read(), DEC);
  return wrong_data;
}

void pump(){
  send_message("Pumping even " + String(data.msg_time) + " with pump " + String(data.pump) + " " + String(directions[data.dir-1]) + " " + String(data.quantity) + "ml of fluid");

  digitalWrite(pins[data.pump-1].dirPin,(boolean)(data.dir-1));
  digitalWrite (EN, LOW);
  for(uint32_t x = 0; x < data.quantity*ML_P_STEP; x++) {
    digitalWrite(pins[data.pump-1].stepPin,HIGH); 
    delayMicroseconds(500); 
    digitalWrite(pins[data.pump-1].stepPin,LOW); 
    delayMicroseconds(500); 
  }
  digitalWrite (EN, HIGH);
  send_message("Finished pumping event from " + String(data.msg_time));
}


void receiveEvent(int howMany) {
  if (Wire.available()) {
    int cmd= Wire.read();
    switch (cmd) {
      case 195:
        if (howMany-1 == sizeof data) {
          Wire.readBytes((byte*)&data, sizeof data);
          uint16_t check = data.pump*data.dir*(data.quantity*100);
          if (check == data.check)
            pump();
          else
            send_message("Error: Data check failed");
        } 
        else
          send_message("Error: Data size mismatch:" + dump_wire());
        break;
      case 100:
        dump_wire();
        response[0] = message.length();
        msg_len = message.length();        
        if (msg_len > I2C_MAXLEN)
          msg_len = 32;
        message.toCharArray(response, msg_len);  
        message = message.substring(msg_len-1);        
        break;
      default:
        send_message("Error: Unknown command "+ String(cmd, DEC));
        dump_wire();
        break;
    }
  }      
}
