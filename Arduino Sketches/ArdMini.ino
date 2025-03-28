#include <SoftwareSerial.h>
#include <rn2xx3.h>

// Define pins for SoftwareSerial
SoftwareSerial unoSerial(2, 3); // D2 as RX, D3 as TX
SoftwareSerial loraSerial(4, 5); // D4 as RX, D5 as TX for LoRa communication

rn2xx3 myLora(loraSerial);

// Constants and global variables
const byte AA = 0xAA;        // Define a constant for the AA byte
String downlinkData = "";    // To store received LoRaWAN downlink data
int Downlink = 0;

void setup() {
  // Initialize serial communication with the PC
  Serial.begin(9600);
  //mySerial.begin(9600); //serial port to radio
  initialize_radio();
  Serial.println("Pro Mini ready to receive data");
}

void loop() {
  // Activate unoSerial for communication with Arduino UNO
  unoSerial.begin(9600);

  // Check if there is data available on unoSerial
  if (unoSerial.available() != 0) {
    Serial.println("UNO Message Received");
    if (unoSerial.available() == 1) {
      // Single-byte message received
      byte receivedByte = unoSerial.read();
      Serial.println("Awake Message received");

      if (receivedByte == AA) {
        // Send the downlink data (in hexadecimal form) as a response
        unoSerial.write(Downlink);
        Serial.println("Downlink sent to UNO:");
        Serial.println(Downlink);
        Serial.println(downlinkData);
        if(Downlink <= 2){
          Downlink += 1;}else{Downlink = 0;}
      }
    } else if (unoSerial.available() >= 4) {
      // Multi-byte message received (4 bytes expected)
      byte receivedData[4];
      for (int i = 0; i < 4; i++) {
        receivedData[i] = unoSerial.read();
      }

      Serial.print("Received data: ");
      for (int i = 0; i < 4; i++) {
        Serial.print("0x");
        Serial.print(receivedData[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
      unoSerial.end(); // Deactivate unoSerial
      sendToAzure(receivedData, sizeof(receivedData));
    }
  } else {
    
  }

  delay(500); // Prevent spamming
}

// Function to initialize LoRa radio
void initialize_radio() {
  loraSerial.begin(9600);
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
  delay(500);
  digitalWrite(12, HIGH);

  delay(100);
  loraSerial.flush();

  // Autobaud the RN2483 module to 9600 baud
  myLora.autobaud();

  // Check communication with radio
  String hweui = myLora.hweui();
  while (hweui.length() != 16) {
    Serial.println("Communication with RN2xx3 unsuccessful. Power cycle the board.");
    delay(10000);
    hweui = myLora.hweui();
  }

  Serial.println("Register this DevEUI for OTAA: ");
  Serial.println(myLora.hweui());

  Serial.println("Joining TTN...");
  bool join_result = myLora.initOTAA("0000000000000000", "114465A28AF38BAE36CF451B328C2D45");

  while (!join_result) {
    Serial.println("Join failed. Retrying...");
    delay(1000);
    join_result = myLora.init();
  }
  Serial.println("Joined TTN successfully.");
  loraSerial.end(); // Deactivate LoRa communication after initialization
}

// Function to send data to Azure via LoRaWAN
void sendToAzure(byte *payload, size_t length) {
  loraSerial.begin(9600); // Activate LoRa communication
  Serial.println("Sending data to Azure...");
  
  if (myLora.txBytes(payload, length)) {
    Serial.println("Data sent successfully.");
  } else {
    Serial.println("Failed to send data.");
  }
  loraSerial.end(); // Deactivate LoRa communication
}

// Function to handle LoRaWAN data exchange
String XFerData(String msg) {
  loraSerial.begin(9600); // Activate LoRa communication
  String result = "";
  
  switch (myLora.txUncnf(msg)) {
    case TX_FAIL:
      Serial.println("TX unsuccessful.");
      break;
    case TX_SUCCESS:
      Serial.println("TX successful.");
      break;
    case TX_WITH_RX:
      String received = myLora.getRx();
      result = myLora.base16decode(received);
      break;
    default:
      Serial.println("Unknown response.");
      break;
  }

  loraSerial.end(); // Deactivate LoRa communication
  return result;
}

////////////////////// DOWLINK IMPLEMENTATION ///////////////////////

/*void loop()
{
    led_on();

    Serial.print("TXing");
    myLora.txCnf("!"); //one byte, blocking function

    switch(myLora.txCnf("!")) //one byte, blocking function
    {
      case TX_FAIL:
      {
        Serial.println("TX unsuccessful or not acknowledged");
        break;
      }
      case TX_SUCCESS:
      {
        Serial.println("TX successful and acknowledged");
        break;
      }
      case TX_WITH_RX:
      {
        String received = myLora.getRx();
        received = myLora.base16decode(received);
        Serial.print("Received downlink: " + received);
        break;
      }
      default:
      {
        Serial.println("Unknown response from TX function");
      }
    }
*/
