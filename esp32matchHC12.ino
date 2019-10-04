#include <SoftwareSerial.h>
//================= PART FOR CONTOLLING GARAGE==========================
#define RelayOff HIGH
#define RelayOn LOW

byte ledStatus = 0;
const byte ledPin = 13;
const byte garageControlPin = 6;
const byte bigGateControlPin = 5;
const byte pressDuration = 1500;
//================= PART FOR CONTOLLING GARAGE==========================
//================= PART FOR HC12==========================
const byte HC12RxdPin = 9; // Recieve Pin on HC12
const byte HC12TxdPin = 8; // Transmit Pin on HC12
const byte HC12SetPin = 7; // Transmit Pin on HC12

//unsigned long timer = millis(); // Delay Timer
char HC12ByteIn;    // Temporary variable

String HC12ReadBuffer = "";     // Read/Write Buffer 1 for HC12

boolean SerialEnd = false;    // Flag to indicate End of Serial String
boolean HC12End = false;      // Flag to indiacte End of HC12 String
boolean commandMode = false;  // Send AT commands

// Software Serial ports Rx and Tx are opposite the HC12 Rx and Tx
// Create Software Serial Port for HC12
SoftwareSerial HC12(HC12TxdPin, HC12RxdPin);
//================= PART FOR HC12==========================

void presentationBlink(void) {
  for (byte i = 0; i < 5; i++) {
    digitalWrite(ledPin, 1);
    delay(500);
    digitalWrite(ledPin, 0);
    delay(500);
  }
}

void setup() {
  pinMode(ledPin, OUTPUT); //LED pin
  pinMode(garageControlPin, OUTPUT); // Control Pin for Garage
  pinMode(bigGateControlPin, OUTPUT); // Remote Control Pin for Big Gate
  digitalWrite(garageControlPin, RelayOff);
  digitalWrite(bigGateControlPin, RelayOff);

  HC12ReadBuffer.reserve(64);     // Reserve 64 bytes for Serial message input
  pinMode(HC12SetPin, OUTPUT);    // Output High for Transparent mode / Low for Command mode
  digitalWrite(HC12SetPin, HIGH); // Enter Transparent mode
  delay(80);                      // 80 ms delay before operation per datasheet
  Serial.begin(57600);             // Open serial port to computer
  HC12.begin(2400);               // Open software serial port to HC12

  Serial.println("Hello from Garage Pro Mini!!!!!!");             // Hello message!

  presentationBlink();
}
// End of setup

void loop()
{
  while (HC12.available()) {  // While Arduino's HC12 soft serial rx buffer has data
    HC12ByteIn = HC12.read(); // Store each character from rx buffer in byteIn
    if (HC12ByteIn == '\n') { // At the end of the line
      HC12End = true; //Set HC12End flag to true
    } else {
      HC12ReadBuffer += char(HC12ByteIn); // Write each character of byteIn to HC12ReadBuffer
    }
  }

  if (HC12End) {    // If HC12End flag is true
    Serial.print("Received: ");
    Serial.print(HC12ReadBuffer.length());
    Serial.print(" bytes. Content: ");
    Serial.println(HC12ReadBuffer);     // Send received message to screen
    Serial.print("Reply with ACK+");
    Serial.print(HC12ReadBuffer);
    Serial.println("+!");
    HC12.print("ACK");                  // Reply with ACK+same message +"!"
    HC12.print(HC12ReadBuffer);         // Reply with ACK+same message +"!"
    HC12.println("!");                  // Reply with ACK+same message +"!"
    
    String passaON("Ciao: 4!");
    String passaOFF("Ciao: 6!");
    String passaApriSoloGrande("Ciao: 7!");
    String passaApriTutto("Ciao: 9!");
    if (!HC12ReadBuffer.compareTo(passaON)){
      ledStatus=1;
      digitalWrite(garageControlPin,RelayOn);
      delay(pressDuration);
      digitalWrite(garageControlPin,RelayOff);
      presentationBlink();
      Serial.println("Match ok! Opening Only Garage and Switching ON!!!!");
      digitalWrite(ledPin,ledStatus);
    } else if (!HC12ReadBuffer.compareTo(passaOFF)) {
      ledStatus=0;
      digitalWrite(garageControlPin,RelayOn);
      delay(pressDuration);
      digitalWrite(garageControlPin,RelayOff);
      presentationBlink();
      Serial.println("Match ok! Opening Only Garage and Switching OFF!!!!");
      digitalWrite(ledPin,ledStatus);
    } else if (!HC12ReadBuffer.compareTo(passaApriSoloGrande)) {
      ledStatus=0;
      digitalWrite(bigGateControlPin,RelayOn);
      delay(pressDuration);
      digitalWrite(bigGateControlPin,RelayOff);
      presentationBlink();
      Serial.println("Match ok! Opening Only Big Gate!!!!");
      digitalWrite(ledPin,ledStatus);
    } else if (!HC12ReadBuffer.compareTo(passaApriTutto)) {
      ledStatus=0;
      digitalWrite(garageControlPin,RelayOn);
      delay(pressDuration);
      digitalWrite(garageControlPin,RelayOff);
      presentationBlink();
      delay(6999);
      digitalWrite(bigGateControlPin,RelayOn);
      delay(pressDuration);
      digitalWrite(bigGateControlPin,RelayOff);
      presentationBlink();
      Serial.println("Match ok! Opening first Garage and then Big Gate!!!!");
      digitalWrite(ledPin,ledStatus);
    } else {
      Serial.print("Sorry I only received: ");
      Serial.println(HC12ReadBuffer);
      presentationBlink();
      digitalWrite(ledPin,ledStatus);
    }
    
    HC12ReadBuffer = ""; // Empty buffer
    HC12End = false; // Reset flag
    
  }
}
