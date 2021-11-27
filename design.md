
David Cardona – CS50 Final Project – Spring 2020

Relationships between Images, Sound and Physical Controllers


//---------- .zip Package Contents ----------//

DESIGN.md
    Document with technical explanations of the project, with an explanation of each included file, and with a list of resources and definitions that were essential to complete the project.

README.md
    Document for a user to understand, build, and execute the installation. This file includes an explanation of the project and the controls, a hardware and software setup sections, and useful notes to render the performance into an audio file, into images/frames, and ultimately, into a video file.

SignalFlow.png
    Diagram outlining the signal flow of the installation.

ArduinoMEGA_Main folder
    This folder contains the 'ArduinoMEGA_Main.ino' sketch. This sketch must be uploaded to the Arduino MEGA 2560 board. Arduino files (.ino) must be stored in a folder with the same name as the Arduino sketch.

ArduinoUNO_SerialThruPort folder
    This folder contains the 'ArduinoUNO_SerialThruPort.ino' sketch. This sketch must be uploaded to the Arduino UNO board.

Max_Host folder
    This folder contains the 'Max_Host.maxpat' sketch.
    Although Max files do not required to be stored in a folder with the same name, I opted for storing the file in this folder so that the audio render that is created when the sequence is run is stored in this folder under the file name 'audioOut.wav'.
    This folder also contains a series of screenshots showing the Max main patcher and subpatchers. This has been provided to show the code in case the user does not have access to Max.

Processing_audioGraphics folder
    This folder contains the 'Processing_audioGraphics.pde' file.
    Just like Arduino, Processing requires a sketch to be stored in a folder under the same name. This is helpful in this situation, because the sequentially numbered png frames that Processing saves will be stored in a folder at this directory under the name 'framesOut'.


//---------- Challenges ----------//

This project's implementation incorporated challenging technical difficulties, the largest by far being setting up correct Serial connections between the various elements of the installation. In detail, the established serial communications reflect the following signal flow:
    1. Arduino MEGA 2560 default serial port (RX0 + TX0) to Max via USB at a Baud rate of 19200.

    2. Arduino MEGA 2560 second serial port (RX1 + TX1) to MIDI synthesizer via MIDI serial at a Baud rate of 31250 (standard MIDI Baud rate).

    3a. Arduino MEGA 2560 third serial port (RX2 + TX2) to Arduino UNO via a jumper cable using the Arduino Software Serial library. This connection is established at a Baud rate of 19200.

    3b. Arduino UNO to Processing via USB at a Baud rate of 19200. The Arduino UNO is thus used for passing serial data from the Arduino MEGA 2560 directly into Processing. This is necessary because both Arduino boards have only one serial port available via USB, and since a serial connection is a one to one connection, it would otherwise not be possible to send data to both Processing and Max from Arduino the same Arduino.

    4. Crow to Max serial USB connection. When the connection is made, it is automatically established for both devices to operate at the same Baud rate.

    5. Max to Processing transferring OSC (Open Sound Control) via a UDP (User Datagram Protocol).


Basic soldering of the Potentiometer and the MIDI connector were also necessary for a couple of reasons:
    1. The Arduino potentiometer is heavily unstable when connecting the pins via a breadboard (as opposed to soldering male jumper cables to the potentiometer pins).
    2. The MIDI connector also needs jumper cables directly soldered onto the the main 3 MIDI pins to properly trasmit data.


//---------- Structure ----------//

The signal flow is outlined in the included 'SignalFlow.png' file.

The Arduino controllers will be connected to the following pins of the Arduino MEGA 2560 board:
    - Joystick SW pin to PWM (Pulse Width Modulation) pin 2.
    - Joystick VRx to Analog In pin A0.
    - Joystick VRy to Analog In pin A1.
    - Potentiometer to Analon In pin A2.
    - Button to PWM pin 3.
    - MIDI connector to Communication pin TX1 18.

It is essential to connect a cable from Arduino MEGA 2560 pin TX2 16, and going into Arduino UNO Digital PWM pin 2. This connection will transfer serial data from the Arduino MEGA 2560 to the Arduino UNO to Processing. In this way, Arduino MEGA 2560 will be sending serial information to Max via the USB serial port, and will send serial information to Processing via the Arduino UNO USB serial port.


//---------- Scripts ----------//

There are four scripts that were written for this project. This section will outline the basic functionality of each one of the scripts. For additional information, each one of the scripts is thoroughly commented.

