#include <Wire.h> 
#include <LiquidCrystal_I2C.h> 
#include <Stepper.h>
#include <Servo.h> 
#include <SoftwareSerial.h>
#include <LowPower.h> 

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

int resval = 0;  // holds the value
int light;
int respin = A1; // sensor pin used
int ressupply = 2;  // holds the value
const int ldrPin = A0;
const int ldrThreshold = 100;
int food; // Binary variable, Food = 1; No Food = 0.
int water; // 4 possible water levels
int food_per; 

const int stepsPerRevolution = 2048; // Number of steps per revolution for 28BYJ-48 motor
const float degreesPerStep = 360.0 / stepsPerRevolution; // Degrees per step
//const int degrees = 90;
const int degrees = 270;

const int trigPin = 4; // Ultrasonic sensor 
const int echoPin = 7;

// Declare the Servo pin 
int servoPin = 3; 
// Create a servo object 
Servo Servo1; 

// Initialize the stepper library on pins 8 through 11
Stepper myStepper(stepsPerRevolution, 8, 5, 9, 6);

// SERIAL COMMUNICATION ARDUINO UNO - MINI:
// Define pins for SoftwareSerial communication with the Pro Mini
#define RX_PIN 10 // Uno receives data on this pin
#define TX_PIN 11 // Uno transmits data on this pin
SoftwareSerial mySerial(RX_PIN, TX_PIN); // Create a SoftwareSerial instance
// Remember to add a common ground between both boards

// Uplink payload:
byte up_payload[4]; // The fourth byte is 1 when the actuators have already worked

void setup() { 
 
  // start the serial console
  Serial.begin(9600);
  mySerial.begin(9600);       // Initialize SoftwareSerial communication
  pinMode(ressupply, OUTPUT);
  lcd.init();         // initialize the lcd 
  lcd.backlight();    // Turn on the LCD screen backlight 
  // Print a message to the LCD.

  // Set the speed at 10 RPMs, food motor
  myStepper.setSpeed(10);

  // Set the pins for the ultrasonic sensor:
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // We need to attach the servo to the used pin number 
  Servo1.attach(servoPin); 

} 
  
void loop() {

  byte awake[1] = {0xAA}; // AWAKE MESSAGE TO MINI
  mySerial.write(awake, 1); // Send it
  enterSleepMode(120); // Wait for 2 minutes in sleep mode

  byte receivedDL;

  if (mySerial.available() != 0) { // Check if 3 bytes are available
    receivedDL = mySerial.read();
  }
  Serial.print("Received DL: ");
  Serial.print("0x");
  Serial.print(receivedDL, HEX);
  Serial.print(" ");
  Serial.println();
  enterSleepMode(120); // Wait for 2 minutes in sleep mode

  if (receivedDL == 0){

    sensorRoutine();

    // Defining the uplink payload
    up_payload[0] = (byte)water;     
    up_payload[1] = (byte)food;      
    up_payload[2] = (byte)food_per; 
    up_payload[3] = 0; // The actuators have not been activated 
    mySerial.write(up_payload, 4); // Send the 3 bytes

    // RESET DL TO O
    receivedDL = 0;

    // Printing the payload
    Serial.print("Payload: ");
    for (int i = 0; i < 4; i++) {
      Serial.print(up_payload[i]);  // Imprime cada valor como decimal
      if (i < 3) Serial.print(", ");  // Separador entre valores
    }
    Serial.println();

    // SLEEP MODE
    enterSleepMode(120); // Wait for 2 minutes in sleep mode

  } else if(receivedDL == 1){
    
    // REFILL WATER
    refillWater();

    sensorRoutine();

    // Defining the uplink payload
    up_payload[0] = (byte)water;     
    up_payload[1] = (byte)food;      
    up_payload[2] = (byte)food_per; 
    up_payload[3] = 1; // The actuators have not been activated 
    mySerial.write(up_payload, 4); // Send the 3 bytes

    // RESET DL TO O
    receivedDL = 0;

    // Printing the payload
    Serial.print("Payload: ");
    for (int i = 0; i < 4; i++) {
      Serial.print(up_payload[i]);  // Imprime cada valor como decimal
      if (i < 3) Serial.print(", ");  // Separador entre valores
    }
    Serial.println();

    // SLEEP MODE
    enterSleepMode(120); // Wait for 2 minutes in sleep mode

  } else if(receivedDL == 2){ //REFILL FOOD

    refillFood();

    sensorRoutine();

    // Defining the uplink payload
    up_payload[0] = (byte)water;     
    up_payload[1] = (byte)food;      
    up_payload[2] = (byte)food_per; 
    up_payload[3] = 2; // The actuators have not been activated 
    mySerial.write(up_payload, 4); // Send the 3 bytes

    // RESET DL TO O
    receivedDL = 0;

    // Printing the payload
    Serial.print("Payload: ");
    for (int i = 0; i < 4; i++) {
      Serial.print(up_payload[i]);  // Imprime cada valor como decimal
      if (i < 3) Serial.print(", ");  // Separador entre valores
    }
    Serial.println();

    // SLEEP MODE
    enterSleepMode(120); // Wait for 2 minutes in sleep mode

  } else if(receivedDL = 3){

    // REFILL WATER
    refillWater();

    // REFILL FOOD
    refillFood();

    sensorRoutine();

    // Defining the uplink payload
    up_payload[0] = (byte)water;     
    up_payload[1] = (byte)food;      
    up_payload[2] = (byte)food_per; 
    up_payload[3] = 3; // The actuators have not been activated 
    mySerial.write(up_payload, 4); // Send the 3 bytes

    // RESET DL TO O
    receivedDL = 0;

    // Printing the payload
    Serial.print("Payload: ");
    for (int i = 0; i < 4; i++) {
      Serial.print(up_payload[i]);  // Imprime cada valor como decimal
      if (i < 3) Serial.print(", ");  // Separador entre valores
    }
    Serial.println();

    // SLEEP MODE
    enterSleepMode(120); // Wait for 2 minutes in sleep mode

  }

}

