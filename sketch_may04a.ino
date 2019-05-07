#include <SPI.h>
#include <Wire.h>
#include <Bounce2.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

#define HWSERIAL Serial1
//defined(TEENSYDUINO)
#define BUTTON_A  4
#define BUTTON_B  3
#define BUTTON_C  8

Bounce debouncer_a = Bounce(); // Instantiate a Bounce object
Bounce debouncer_b = Bounce(); // Instantiate a Bounce object
Bounce debouncer_c = Bounce(); // Instantiate a Bounce object

enum INPUT_TYPE { BUTTON, RANGE };

void sendData(INPUT_TYPE, String, String = "1");
void debug_print(String, String = NULL);

void setup() {
  Serial.begin(9600);
  HWSERIAL.begin(9600);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  //splashscreen
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  debouncer_a.attach(BUTTON_A,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  debouncer_a.interval(25); // Use a debounce interval of 25 milliseconds

  debouncer_b.attach(BUTTON_B,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  debouncer_b.interval(25); // Use a debounce interval of 25 milliseconds

  debouncer_c.attach(BUTTON_C,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  debouncer_c.interval(25); // Use a debounce interval of 25 milliseconds

  debug_print("ready");
}

void loop() {

  if (canReceiveMessage()) {
    String msg = receiveMessage();
    
  }

  /*if(Serial.available() > 0) {
    String test = Serial.readString();
    //sendMessage(test);
  }*/

  checkButtons();
}

//[ BUTTONS
void checkButtons() {
   debouncer_a.update(); 
   debouncer_b.update(); 
   debouncer_c.update(); 

   if ( debouncer_a.fell() ) {  // Call code if button transitions from HIGH to LOW
     sendData(BUTTON, "a");
   }

   if ( debouncer_b.fell() ) {  // Call code if button transitions from HIGH to LOW
     sendData(BUTTON, "b");
   }
  
   if ( debouncer_c.fell() ) {  // Call code if button transitions from HIGH to LOW
     sendData(BUTTON, "c");
   }
}

//[ NETWORKING
boolean canReceiveMessage() {
  return HWSERIAL.available() > 0;
}

String receiveMessage() {
  if(!canReceiveMessage()) return NULL;

  String message = "";
  while(HWSERIAL.available() > 0) { //TODO test with and without loop
    message += HWSERIAL.readString();
  }
  debug_print(message, "bt_received");
  return message;
}

void sendMessage(String msg) {
  //while (HWSERIAL.available()) { } //buffer is full, don't send data yet
  HWSERIAL.println(msg);
  HWSERIAL.flush();
  debug_print(msg, "bt_send");
}

void sendData(INPUT_TYPE type, String tag, String value = "1") {
  String input = inputValue(type);
  sendMessage("/" + input + "/" + tag + " " + value);
}

String inputValue(INPUT_TYPE type) {
  switch (type) {
    case BUTTON:
      return "button";
    case RANGE:
      return "range";
    default:
      return "unknown";
  }
}

//[ DEBUG PRINT
void debug_print(String msg, String tag = NULL) {
  String mutableTag = tag;
  if (mutableTag == "0") {
    mutableTag = "GENERAL";
  }

  mutableTag.toUpperCase();
  mutableTag = "[" + mutableTag + "]";

  Serial.print(mutableTag + " ");
  Serial.println(msg);
}