Max_Host.maxpat
    Max is serving as the main host for this project.
    The patch consists of a main patch, and five subpatches accessed via the 'p' patcher object:
        - Main patch
            This patch has instructions and the necessary buttons and toggles for setting up the serial connections between the inteacting devices.
            It also has the buttons to create audio files and start audio.
            It also includes all of the necessary connections between the subpatchers. These connections are established via patch cords and via the 's' send and 'r' receive objects.

        - p crow_module subpatch
            This is the same setup patch provided in the Crow reference for setup.
            It receives two inputs:
                - Input 1 sends an "open" message to the 'pcontrol' object, which opens a floating window from where Crow's serial port can be selected. This input is fed by the 'Initiate' button in the 'Crow' section of the main patch's interface.
                - The second input sends a "print('connected')" message to the 'crow' object to print the message 'connected' onto the Max Console via the 'print' object. This input is fed by the 'Verify' button in 'Crow' section of the main patch's interface.

            Both selecting and verifying a connection will cause a bang to be sent out to the main interface to visually confirm that a connection was triggered. This bang also is also sent to a second subpatcher that sends reset messages to Crow for initializing all of its inputs and outputs to the default settings.

            The rest of the objects in the patch handle sending and reciving instructions from Crow. This project is only sending information to Crow, which is why it is important to note the 'r commands_to_crow' object. This object will receive the output messages of the 'p data_to_crow' subpatcher to feed it into the crow object (see 'p data_to_crow' below for information about output formatting).

        - p arduino_input subpatch
            This subpatch receives 4 inputs:
                - A toggle to regularly check if Arduino's serial port is available, and in case it is, the 'serial' object will read from this port at regular intervals. By default the regularity of these intervals is 10ms.
                - A number set by a 'slider' object in the main patch. This number will change the regularity of the serial read intervals. The slider ranges from 10ms to 100ms.
                - A bang input to print the available serial ports onto the Max Console via the 'serial' object. A bang in max is a pulse-like signal that instructs objects in Max to execute their action.
                - A menu in the main patch (via the 'umenu' object) to select the port that corresponds to the Arduino MEGA 2560.

            Once a connection has been established, the patch will read serial data and perform multiple actions in the following order:
                - The 'print' object will print the raw serial information onto the Max Console.
                - The 'zl' object will use the 'group' method to group packages of information. 'zl' knows when to finish a grouping based on a bang it receives from the 'sel' object, which sends a bang each time it encounters an ASCII termination method (CR LF or 13 10). This is the delimeter because the function that sends serial data from Arduino to Max is a println function.
                - Since the data-type that the 'serial' object outputs is lists of int that correspond to ASCII characters, it is necessary to convert this data to UTF-8 (via the 'itoa' object), to then convert this character to a symbol that Max understands via the 'fromsymbol' object.

            The outputs of the subpatch are configured as follows:
                - 'fromsymbol' sends messages into the 'route' object. 'route' will receive the message, and filter the information based on its arguments. In this case, if the message received matches an A, 'route' will send a bang and the rest of the message out of its left outlet. Since Arduino MEGA 2560 is only sending the A tag, no "rest of the message" will be sent, but the bang will trigger (via the subpatcher outlet 1 into the main patcher) a switch that starts and stops the sequence of pulses that are sent to Crow.
                - If the input of 'route' does not match the A tag, the rest of the message will be sent out into a 'scale' object. The other information that Arduino MEGA 2560 is sending is the data of the potentiometer. This data will be 'scaled' into a given range. The 5 Volts of the Arduino potentiometer are read by in a scale from 0 to 1023. This data will be used in the main patcher to control the speed of the sequence in milliseconds that represent a quarter note. Musical tempo ranges vary a lot, but for this project, I found the range from 300ms (200bpm) to 715ms (circa 86bpm) to yield very musical results. Once the data has been scaled, it is sent out into the main patcher via outlet 2.

        - p data_to_crow subpatch
            The main patcher routes the information from both of the 'p arduino_input' outlets into 'p data_to_crow'.

            This information is sent to multiple places to generate the audio pulses sent out from Crow:
                - The button input (inlet 1; activated with a toggle) starts and stops the 'metro' object sending triggers at the specified millisecond rate.
                - This rate is specified by the value coming through the potentiometer input (inlet 2), which is the millisecond value assigned by the 'scale' object in the arduino_input subpatcher. This number uses a 'line' object, which smooths the change between values by a given number of milliseconds, in this case 100ms.

            The bangs sent out of the 'metro' object will then be sent to 3 destinations to format output channels 1, 2, and 4.
            The toggle that triggers the 'metro' object will also be sent to format output channel 3.
            The layout of the channels is:
                Channels 1 and 4 are identical. Different channels were made to incorporate individual bypass channel buttons:
                    - The 'metro' bangs go into a 'gswitch' object, which serves as a selector of multiple inputs to one output. The passing input of gwitches is changed by a bang message sent to the object's left input. In this project, the message is being delivered by a series of 'toggle' objects that send bypass messages from the Main interface (via inlets 4 and 7 of the data_to_crow subpatcher).
                    - The gswitch routes its output to the right inlet of the 'gate' object. The gate is receiving its output method from its left inlet, which is fed by the "Set Outputs to Pulse" menu in the Crow section of the main patch. For this project, the "Pulse" option must be selected, as this instructs gate to open and close very quickly. 'gate' will the send a pulse each time it receives a bang in its right intel.
                    - The 'gate' pulses are received by messages formatted to send pulses out of Crow outlets 1 and 4. The messages follow the syntax: "output[ID](pulse(sec, V, pol))", where the 'ID' is the output channel number, 'sec' is the length of the pulse, 'V' is the voltage of the pulse, and 'pol' is the polarity of the pulse.

                Channel 2 is very similar to Channels 1 and 2, with the exception that the bangs that it receives are delayed by a value that is equal to an eight note. This is done by sending the 'metro' bangs to a delay object, which will delay the received messages by a number of milliseconds that is equal to half the 'metro' rate. This is calculated with the '/' divide object. The 'delay' will output the received bangs into a gswitch, and from there, the signal follows the same processes as channels 1 and 4. The bypass switch for channel 2 comes via inlet 5 of the subpatcher.

                Channel 3 is different from the other channels in the sense that it is not going to be configured to send out pulses. The functionality of this output is to send and maintain a voltage of 5 volts as long as the sequence (activated via the Arduino button) is running. Once the sequence stops, the voltage will be set to 0 volts. The following actions were setup in order to achieve this:
                    - A 'gswitch2' object is used a a selector of one input to multiple outputs. In this case, I patched the same bang received by the Arduino button toggle to go into both inlets of 'gswitch2'. Because the order of operations in Max are from right to left, and from bottom to top, 'gswitch2' will first receive the bang on its right inlet (sending the message out the outlet) and then on its left inlet (switching the output to the next inlet). The bypass button of channel 6 is fed via inlet 6 of the subpatcher.
                    - The outlets of 'gswitch2' go into messages that set the voltage in a Crow outlet to a specified value. The syntax for this is: "output[ID].volts = value", where ID is the output channel number, and value is the voltage that will be outputted (ranging from -5 to 10).

            Once triggered, all Crow instruction messages converge at the outlet of the subpatcher, which is sent in the main patcher to the 's commands_to_crow' object, which is received in the 'p crow_module' subpatch to send messages to Crow.
            Note that all pulses and set values will be sent out of Crow outputs as CV (Control Voltage), which is used to operate analog modulay synthesizers.

        - p record_audio subpatch
            Note that Max uses the '~' chatacter to denote MSP objects, which are objects that interact with audio signals rather than with standard numbers and messages.
            This subpatcher renders audio using the 'sfrecord~' object. 'sfrecord~' is declared with an argument of 2 to allow 2 inlets that will be rendered into a stereo audio file. This audio signals are fed via inlets 1 and 2, and activated via the 'ezadc~' object, which is used to start/stop the audio converters of the connected unit. ADC stands for Analog-to-Digital converter, and they are used for bringing audio signals into a computer. DAC stands for Digital-to-Analog converter, and they are used for bringing audio signal out of a computer.

            'sfrecord~' also receives a series of messages to select the capture resolution of the audio file:
                - Inlet 3 feeds a toggle that will start and stop the audio recording.
                - Inlet 4 feeds a bang into the "open audioOut wave" to create a destination wave file named: audioOut.wav
                - Inlet 5 triggers multiple messages:
                    - the "resample $1" message will set the sample rate of 'sfrecord~' outputted file to a different sample rate. For example, if Max is running at a sample rate of 48000kHz, an argument of 1 will keep the sample rate the same, an argument of 2 will double the sample rate to 96000kHz, etc. The '$' symbol is used in Max to create place holders, where '$1' corresponds to the first placeholder, '$2' corresponds to the second placeholder, and so on.
                    - The "int24" message will be fed into the 'prepend' object, which has been given an argument of samptype. 'prepend' will add its argument to its input message, outputting in this case "samptype int24". This message will instruct 'sfrecord~' to render each audio sample at a bit-depth resolution of 24-bits. The menu can be used to select other common bit-depth audio resolutions.
                    - The "nchans $1" message will set the number of channels in 'sfrecord~'. This is a stereo file by default, but the message was included so that multichannel or mono recordings can be rendered ('sfrecord~' will not collapse stereo to mono but read only from its first inlet instead).

            The 'number~' object that 'sfrecord~' is connected to will display the values of a signal that it receives. It is also an effective way to verify that audio is being recorded, which is why it is also included in the Main patcher window.

        - p data_to_processing subpatch
            The audio signal from the audio interface is also fed to the data_to_processing subpatcher via inlets 1 and 2.

            The information that will be generated for sending to Processing will be based on a mono signal. To sum a stereo signal into mono (with the '+~' object), it is first necessary to scale down the signals. Since the curve that outlines perception of sound is logarithmic, for a reduction of 3dB in the audio signal, it should be multiplied by a float value of 0.71 with the '*~' object.
            The resulting summed mono signal goes into a 'gswitch' that acts as a signal bypass. The toggle for this bypass can be found in the Main patcher window with the name 'Bypass Graphics', and it is fed into the subpatcher via inlet 3.

            The signal then splits in two routes to be analyzed differently:
                The first route assigns a float value between 0 and 1 to the amplitude of the signal. This value will be passed to Processing to determine the size of the ellipse that the Processing script runs. The following process was used to do this:
                    - The signal is first run through a 'omx.comp~' object, which acts as an audio compressor, which reduces the amplitude of the peak values of the signal, and increases the amplitude of the valleys of the signal.
                    - The compressor's output is the connected to a 'gain~' object to set the gain of the signal.
                    - 'gain~' sends its signal to the 'meter~' object, which gives a visual indication of the amplitude of the signal. The 'meter~' outputs a float value between 0 and 1 that corresponds to the amplitude of the signal. This output is also sent to affect the randomly generated values in the other route (explained below).
                    - 'prepend /amp' receives this float number and formats the message with the /amp tag, thus returning: /amp float_value. The /amp tag will be used for Processing to identify this message as the amplitude of the signal.

                The second route generates random values for the RGB pixel values of the ellipse that Processing will be drawing. A new random number will be generated each time the 'bonk~' object detects a transient (broadband spectral impulses). 'bonk~' detects transients above a given velocity value, which is fed by an int preceded with the minvel, in this case via the 'minvel $1' message box. I believe 'bonk~' implements a FFT (fast fourier transformer) algorithm to extract the input's spectral data.
                Once 'bonk~' detects a transient, it outputs a list of three values out of its second outlet (cooked output):
                    - The first value corresponds to the instrument number (only relevant when the object is implemented in learn mode, which is not the case for this project).
                    - The second value is the velocity of the instrument's transient.
                    - The third value is weighted sum of the spectral analysis values of the bins (also not relevant in this project).
                
                This cooked output is sent to a 'route 0' object, which will route the message with a matching tag (0) to the left outlet, without including the 0. The 0 was assigned because 'bonk~' was not initiated in learn mode, and only has one instrument's transient loaded into memory (which is a short impulse that exceeds the minimum velocity threshold).

                Once the 0 has been filtered out, the 'unpack f f' object will separate the elements of a two-element list into two elements that are routed out of each of the 'unpack' outlets. In this case, and as per 'f f' argument, the first and second elements in the list will be float values. The first outlet of 'unpack' have a velocity value that is sent to a 'number' box. Every time there is change in the box's value, a bang will be triggered, and this bang will trigger three separate 'random' objects, each one generating a value between 0 and 255.
                Each time a number is generated, it will be multiplied (with the '*' object) to the amplitude value that was extracted by the 'meter~' object. This multiplication will scale the randomly generated values down for increasing and decreasing the intensity of each RGB pixel based on the amplitude of the signal.
                The resulting value, which will be between 0 and 255, will the be scaled with the 'scale' object into a float that ranges from 0 to 1.
                The 'line' object will create a 100ms linear ramp to smooth the variation of the pixel values. Each of the three values will be assigned a tag with the 'prepend' object:
                    - Red value: /red
                    - Green value: /green
                    - Blue value: /blue
                Processing will use this tags to assign the separate values to the corresponding pixel.

            Both the amplitude and the color values with be sent out the outlet of the patcher directly into the 'udpsend' object, which will transmit the messages to Processing via the OscP5 library. The UDP arguments represent a destination address, in this case, 127.0.0.1, which represents the computer's local address, and a transmission port, in this case, port 12000 as it is an available port. Note that any available port may be used as long as the port arguments match in both Max and Processing.


