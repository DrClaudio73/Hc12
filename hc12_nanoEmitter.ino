#include <SoftwareSerial.h>

const byte HC12RxdPin = 4; // Recieve Pin on HC12
const byte HC12TxdPin = 5; // Transmit Pin on HC12
const byte HC12SetPin = 6; // Transmit Pin on HC12

//unsigned long timer = millis(); // Delay Timer
char SerialByteIn;  // Temporary variable
char HC12ByteIn;    // Temporary variable

String HC12ReadBuffer = "";     // Read/Write Buffer 1 for HC12
String SerialReadBuffer = "";   // Read/Write Buffer 2 for Serial

boolean SerialEnd = false;    // Flag to indicate End of Serial String
boolean HC12End = false;      // Flag to indiacte End of HC12 String
boolean commandMode = false;  // Send AT commands

// Software Serial ports Rx and Tx are opposite the HC12 Rx and Tx
// Create Software Serial Port for HC12
SoftwareSerial HC12(HC12TxdPin, HC12RxdPin);

void setup() {
  HC12ReadBuffer.reserve(64);     // Reserve 64 bytes for Serial message input
  SerialReadBuffer.reserve(64);   // Reserve 64 bytes for HC12 message input
  pinMode(HC12SetPin, OUTPUT);    // Output High for Transparent mode / Low for Command mode
  digitalWrite(HC12SetPin, HIGH); // Enter Transparent mode
  delay(80);                      // 80 ms delay before operation per datasheet
  Serial.begin(9600);             // Open serial port to computer
  HC12.begin(2400);               // Open software serial port to HC12
  Serial.println("Hello from Pro Mini!!!!!!");             // Hello message!
}

int k=0;

void loop() {
  while (HC12.available()) {  // While Arduino's HC12 soft serial rx buffer has data 
    HC12ByteIn = HC12.read(); // Store each character from rx buffer in byteIn 
    if (HC12ByteIn == '\n') { // At the end of the line
      HC12End = true; //Set HC12End flag to true
    } else {
        HC12ReadBuffer += char(HC12ByteIn); // Write each character of byteIn to HC12ReadBuffer 
    }    
  }

  if (HC12End) {    // If HC12End flag is true
    if (HC12ReadBuffer.startsWith("ACK")) {  // Check to see if a ACK is recevied from remote
        Serial.print(" Received ACK: ");    // Display ACK on screen
        Serial.print(HC12ReadBuffer);    // And does not reply again
        Serial.println("");
      } else {
        Serial.println(HC12ReadBuffer);    // Send message to screen
        HC12.print("ACK");    // Reply with ACK+same message +"!"
        HC12.print(HC12ReadBuffer);    // Reply with same message
        HC12.println("!");    // Reply with same message
    }
      HC12ReadBuffer = ""; // Empty buffer
      HC12End = false; // Reset flag
    }

    HC12.print("BEF0BEF");
    HC12.println(k);          
    Serial.print("Sending BEF0BEF");    // Send local command to remote HC12 before changing settings
    Serial.println(k);                
    k++; 
    delay(2000);       
}
