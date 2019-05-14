#include <SPI.h>
#include <Wire.h>
#include <Bounce2.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <medianFilter.h>

#define HWSERIAL Serial1
//defined(TEENSYDUINO)
#define BUTTON_A  4
#define BUTTON_B  3
#define BUTTON_C  8

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

enum SENSITIVITY { sLOW, sMED, sHIGH };
SENSITIVITY currentSensitivity = sMED;
int maxRangeValue = 64; //range sensor will map from 0 to maxRangeValue

bool bluetoothEnabled = true;

Bounce debouncer_a = Bounce(); // Instantiate a Bounce object
Bounce debouncer_b = Bounce(); // Instantiate a Bounce object
Bounce debouncer_c = Bounce(); // Instantiate a Bounce object

medianFilter filter1; //removing noise from potentiometer
medianFilter filter2;

enum INPUT_TYPE { BUTTON, RANGE, PIVOT };

void sendData(INPUT_TYPE, String, String = "1");
void debug_print(String, String = NULL);
boolean delay_without_block(unsigned long &since, unsigned long time);

void splashScreen() {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("Sketchuino");
  display.setTextSize(0.5);
  display.println("santi, foster, katya");
  display.display();
  delay(2000);
}

void setup() {
  Serial.begin(9600);
  HWSERIAL.begin(9600);
  filter1.begin();
  filter2.begin();
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

  // Clear the buffer.
  display.clearDisplay();
  display.display();
  splashScreen();

  debouncer_a.attach(BUTTON_A,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  debouncer_a.interval(25); // Use a debounce interval of 25 milliseconds

  debouncer_b.attach(BUTTON_B,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  debouncer_b.interval(25); // Use a debounce interval of 25 milliseconds

  debouncer_c.attach(BUTTON_C,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  debouncer_c.interval(25); // Use a debounce interval of 25 milliseconds

  updateSensitivity();
  
  debug_print("ready");
}

void loop() {
  displayParameters();
  
  if (canReceiveMessage()) {
    String msg = receiveMessage();
    
  }

  /*if(Serial.available() > 0) {
    String test = Serial.readString();
    //sendMessage(test);
  }*/

  publishRange();
  checkButtons();

  delay(100);
}

void onlineRefresh() {
  publishRange();
  updateSensitivity();
}

//[ RANGE SENSORS
int lastSensorValue1 = 0;
int lastSensorValue2 = 0;
void publishRange() {
  
  int analogInPin1 = A1;
  int analogInPin2 = A3;
  
  int sensorValue1_noisy = analogRead(analogInPin1);
  int sensorValue2_noisy = analogRead(analogInPin2);
  
  int sensorValue1 = map(sensorValue1_noisy, 0, 1023, 0, maxRangeValue);
  int sensorValue2 = map(sensorValue2_noisy, 0, 1023, 0, maxRangeValue);

  int filteredValue1 = filter1.run(sensorValue1);
  int filteredValue2 = filter2.run(sensorValue2);

  if (lastSensorValue1 != filteredValue1) {
    sendData(RANGE, "a", filteredValue1);
    lastSensorValue1 = filteredValue1;
  }

  if (lastSensorValue2 != filteredValue2) {
    sendData(RANGE, "b", filteredValue2);
    lastSensorValue2 = filteredValue2;
  }
}

//[ SENSITIVTY
void toggleSensitivity() {
  if (currentSensitivity == sLOW) {
    currentSensitivity = sMED;
  } else if (currentSensitivity == sMED) {
    currentSensitivity = sHIGH;
  } else if (currentSensitivity == sHIGH) {
    currentSensitivity = sLOW;
  }
  
  updateSensitivity();
}

void updateSensitivity() {
  switch (currentSensitivity) {
    case sLOW:
      maxRangeValue = 32;
      break;
    case sMED:
      maxRangeValue = 64;
      break;
    case sHIGH:
      maxRangeValue = 128;
      break;
  }
  
  sendData(PIVOT, "a", maxRangeValue/2);
}

//[ BUTTONS
void checkButtons() {
   debouncer_a.update(); 
   debouncer_b.update(); 
   debouncer_c.update(); 

   if ( debouncer_a.fell() ) {  // Call code if button transitions from HIGH to LOW
     sendData(BUTTON, "a");
     toggleSensitivity();
   }

   if ( debouncer_b.fell() ) {  // Call code if button transitions from HIGH to LOW
     bluetoothEnabled = !bluetoothEnabled;
     sendData(BUTTON, "b");
     onlineRefresh();
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
  if (!bluetoothEnabled) return;
  //while (HWSERIAL.available()) { } //buffer is full, don't send data yet
  HWSERIAL.print(msg + "\r\n");
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
    case PIVOT:
      return "pivot";
    default:
      return "unknown";
  }
}

//[ DISPLAY
void displaySensitivity() {
  switch (currentSensitivity) {
    case sLOW:
      display.print("LOW/");
      display.println(maxRangeValue);
      break;
    case sMED:
      display.print("MED/");
      display.println(maxRangeValue);
      break;
    case sHIGH:
      display.print("HIGH/");
      display.println(maxRangeValue);
      break;
  }
}

unsigned long screenTime = 0;
void displayParameters() {
  if (!delay_without_block(screenTime, 500)) return;

  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  
  display.print("/range/a ");
  display.println(lastSensorValue1);
  
  display.print("/range/b ");
  display.println(lastSensorValue2);
  
  display.print("sensitivity: ");
  displaySensitivity();
  
  display.print("publishing: ");
  if (bluetoothEnabled) {
    display.println("enabled");
  } else {
    display.println("disabled");
  }
  
  display.display();
}

//[ DELAY
boolean delay_without_block(unsigned long &since, unsigned long time) {
  // return false if we're still "delaying", true if time ms has passed.
  unsigned long currentmillis = millis();
  if (currentmillis - since >= time) {
    since = currentmillis;
    return true;
  }
  return false;
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