ArduinoMEGA_Main.ino
    This is the script that must be uploaded to the Arduino MEGA 2560 board.

    The script first declares all of the global constants and variables that will be used. In this case, this variables are going to be relevant to the button, the potentiometer, the joystick, and the Arduino built-in LED pin.

    The setup() function will the run only once, setting the mode for each one of the pins via the 'pinMode(pin, state)' function (the possible states are INPUT and OUTPUT). Then the initial value of the LED pin is assigned to HIGH via the 'digitalWrite(pin, value)' function (the possible states are HIGH and LOW). Then, three serial connections are initiated with the 'Serial.begin(baud_rate)' function:
        - The default serial connection is used to communicate with the computer via USB. This connection is established at a Baud rate of 19200Bd.
        - The second serial connection is used to communicate with the MIDI synthesizer via MIDI. This connection is established at a Baud rate of 31250Bd, which is the Baud rate of the MIDI protocol. This port is initiated with the 'Serial1.begin(baud_rate)' function.
        - The third serial connection is used to communicate with Processing via the serial port of the Arduino UNO. This is because the Arduino MEGA 2560 is unable to establish a second serial connection with the computer via USB, so the information is sent to another Arduino board that connects serially to the computer via USB. This serial connection is established at a Baud rate of 19200Bd. This port is initiated with the 'Serial2.begin(baud_rate)' function.

    The loop() function will iterate indefinitely, receiving and sending information from the controllers to the computer (both directly and via the Arduino UNO) and to the MIDI synthesizer. The loop() function has three parts:
        - Button: First, the button state is read into memory via the 'digitalRead(pin)' function. The series of conditions are then set to compare the current value against the previous value of the button. When the button is released, the code sends a capital letter 'A' to Max via serial. This is done with the 'Serial.println(message)' function. The 'println()' is necessary, as Max will use the ASCII line termination method (CR LF) to parse the data. Max will start and stop the sequence each time it receives an 'A' message.
        Arduino then updates the previous value variable.

        - Potentiometer: First, the potentiometer value is read via the 'analogRead(pin)'. Then a delta is calculated with the 'abs()' function (absolute value) to ensure that the normal electrical fluctuations in the system won't trigger the serial message. If Arduino detects a change in value that is greater that 10, it will send that value out to Max.
        Since the information that is being sent to Max comes only from the button and the potentiometer, there is no need to create a tag for Max to read it. In other words, Max is setup to receive the button data if the header tag matches 'A', and to receive the potentiometer data when there is no tag.
        Finally, the delay function adds a 10ms delay between each potentiometer data transfer to prevent the port from saturating.

        - Joystick: The first condition corresponds to the Joystick's switch. It uses the 'j_switch' variable to compare incoming data against the previous data. Arduino reads the joystick's state via the 'digitalRead()' function.
        The set actions for pressing the switch will be:
            - Send out a MIDI note on command to the analog synthesizer using the 'midiNote(command, pitch, velocity)' function, which is declared after the 'loop()' function (the MIDI note function is explained in more detail in the definition of the MIDI functions below).
            - Send an '/s' tag to Processing via the Arduino UNO serial connection. The function used to send data via the third serial port is: 'Serial2.println()'.
            - The code updates the previous value of 'j_switch' to prepare the switch release stage.
        The set actions for releasing the switch will be:
            - Send out a MIDI note off command to the analog synthesizer using the 'midiNote(command, pitch, velocity)' function.
            - Send an '/r' tag to Processing via the Arduino UNO serial connection.
            - The code updates the previous value of 'j_switch' to prepare the switch press stage.
        The joystick section then reads the value of the joystick's X axis into x, using the 'analogRead()' function. This value, which ranges between 0 and 1023, is then mapped (via the 'map()' function)to a range between 0 and 127, which is the range MIDI implements for controlling parameters. The mapped value is stored into 'audioX'.
        If the MIDI value for the joystick's X axis is off the range given to the center position (58 > x < 70), the 'midiCC()' function (also explained in detail below) will shift the active octave of the synthesizer's oscillator.
        Despite the positon of the X axis, serial information will be constantly sent to Processing (via the Arduino UNO by using the 'Serial2.println()' function). The code will first send a '/x' tag for Processing to recognize the value as the X coordinate, and will then send the value of the X axis (without scaling it, so that a wider range can be used in Processing).
        The code for the Y axis is exactly the same as the code for the X axis, with the exception that the MIDI information being sent out will control the waveshape of the oscillators and the overdrive of the filter in the MIDI synthesizer, and then send the value of the Y axis to Processing preceed with the tag '/y'. Note that the information sent to the filter is mapped inversely: 127 to 0, instead of 0 to 127.

    There are two additional functions declared at the bottom of the code that send MIDI data to the MIDI synthesizer via the second serial port, using the 'Serial.write()' functions. The specifics of each function are:
        - The 'midiNote()' function takes three arguments:
            - command: Standardized MIDI command to start and stop a note. The command is represented by an int, or by a hexadecimal value. The note on command is 0x90 (or 144), and the note off command is 0x80 (or 128).
            - pitch: The pitch of the note that will be triggered by the synthesizer, represented by an int between 0 and 127.
            - velocity: Intensity of the impact that is to trigger the designated MIDI note, also represented by and int between 0 and 127.
        - The 'midiCC()' function rakes three arguments:
            - command: The standardized MIDI command to transmit MIDI Continuous Change (or Continuous Controller) messages, which are used to modulate parameters of the synthesizer. The command to transfer MIDI CC is 0xB0 (or 176).
            - ccParameter: The number of the CC parameter that will have its value updated. CC parameter are more or less standardized, but it's good to find the MIDI reference chart of the MIDI synthesizer to accurately tell which parameters to control. This project controls CC parameters:
                - 74 and 75, representing the octave of the two oscillators of the Moog Sub37.
                - 9 and 14, representing the waveshape of the two oscillators.
                - 18, representing the drive of the filter.
            - ccValue: Value that will be assigned to the specified MIDI CC parameter.


