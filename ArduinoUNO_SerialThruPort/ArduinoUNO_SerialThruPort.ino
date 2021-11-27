// NOTES:
// This sketch makes use of 1 serial ports, and the SoftwareSerial library establish a serial communication between two Arduinos.
// Recommended board: Arduino UNO.

// Make use of the SoftwareSerial library.
#include <SoftwareSerial.h>

// Define the Arduino digital pins that will receive (RX) and transmit (TX) serial data.
SoftwareSerial mySerial(2, 3); // RX: Digital Pin 2, TX: Digital Pin 3

//---------- Setup ----------//

// Code runs only once when the script is unloaded.
void setup()
{
  // Initiate SoftwareSerial connection via the declared mySerial digital pins.
  // Initiate at a Baud rate of 19200.
  mySerial.begin(19200);

  // Initiate internal USB Serial connection to transfer incoming data directly into Processing.
  Serial.begin(19200);
}


//---------- Loop ----------//

// Code continuously loops once the setup() function has been executed.
void loop()
{
  // Condition checks if software serial port is available.
  if (mySerial.available() > 0)
  {
    // If yes, the data will be read from the software serial,
    // and immediately transeferred via USB serial to Processing.
    Serial.write(mySerial.read());
  }
}
