// NOTES:
// This sketch makes use of 3 serial ports, available in the Arduino Mega 2560.
// Recommended board: Arduino Mega 2560.

//---------- Initiate Global Constants and Variables ----------//

// constants and variables
const int button_pin = 3; // The Button's digital pin is pin 3.
const int led_pin = 13; // Initialize led_pin digital pin (pin 13) to visually indicate that the Button has been pressed.
int button_state = 0; // Variable to store Button's current status.
int previous_state = 0; // Variable to store Button's previous status.

// Variables for Potentiometer
const int p_pin = A2; // Potentiometer connected to Analog Pin 2.
int p_value; // Variable to store Potentiometer's current value.
int p_prevalue = -1; // Variable to store Potentiometer's previous value intialized to a value other than 0.

// Variables for Joystick
const int SW_pin = 2; // The Joystick's switch output is assigned to digital pin 2.
const int X_pin = A0; // Analog pin 0 is connected to X output.
const int Y_pin = A1; // Analog pin 1 is connected to Y output
int j_switch = 0; // Variable for reading the joystick switch status.
int note = 36; // Variable for storing a note that will be triggered when pressing the joystick. Initiated to 36 to play two octaves below middle C via MIDI.


//---------- Setup ----------//

// Code runs only once when the script is unloaded.
void setup()
{
  // Configure Button signal flow
  pinMode(button_pin, INPUT); // "button_pin" as INPUT
  pinMode(led_pin, OUTPUT); // "led_pin" as OUTPUT

  // Configure Potentiometer signal flow
  pinMode(p_pin, INPUT); // "p_pin" as INPUT

  // Configure Joystick signal flow
  pinMode(SW_pin, INPUT); // "SW_pin" constant as INPUT
  digitalWrite(SW_pin, HIGH); // Change state of resistor to HIGH

  // Initialize Serial for communicating with Max at a 19200 Baud rate.
  // This is the computer's internal port.
  Serial.begin(19200);

  // Initialize a second Serial connection for communicating with MIDI devices.
  // This Serial connection will communicate via Digital Pins 18 and 19 (TX1, RX1).
  // In this case, data will only be transmitted,
  // requiring only a 5-pin MIDI connector to be connected to Pin 18. 
  // Set connection for MIDI at 31250 Baud rate (MIDI Baud rate).
  Serial1.begin(31250);

  // Initialize a third Serial connection for communicating with Processing via the USB serial port of an Arduino UNO.
  // This Serial connection will communicate via Digital Pins 16 and 17 (TX2, RX2).
  // In this case, data will only be transmitted,
  // requiring only a connection to Pin 16 of the Arduino MEGA to Pin 2 of the Arduino UNO.
  // Set connection with Arduino UNO at 19200 Baud rate.
  Serial2.begin(19200);
}


//---------- Loop ----------//