ArduinoUNO_SerialThruPort.ino
    This is the script that must be uploaded to the Arduino UNO board.

    The script first includes the SoftwareSerial.h library, which will be used for establishing a serial communication between the ArduinoMEGA 2560 and the Arduino UNO.
    The script then defines the serial ports that will be used for the software serial, reading on pin 2, and writing on pin 3, on the variable mySerial.

    The setup() function then begins the softare serial connection with the Arduino MEGA 2560 with the begin method on the mySerial variable via 'mySerial.begin(19200)'. A Baud rate of 19200Bd was selected because it is the same rate declared in the third serial port of the Arduino MEGA (Serial2).
    The setup() function starts a USB serial connection with the computer via 'Serial.begin(19200)'. This port is also initiated to 19200Bd, because that is the same rate that is declared in the Processing script.

    The loop() function has a condition that checks if the software serial port is active by using the 'mySerial.available()' function. If there is an established connection, Ardino uno will pass the received serial data straight into Processing, by using the functions 'Serial.write(my.Serial.read())';

Processing_AudioGraphics.pde
    The script first imports the OscP5 and the Processing Serial libraries.

    It then declares all of the global constants and variables that will be used. Constants are defined in Processing with the 'final' keyword.

    Just like in Arduino, the 'setup()' funtion will run only once, and the code will the proceed to continuously iterate over the 'draw()' function. The Processing setup() function performs a few tasks:
        - It first declares the size of the Canvas that will be displaying the Processing graphics. This is done via the 'size(width, height)' function. Width and height are passes as numeric values, and Processing then automatically stores the value of the canvas' width into a special constant called 'width', and the value of the canvas' height into a special constant called 'height'.
        - setup() then evaluates which is the shorter axis between width and height, and stores that special constant in the variable 'shortEdge'. This value is used to scale the size of the drawn ellipse in the loop() function.
        - Set the resolution of the rendered frames to high resolution with the 'pixelDensitiy(displayDensity())' functions. 'displayDensity()' will return 2 for high resolution retina screen to duplicate the pixel density, and return 1 for standard displays to properly set the screen to the maximum available resolution.
        - The 'frameRate()' function specifies the rate at which frames will be generated/rendered.
        - The 'noCursor()' function ensures that the cursor is not visible when passing on top of the graphics created by processing.
        - Then, an new instance of oscP5 is initialized with the function 'OscP5(this, 12000)'. The argument 'this' specifies the connection to be local to the computer, and 12000 is the UDP transfer port specified for receiving OSC from Max.
        - The 'printArray(Serial.list())' functions will print the available serial ports onto the Processing terminal for selecting the port that corresponds to the Arduino UNO.
        - The next line is used to assign the corresponding port to the variable 'portName' via the function 'Serial.list()[ID]', where the ID is the port number.
        - A new Processing serial instance is initialized into 'myPort' via the function 'Serial(this, portName, 19200)'. The argument 'this' also specifies a local connection to the computer, via the USB port assigned as 'portName', at the specified Baud rate. 19200Bd is used since the information was set to connect at 19200Bd inside the Arduino UNO.
        - The 'myPort.clear()' function empties the port to ensure no lingering data is on queue in the serial port's buffer.
    
    The 'draw()' function first checks the Arduino UNO serial port to get the joystick's information. This is done in a series of steps:
        - The 'myPort.available()' method in the Processing's serial library checks if the port is connected.
        - If the port is connected, a message will be read into the 'serialMessage' variable via the 'myPort.readStringUntil()' method. The LF value for ASCII (10) is passed as an argument. This will use the ASCII linefeed value as a delimeter between messages.
        - The program then verifies that the value of 'serialMessage' is different from null.
        - If the serialMessage is not null, the code will proceed to find the tag used when transferring the message to determine which condition to execute. Since all the tags are preceeded by a forward slash, to check the actual tag, the program must evaluate the second digit in the message. This is done via the 'serialMessage.charAt(1)' method, which indexes string values in an array-type manner.
        - If the tag equals 's', the joystick switch has been pressed and the 'flip_switch' boolean variable will be updated to true.
        - If the tag equals 'r', the joystick switch has been released and the 'flip_switch' boolean variable will be updated to false.
        - The 'x' and 'y' tags on the other hand, will perform a second 'myPort.readStringUntil(lf)', again using a linefeed as a delimeter, and storing the message's value into the string 'x' or 'y' respectively. If the contents of the 'x' or 'y' strings is different from null, Processing will first use the 'trim()' function to eleminate the whitespace characters from the 'x' and 'y' strings.
        - The string 'x' will containt a number which will be parsed into an int via the java 'Integer.parseInt()' method. This int will be stored in the 'x_offset' and 'y_offset' variables. The value of 'x_offset' and 'y_offset' is then mapped to a range that better fits the Processing drawing screen, setting limits that prevent joystick movements from making the drawn ellipse go outside the Processing canvas.
    The 'draw()' function then receives the Max amplitude and color data. It uses rate of change, a smoothing factor, and a scale value to set map the amplitude to ranges that are functional in the Processing canvas. This is done via a series of simple mathematical operations. More information on the sequence of operations can be found in the Processing file.
    The colors are assigned by scaling the amplitude values (sent to Processing as a float ranging between 0 and 1) to an appropriate pixel range (0 to 255). This value is then multiplied by the random values received from Max via UDP (explained in detail below).
    Once the color and amplitude information has been parsed, the script then moves on to the drawing stage:
        - If the 'flip_switch' boolean variable (activated via the joystick switch) is false, the background will be drawn black, and an ellipse with randomly assigned values for each of its pixels will be drawn in the middle of the screen.
        - If the 'flip_switch' boolean variable is true, the colors between the background and the ellipse will flip, so that the ellipse is black, and the background changes colors.
        - Regardless of the state of the 'flip_switch' variable, the X and Y axes of the joystick will send offset values to the X and Y coordinates of the ellipse that is drawn.
    After the frames have been drawn, the saving stage of the program will run. It's good to note that the frame capture/recording will only start once the amplitude exceeds a very small value. This means that as long as there is no audio amplitude, there will be no graphics nor saving. However, once the recording has been started, the 'r' key of the keyboard can be used as a recording toggle for the frames (this function is explained in more detail below). The function used to save frames is the 'saveFrame("folder/filename####.ext")' function. Hashes are used to represent placeholder values that will correspond to the active frame of the Processing sketch. The folder containing the images will be created at the same directory where the Processing sketch is located.

    Two additional functions are declared on this script:
        The 'oscEvent' function receives an OSC message via the OscP5 library. This information is coming from max.
            - The message is first loaded into the 'oscValue' variable via: theOscMessage.get(0).floatValue();
            - The function then checks to see if the message has an '/amp', a '/red', a '/green', or a '/blue' tag, to determine that which information is carried in the message. Depending on the tag, the value stored into 'oscValue' will be assigned to the 'amplitude', 'red', 'green', and 'blue' variables. The method to check the tag is: theOscMessage.checkAddrPattern()

        The second additional function is the keyReleased() function. This function enables the 'r' key as a recording toggle once the first recording has started.


