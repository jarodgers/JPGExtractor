# JPGExtractor

Compile instructions:

Upload the Arduino code to an Arduino, hooking the camera's serial wires to ports 2 and 3, as stated in the code. Using Windows or Linux, compile request_picture.c and rs232.c. Run the resulting file with a command line argument for the serial port number that the Arduino is using.

There are functions in the Arduino code for changing the picture quality, baud rate, and things like this. If using these, call the functions in the setup and upload the code to the Arduino again. Then remove those function calls, and upload the code again. These settings are maintained once they are changed.