// Code continuously loops once the setup() function has been executed.
void loop()
{
  
  //---------- Button ----------//
  
  // Read button state into the button_state variable.
  button_state = digitalRead(button_pin);

  // Both button_state and previous_state are both initated to 0,
  // meaning that this code will not run unless the button is pressed, and thereof, changing the button_state value.
  if (button_state != previous_state)
  {
    // A button pressed will change the state to HIGH,
    // meaning that this code will not be triggered until the button is released.
    if (button_state != HIGH)
    {
      // Cause Digital Pin to blink.
      digitalWrite(led_pin, LOW);

      // Transmit serial to Max via the default Serial port.
      // println is used to include ASCII linefeeds (13 10) as message delimeters.
      Serial.println("A");
    }
  }

  // Update previous_state for next press
  previous_state = button_state;


  //---------- Potentiometer ----------//
  
  int p_value = analogRead(p_pin); // Read potentiometer current value into p_value.

  // Arduino's Potentiometer runs at 5 Volts.
  // These 5 volts are represented digitally by 1024 values (0 through 1023).
  // The potentiometer is sensitive to movement. The if condition prevents
  // messages from being sent unless the amount of change is more than 10.
  if (abs(p_value - p_prevalue) > 10)
  {
    // Send Potentiometer value to Max via the default Serial port.
    Serial.println(p_value);
    
    // Update p_previous for next change in value.
    p_prevalue = p_value;

    // Add a 10ms delay to not saturate the port with information.
    delay(10);
  }


  //---------- Joystick ----------//

  // The joystick data will be sent via serial ports 2 and 3 (Serial1 and Serial2 respectively):
  
  // Serial1 will send MIDI data via the midiNote() and midiCC() functions found after the loop() function.
  
  // Serial2 will send Serial messages to Arduino UNO, so that these messages can in turn be transmitted to Processing.
  // Arduino UNO's role is creating a Serial channel to communicate with Processing.
  // This is because the Arduino MEGA is using its internal USB serial connection to communicate Max,
  // and Serial information cannot be sent via the same port to multiple destinations.
  
  // Detect if the switch is pressed.
  if ((digitalRead(SW_pin) == 0) && (j_switch == 0))
  {
    // 0x90 corresponds to the note on command in the MIDI protocol.
    // The note variable will select the pitch that will be played.
    // The number 69 represents the velocity with which the synth will be triggered.
    midiNote(0x90, note, 69);

    // Use the /s tag for Processing to recognize that the joystick's switch has been pressed.
    Serial2.println("/s");

    // Change the state of j_switch to prepare to send messages when the joystick is released.
    j_switch = 1;
  }

  // Detect if the joysetick switch is released.
  else if ((digitalRead(SW_pin) == 1) && (j_switch == 1))
  {
    // 0x80 corresponds to the note off command in the MIDI protocol.
    // The note variable will select the pitch that will be stopped from playing.
    // The number 0 represents a closed velocity to mute the synthesizer's envelope generator.
    midiNote(0x80, note, 0);

    // Change the state of j_switch to prepare to send messages when the joystick is pressed.
    j_switch = 0;
    
    // Use the /r tag for Processing to recognize that the joystick's switch has been released.
    Serial2.println("/r");
  }

  // Read Joystick's X axis current value into x.
  int x = analogRead(X_pin);
  
  // Similarly to the potentiometer, the X and Y axis run at 5 Volts
  // and are thus expressed on a scale of 1024 values, ranging from 0 to 1023.
  // The map function is used to scale the data to values between 0 and 127
  // so that the MIDI protocol can use the value.
  int audioX = map(x, 0, 1023, 0, 127);

  // Send serial information to modulate CC controllers in the analog synth only if joystick's X axis is not centered.
  if (audioX < 58 || audioX > 70)
  {
    // Send MIDI information to modulate continuous controller parameters.
    midiCC(0xB0, 74, audioX); // MIDI CC to Moog Sub37 OSC 1 Octave is CC 74.
    midiCC(0xB0, 75, audioX); // MIDI CC to Moog Sub37 OSC 2 Octave is CC 75.
  }
  
  // Send serial information to control graphics in Processing from joystick's X axis.
  // Use the /x tag for Processing to recognize that the message is coming from the joystick's X axis.
  Serial2.println("/x");
  // Send the value of x immediately after sending out the /x tag.
  Serial2.println(x);


  // Read Joystick's Y axis current value into y.
  int y = analogRead(Y_pin);

  // Map the value to MIDI range.
  int audioY = map(y, 0, 1023, 0, 127);

  // Send serial information to modulate CC controllers in the analog synth only if joystick's Y axis is not centered.
  if (audioY < 58 || audioY > 70)
  {
    // Send MIDI information to modulate continuous controller parameters.
    midiCC(0xB0, 9, audioY); // MIDI CC to MoogSub37 OSC 1 Waveshape is CC 9.
    midiCC(0xB0, 14, audioY); // MIDI CC to Moog Sub37 OSC 2 Waveshape is CC 14.

    // Invert Y axis control for the Multridrive parameter.
    audioY = map(audioY, 0, 127, 127, 0);
    midiCC(0xB0, 18, audioY); // MIDI CC to Moog Sub37 Filter Multidrive is CC 18.
  }

  // Send serial information to control graphics in Processing from joystick's Y axis.
  // Use the /y tag for Processing to recognize that the message is coming from the joystick's Y axis.
  Serial2.println("/y");
  // Send the value of y immediately after sending out the /y tag.
  Serial2.println(y);
}


//---------- MIDI Functions ----------//

// These functions send Serial MIDI information out of the second Serial port that was created (Serial1),
// at a Baud rate of 31250 (MIDI Baud rate). These instructions go to MIDI devices via a 5-pin MIDI physical connection.

// The midiNote function is used to start and stop MIDI notes.
// The function needs either an int or a hexadecimal value to represent commands.
// The function assumes that the pitch and the velocity inputs will be between 0 and 127.
void midiNote(int command, int pitch, int velocity) {
  // The note on MIDI message is used to start MIDI notes by sending the hexadecimal command 0x90 (or 144),
  // the pitch that will be triggered, and the desired velocity with which that pitch will be triggered.
  // The note off MIDI message is used to stop MIDI notes by sending the hexadecimal command 0x80 (or 128),
  // the pitch that will stopped, and a velocity of 0 to instruct that pitch to stop.
  Serial1.write(command);
  Serial1.write(pitch);
  Serial1.write(velocity);
}

// The midiCC function is used to transmit MIDI Continuous Controller (CC) messages.
// The function needs the MIDI CC command 0xB0 (or 176) as its command input.
// The function also needs a CC parameter number (which will control various functions of a synthesizer, depending on how MIDI has been mapped).
// MIDI CC parameters are more or less standardized according to the MIDI specifications:
// https://www.midi.org/specifications-old/item/table-3-control-change-messages-data-bytes-2
// For this example, a Moog Sub37 is being triggered. The Sub37's MIDI CC information can be found in the manual on pages 53 through 55:
// https://api.moogmusic.com/sites/default/files/2018-09/SUB_37_MANUAL_v1.1_0.pdf
// Finally, the function needs a value (between 0 and 127) that will be set for the given CC parameter. 
void midiCC(int command, int ccParameter, int ccValue) {
  Serial1.write(command);
  Serial1.write(ccParameter);
  Serial1.write(ccValue);
}