//---------- Basic Definitions ----------//

- Baud rate: Measure symbol rate (symbols per second) that determines the speed of communication over a data channel.
- MIDI (Musical Intrument Digital Interface): A technical standard that incorporates a communcation protocol, a digital interface, and electrical connectors for the communication of electronic musical instruments and computers. The protocol is primarily used for creating music with a computer, and for communication between various musical instruments.
- OSC (Open Sound Control): OSC is a protocol for communication among multimedia devices that has been optimized for networking technology, and it is widely used in the world of electronic musical instruments.
- Peripherals: An external device that provides input and output for the computer.
- Serial Communication: Communication protocol used to transfer data between data processing equipment and peripherals. Serial communication sends sequential data one bit at a time over the communication channel. It important to note that the way this bits are transmitted follow a queue-type algorithm, meaning that the first data that is sent via the port will be the first data that is read by the receiving end.
- UDP (User Datagram Protocol): Core component of the Internet Protocol (IP) suite. Applications may send messages to other IP network hosts. UDP uses a connectionless communication model that transfers data via specified port numbers.


//---------- Resources ----------//

Arduino
    - Arduino MEGA auxiliary serial ports: https://www.arduino.cc/en/Tutorial/MultiSerialMega
    - Arduino Serial reference: https://www.arduino.cc/reference/en/language/functions/communication/serial/
    - Arduino Software Serial library: https://www.arduino.cc/en/Reference/softwareSerial
    - Arduino Software Serial tutorial: https://www.youtube.com/watch?v=vcXQ79YxeGM 
    - Button tutorial: https://www.arduino.cc/en/tutorial/button
    - Joystick tutorial: https://www.brainy-bits.com/arduino-joystick-tutorial/
    - Main reference: https://www.arduino.cc/reference/en 
    - Potentiometer tutorial: https://www.arduino.cc/en/tutorial/AnalogInput 
    - Stabilize incoming Potentiometer data: https://forum.arduino.cc/index.php?topic=163149.0 

