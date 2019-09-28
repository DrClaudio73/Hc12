#include <LiquidCrystal.h> // include the LCD library

const int rs = PB14, en = PB13 , d4 = PC14, d5 = PA7, d6 = PB10, d7 = PB11; //mention the pin names to with LCD is connected to
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //Initialize the LCD

const byte HC12SetPin = PB12; // Transmit Pin on HC12
const byte LedPin = PC13; // Led Pin on STM32

int ledStatus = 0;
char SerialByteIn;  // Temporary variable
char HC12ByteIn;    // Temporary variable

String HC12ReadBuffer = "";     // Read/Write Buffer 1 for HC12
String SerialReadBuffer = "";   // Read/Write Buffer 2 for Serial
String NumRcvStr = "";

boolean SerialEnd = false;    // Flag to indicate End of Serial String
boolean HC12End = false;      // Flag to indiacte End of HC12 String
boolean commandMode = false;  // Send AT commands

// Software Serial ports Rx and Tx are opposite the HC12 Rx and Tx
// Create Software Serial Port for HC12
//SoftwareSerial HC12(HC12TxdPin, HC12RxdPin);
const byte HC12RxdPin = PA2; // "RXD" Pin on HC12
const byte HC12TxdPin = PA3; // "TXD" Pin on HC12

HardwareSerial SerialPC(PA10, PA9);
HardwareSerial SerialHC12(HC12TxdPin, HC12RxdPin);

void setup() {
  HC12ReadBuffer.reserve(64);     // Reserve 64 bytes for Serial message inpu
  SerialReadBuffer.reserve(64);   // Reserve 64 bytes for HC12 message input
  NumRcvStr.reserve(9); 
  
  pinMode(HC12SetPin, OUTPUT);    // Output High for Transparent mode / Low for Command mode
  
  digitalWrite(HC12SetPin, HIGH); // Enter Transparent mode
  delay(80);                      // 80 ms delay before operation per datasheet
  SerialPC.begin(9600);             // Open serial port to computer
  SerialPC.println("Hello from STM32 (new)!!!!");
  SerialHC12.begin(2400);            // Open software serial port to HC12
  pinMode(LedPin, OUTPUT);    // Output High for Transparent mode / Low for Command mode
  lcd.begin(16, 2);//We are using a 16*2 LCD
  lcd.setCursor(0, 0); //At first row first column
  lcd.print("Testing HC12"); //Print this
  lcd.setCursor(0, 1); //At secound row first column
  lcd.print("- Claudio C."); //Print this
  delay(2000); //wait for two secounds
  lcd.clear(); //Clear the screen
}

byte Num_Rcv = 0;

void loop() {
    char cstr[16];
    /*lcd.setCursor(0, 0); //At first row first column
    lcd.print("STM32 -Blue Pill"); //Print this
    lcd.setCursor(0, 1); //At secound row first column
    lcd.print(millis() / 1000); //Print the value of secounds
    delay(1500);
    lcd.clear();*/
  while (SerialHC12.available()) {  // While Arduino's HC12 soft serial rx buffer has data
    HC12ByteIn = SerialHC12.read(); // Store each character from rx buffer in byteIn
    SerialPC.print(HC12ByteIn);
    if (HC12ByteIn == '\n') { // At the end of the line
      HC12End = true; //Set HC12End flag to true
    } else {
      HC12ReadBuffer += char(HC12ByteIn); // Write each character of byteIn to HC12ReadBuffer
    }
  }

  while (SerialPC.available()) {                // If Arduino's computer rx buffer has data
    SerialByteIn = SerialPC.read();             // Store each character in byteIn
    SerialReadBuffer += char(SerialByteIn);   // Write each character of byteIn to SerialReadBuffer
    if (SerialByteIn == '\n') {               // Check to see if at the end of the line
      SerialEnd = true;                       // Set SerialEnd flag to indicate end of line
    }
  }

  if (SerialEnd) { // Check to see if SerialEnd flag is true
    if (SerialReadBuffer.startsWith("ACK")) {  // Has a command been sent from local computer
    }  else {
      SerialHC12.println(SerialReadBuffer);  // Transmit non‐command message
    }
    SerialReadBuffer = ""; // Clear SerialReadBuffer
    SerialEnd = false; // Reset serial end of line flag
  }

  if (HC12End) {    // If HC12End flag is true
    if (HC12ReadBuffer.startsWith("ACK")) {  // Check to see if a ACK is recevied from remote
      SerialPC.print("Received ACK: ");    // Display ACK on screen
      SerialPC.println(HC12ReadBuffer);    // And does not reply again
    } else {
      Num_Rcv++;
      /* SHOW DEBUG ON LCD*/
      lcd.clear(); //Clear the screen
      lcd.setCursor(0, 0); //At first row first column
      lcd.print("RCV:");
      lcd.print(HC12ReadBuffer); //Print this
      lcd.setCursor(0, 1); //At secound row first column
      lcd.print("s");
      lcd.print(millis() / 1000); //Print the value of seconds
      lcd.print("#Rx:"); //Print the value of received packets
      sprintf(cstr, "%04d", Num_Rcv);
      lcd.print(cstr); //Print the value of recevied messages
      //lcd.print(Num_Rcv); //Print the value of recevied messages
      /* SHOW DEBUG ON SERIAL PORT*/
      //SerialPC.println(HC12ReadBuffer);    // Send message to screen
      SerialPC.print("RCV: ");
      SerialPC.print(HC12ReadBuffer); //Print this
      SerialPC.print(" s: ");
      SerialPC.print(millis() / 1000); //Print the value of seconds
      SerialPC.print(" #RCV: "); //Print the value of received packets
      SerialPC.println(Num_Rcv); //Print the value of recevied messages
      /* REPLY WITH ACK */
      SerialHC12.print("ACK");    // Reply with ACK+same message +"!"
      SerialHC12.print(HC12ReadBuffer);    // Reply with same message
      SerialHC12.println("!");    // Reply with same message
      SerialPC.print("Sending ACK:");    // Reply with ACK+same message +"!"
      SerialPC.print(HC12ReadBuffer);    // Reply with same message
      SerialPC.println("!");    // Reply with same message
    }
    HC12ReadBuffer = ""; // Empty buffer
    HC12End = false; // Reset flag
  }

  ledStatus=1-ledStatus;
  digitalWrite(LedPin,ledStatus);
  //delay(150);
}
