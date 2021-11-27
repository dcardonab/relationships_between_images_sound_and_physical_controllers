//---------- Import Libraries ----------//

// oscP5 library contains an implementation of the OSC (Open Sound Control) protocol for Processing.
// oscP5 was written by Andreas Schlegel, and can be found at: http://www.sojamo.de/libraries/oscP5/
import oscP5.*;

// Import the Processing serial library
import processing.serial.*;


//---------- Create Global Variables ----------//

// oscP5 (Max)
OscP5 oscP5; // Create oscP5 variable of class OscP5.
float oscValue; // Variable to store data received via UDP.

// Serial (Arduino UNO)
Serial myPort; // Create myPort variable of class Serial.
String serialMessage; // Variable to store data received from the serial port.
// The final keyword is used to declare constants in Processing.
final int lf = 10; // ASCII linefeed will be used to determine the end of a line, as per the Arduino println() function.

// Amplitude and Color
// The amplitude, red, green, and blue values are continuously set by the oscEvent() function.
float amplitude = 0;
float red = 0;
float green = 0;
float blue = 0;

// Amplitude Scaling
final float scale = 6; // Set scaling factor to implement incoming values in a more usable range within Processing.
final float smooth_factor = 0.1; // Set Smoothing Factor to implement incoming amplitude values in a more visually stable way in Processing.
float sum = 0; // Variable used for evaluating values by taking into consideration the previous value.
float amp_scaled; // Variable for storing the scaled amplitude value that will be passed to define the size of the drawn ellipse.
int shortEdge; // Variable used to determine which edge to use in the scaling process.

// Variables to allocate values from Joystick controls.
int x_offset = 0; // The X position of the joystick will offset the X coordinate when drawing the ellipse.
int y_offset = 0; // The Y position of the joystick will offset the Y coordinate when drawing the ellipse.
boolean flip_switch = false; // Pressing the joystick switch will invert the colors of the drawn background and the drawn ellipse.

// Processing variables
boolean recording = false; // Boolean used to start video recording when there is a detected amplitude change. Recording will stop when the Processing window is stopped.
boolean firstRecording = false;


//---------- Setup ----------//

// Code runs once when Sketch is started
void setup()
{
  // Define Canvas size
  size(1024, 1024);
  
  // Determine which edge of the canvas is shorter
  // The value for the height variable is declared in the size() function.
  if (width < height)
  {
    shortEdge = width;
  }
  
  else
  {
    shortEdge = height;
  }
  
  // Set image Resolution
  // The displayDensity() function will return 2 if computer screen has a high-resolution display.
  // It will otherwise return 1, setting the pixelDensity, and thus, the quality.
  pixelDensity(displayDensity());
  
  // Set frameRate() to 30 frames per second.
  frameRate(30);
  
  // Hide Cursor when on Canvas
  noCursor();
  
  // Initialize an instance of OSC via UDP port 12000
  // This port brings in serial information from Max
  oscP5 = new OscP5(this, 12000);
  
  // Print available Serial ports into the Console to select an adequate port.
  printArray(Serial.list());
  
  // 6 represents the Serial port where the Arduino UNO is connected.
  String portName = Serial.list()[5];
  
  // Initiate a new serial instance in the myPort variable.
  myPort = new Serial(this, portName, 19200);
  
  // Empty serial Port prior to beginning of transmission.
  myPort.clear(); 
}


//---------- Draw ----------//