Crow
    - Running Crow from Max: https://monome.org/docs/crow/max-m4l/ 
    - See the Crow reference patches within Max for additional intructions.

Interconnectivity:
    - Arduino to Max tutorial 1: https://maker.pro/arduino/tutorial/how-to-send-and-receive-data-between-an-arduino-and-maxmsp 
    - Arduino to Max tutorial 2: https://www.youtube.com/watch?v=6bT3G4Mep7E 
    - Arduino to Processing tutorial: https://learn.sparkfun.com/tutorials/connecting-arduino-to-processing/all 
    - Max to Processing tutorial via OscP5: https://medium.com/bytes-of-bits/max-facts-using-osc-to-route-max-into-processing-7635b1dba154 
    - OscP5 library by Andreas Schlegel: http://www.sojamo.de/libraries/oscP5/

Max:
    - Bang definition: https://docs.cycling74.com/max7/tutorials/basicchapter02
    - Extract transients from audio signal in Max with bonk~ object, written by Miller Puckett for PureData, and adapted to Max by Volker Böhm: https://cycling74.com/forums/64-bit-versions-of-sigmund-fiddle-and-bonk
    - Information about the third float element of the list outputted by the cooked output in the bonk~ object: https://cycling74.com/forums/bonk-ers 
    - 'omx.comp~' object reference: https://docs.cycling74.com/max5/refpages/msp-ref/omx.comp~.html
    - Order of operations in Max: https://docs.cycling74.com/max5/tutorials/max-tut/basicchapter05.html 
    - 'serial' object reference: https://docs.cycling74.com/max7/refpages/serial 
    - 'sfrecord' object reference: https://docs.cycling74.com/max5/refpages/msp-ref/sfrecord~.html 
    - Summing stereo signals into mono: https://cycling74.com/forums/do-i-need-to-change-gain-if-merging-from-stereo-to-mono
    - 'zl' object reference: https://docs.cycling74.com/max7/refpages/zl 