//////////////////////////// FUNCTIONS ////////////////////////////////////////

long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled.
  return microseconds / 29 / 2;
}

void sensorRoutine() {

  digitalWrite(ressupply, HIGH); // Power on water level sensor
  Serial.println("Calibrating Sensors");
  delay(3000); // Time to calibrate the sensors

  resval = analogRead(respin); //Read data from analog pin and store it to resval variable
  light = analogRead(ldrPin); // Read the LDR value

  // FOOD TANK ULTRASONIC SENSOR:
  long duration, cm;
  float food_tank = 20.0; // Total height of the food tank
  //int food_per;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  cm = microsecondsToCentimeters(duration); // This distance will vary according to the height of the food tank
  
  food_per = (int)(((food_tank - cm) / food_tank) * 100); // Food fill rate calculation:

  if (food_per > 100){

    food_per = 100;

  } else if(food_per < 0){

    food_per = 0;

  }


  Serial.print(food_per);
  Serial.print(" % of food"); 
  Serial.println();

  // FOOD POT LIGHT DETECTOR 
  lcd.setCursor(0,0);
  if (light > ldrThreshold) { // Check if the LDR value meets or exceeds the threshold
    lcd.print("Food: No ");
    Serial.print("No food: ");
    Serial.println(light);
    // Calculate the number of steps
    //int steps = degrees / degreesPerStep;
    // Rotate the motor
    //myStepper.step(steps);
    //Serial.print("Rotating ");
    food = 0;
  }else{
    lcd.print("Food: Yes");
    Serial.print("Food: ");
    Serial.println(light);
    food = 1;
  }

  // WATER LEVER POT SENSOR
  lcd.setCursor(0,1);
  if (resval <= 100) {
    Serial.println("Water Level: Empty");
    lcd.println("Water[%]: Empty ");
    water = 1;
  } else if (resval > 100 && resval <= 300) {
    Serial.println("Water Level: Low");
    lcd.println("Water[%]: Low   ");
    water = 2;
  } else if (resval > 300 && resval <= 330) {
    Serial.println("Water Level: Medium");
    lcd.println("Water[%]: Medium");
    water = 3;
  } else if (resval > 330) { 
    Serial.println("Water Level: High"); 
    lcd.println("Water [%]: High ");
    water = 4;
  }

  digitalWrite(ressupply, LOW);

}

void refillWater(){

  // Make servo go to 0 degrees when there is NO WATER 
  Servo1.write(0); 
  delay(1000); 
  // Make servo go to 90 degrees 
  Servo1.write(10); 
  delay(1000); 
  // Make servo go to 180 degrees 
  Servo1.write(0); 

}

void refillFood(){

  // Calculate the number of steps
  int steps = degrees / degreesPerStep;
  // Rotate the motor
  myStepper.step(steps);
  Serial.print("Rotating ");


void enterSleepMode(int seconds) {
  for (int i = 0; i < (seconds / 8); i++) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); // Entra en modo de bajo consumo durante 8 segundos
  }
}