// Code runs continuously until stopped
void draw()
{ 
  //---------- Assign Joystick's serial data (from Arduino UNO) ----------//
  
  // Run code when myPort is available and sending data.
  while (myPort.available() > 0)
  {
    // serialMessage will store a string that is read until the ASCII linefeed terminator.
    serialMessage = myPort.readStringUntil(lf);
    
    // The message will be evaluated if it is not null
    if (serialMessage != null)
    {
      // The message will then be compared to see if it matches a series of defined identifiers that will trigger actions.
      // The tag 's' corresponds to the joystick pressed switch, and will invert the colors of the ellipse and the background.
      if (serialMessage.charAt(1) == 's')
      {
        // The flip_switch boolean variable is used below to control the behavior of what will be drawn.
        flip_switch = true;
      }
      
      // The tag 'r' corresponds to the joystick switch release, and will invert back to default the colors of the ellipse and the background.
      else if (serialMessage.charAt(1) == 'r')
      {
        flip_switch = false;
      }
      
      // The tag 'x' corresponds to the X axis of the joystick. It will move the width of the ellipse in the canvas.
      else if (serialMessage.charAt(1) == 'x')
      {
        // If the message matched tag 'x', read again to read the value of the joystick's X axis.
        String x = myPort.readStringUntil(lf);
        
        // Make sure the read value is not null.
        if (x != null)
        {
          // Use the trim function to rid the string of any whitespace (in this case, most likely ASCII character 13, which preceeds the linefeed terminator in every serial message).
          x = trim(x);
          
          // Convert the message into an int with the Java function Integer.parseInt().
          x_offset = Integer.parseInt(x);
          
          // map the X offset value to a range that is relevant to Processing.
          // Since the map function returns a float, it is necessary to round this value to the nearest int.
          x_offset = round(map(x_offset, 0, 1023, (-width / 2), (width / 2)));
        }
      }
      
      // The tag 'Y' corresponds to the Y axis of the joystick. It will move the height the ellipse in the canvas.
      else if (serialMessage.charAt(1) == 'y')
      {
        // Follow the same steps as the Joystick's X axis to read data from the Y axis.
        String y = myPort.readStringUntil(lf);
        
        if (y != null)
        {
          y = trim(y);
          y_offset = Integer.parseInt(y);
          y_offset = round(map(y_offset, 0, 1023, (-height / 2), (height / 2)));
        }
      }
    }
  }
  
  
  //---------- Assign Color and Amplitude data (from Max) ----------//
  
  // Apply smoothing factor to amplitude data to reduce jittery graphics
  sum += (amplitude - sum) * smooth_factor;
  
  // Scale incomming signal and apply scaling factor to size the drawn ellipse
  amp_scaled = sum * (shortEdge / 2) * scale;
  
  // Scale floating values between 0 and 1 to a value between 0 and 255
  float mappedColor = map(amplitude * scale, 0, 1, 0, 255);
  float mapRed = mappedColor * red;
  float mapGreen = mappedColor * green;
  float mapBlue = mappedColor * blue;
  
  
  //---------- Draw Ellipse and Background onto the Canvas screen ----------//
  
  // The ellipse and background colors will invert when the Joystick button is pressed, so that the background changes color instead of the ellipse.
  if (!flip_switch)
  {
    background(0);
    // Draw an ellipse dynamically using the size and color information
    // The fill() function takes in 3 arguments that are assigned to the values of RGB pixels.
    fill(mapRed, mapGreen, mapBlue);
    ellipse((width / 2 + x_offset), (height / 2 + y_offset), amp_scaled / 4, amp_scaled / 4);
  }
  
  // The colors will change in the ellipse by default, meaning when the Joystick button not pressed (or released).
  else
  {
    background(mapRed, mapGreen, mapBlue);
    // Draw an ellipse dynamically using the size and color information
    // The fill() function takes in 3 arguments that are assigned to the values of RGB pixels.
    fill(0);
    ellipse((width / 2 + x_offset), (height / 2 + y_offset), amp_scaled / 4, amp_scaled / 4);
  }
  
  //---------- Save Frames ----------//
  
  // Start recording when the sound starts and an amplitude is generated.
  // This only happens for the first recording.
  // Subsequent recordings can be started or stopped by pressing the 'r' key.
  if (amplitude > 0.05 && !firstRecording)
  {
    // Recording starts when this Boolean is set, as per the following if statement.
    recording = true; // Verify that there is sound coming through the system and start recording.
    firstRecording = true; // Verify that the first recording has been started to then use the r key as a recording toggle.
  }
  
  if(recording)
  {
    // Processing will render png image files/frames that can then be put together into a video file in a video editor.
    // Each frame will be consecutively numbered.
    // This numbering is according to the frameCount value, which runs in processing each time the code iterates:
    // Setup would be frame 0; the first iteration of draw would be frame 1...
    saveFrame("framesOut/movieOut_#####.png");
  }
}


//---------- Additional Functions ----------//

// The oscEvent() function receives and assigns serial data via UDP from Max
// The oscP5 instace calls the oscEvent function anytime it receives a message.
void oscEvent(OscMessage theOscMessage)
{
  // The message is captured in a float value
  oscValue = theOscMessage.get(0).floatValue();
  
  // The address pattern is checked to verify that it is the expected data.
  // /amp is the amplitude tag, as per the prepend object in Max/MSP, and it contains the amplitude of the audio signal.
  if (theOscMessage.checkAddrPattern("/amp"))
  {
    // The value of the message is assigned to the amplitude variable.
    // A threshold value is set to prevent non-transients from drawing content.
    float threshold = 0.1;
    
    // If the audio signal's amplitude is greater than the threshold, this value will be assigned to the amplitude variable.
    if (oscValue > threshold)
    {
      amplitude = oscValue;
    }
    
    // The amplitude will otherwise be 0.
    else
    {
      amplitude = 0.0;
    }
  }
  

  // Each of the random values assigned to pixels have been generated in Max by multiplying a random value
  // between 0 and 255 to a float between 0 and 1 that represents the amplitude of the signal.
  // This creates the effect of brighter colors at higher amplitudes.
  
  // The /red tag carries the semi-random data that Max generated for the red pixel.
  if (theOscMessage.checkAddrPattern("/red"))
  {
    red = oscValue;
  }
  
  // The /green tag carries the semi-random data that Max generated for the green pixel.
  if (theOscMessage.checkAddrPattern("/green"))
  {
    green = oscValue;
  }
  
  // The /blue tag carries the semi-random data that Max generated for the blue pixel.
  if (theOscMessage.checkAddrPattern("/blue"))
  {
    blue = oscValue;
  }
}

// Start and stop recordings with the 'r' key
// The r key will not work until the software automatically starts a recording by receiving an audio amplitude via UDP.
// The keyReleased function checks if a key in the keyboard has been pressed and then released.
void keyReleased()
{
  // Check that the firstRecording has been started, and that the r key has been pressed and released.
  if (firstRecording && (key == 'r' || key == 'R'))
  {
    // Change the boolean state of 'recording' to the opposite boolean state.
    recording = !recording;
  }
}