MIDI and CV
    - CV and Gate signals in synthesizers: https://en.wikipedia.org/wiki/CV/gate
    - General: https://ccrma.stanford.edu/~craig/articles/linuxmidi/misc/essenmidi.html 
    - List of Continuous Change (CC) messages: https://www.midi.org/specifications-old/item/table-3-control-change-messages-data-bytes-2

Processing:
    - Declare constants: https://processing.org/reference/final.html 
    - Environment reference: https://processing.org/reference/environment/ 
    - Evaluate character in a string: https://processing.org/reference/String_charAt_.html 
    - Main reference: https://processing.org/reference/ 
    - Remove string whitespacing: https://processing.org/reference/trim_.html 
    - Round a float to an int: https://processing.org/reference/round_.html 
    - Saving tutorial: https://www.youtube.com/watch?time_continue=808&v=G2hI9XL6oyk&feature=emb_logo 
    - Save frames: https://processing.org/reference/saveFrame_.html 
    - Screen resolution: https://processing.org/reference/displayDensity_.html 
    - Serial library: https://processing.org/reference/libraries/serial/index.html 
    - Serial read string: https://processing.org/reference/libraries/serial/Serial_readStringUntil_.html

Serial communcation:
    - Basic: https://en.wikipedia.org/wiki/Serial_communication
    - Baud rate: https://en.wikipedia.org/wiki/Baud 
    - OSC: http://opensoundcontrol.org/introduction-osc 
    - TTY vs CU: https://learn.sparkfun.com/tutorials/terminal-basics/tips-and-tricks 
    - UDP: https://en.wikipedia.org/wiki/User_Datagram_Protocol 

Miscellaneous and Others:
    - ASCII chart: https://asciichart.com 
    - Info on ASCII characters CR (13) and LF (10): https://stackoverflow.com/questions/1552749/difference-between-cr-lf-lf-and-cr-line-break-types 
    - List of Moog Sub37 MIDI CC mapping (pages 53 through 55): https://api.moogmusic.com/sites/default/files/2018-09/SUB_37_MANUAL_v1.1_0.pdf 
    - Peripheral definition: https://en.wikipedia.org/wiki/Peripheral
