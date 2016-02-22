/**************************************
request_picture.c
by Jacob Rodgers
02-21-2016

Program that retrieves jpg picture data via RS232 serial.

Takes one command line argument, which is the serial port number.
The program will attempt to connect to the serial port, and on success,
will ask for the name of an image file to save.

Upon acceptance of image file name, program will send a 'T' character
to the serial port, which will signal the Arduino to take a picture with
the camera. When the picture data is ready to transmit, an 'A' character
will come back. Then, this program will read a chunk of data the size of
CHUNK_LENGTH (probably 32 bytes), and then send an 'R' back to the Arduino,
where it will begin readying the next chunk of data. This cycle continues
until the jpg stop flag (ff d9) is encountered. At this point, the new
image is saved and ready. Then the program will ask for another file name,
and further pictures may be taken, until "exit" is typed.

**************************************/

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
typedef unsigned char int8_t;
#else
#include <unistd.h>
#endif

#include "rs232.h"

#define CHUNK_LENGTH 32
int com_port;

/* get a chunk of a jpg file from serial and write it to jpg file
 * arguments:
 *		com_port: the selected com port
 * 		byte_array: array that will store bytes retrieved from serial
 *		len: length of byte_array
 *		f: file that chunks will be written to
 *		previous: pointer to the last byte of the previous chunk, in case it was the first half of a stop flag
 * return 1 (true) if there are still more bytes available
 * return 0 (false) if the end of the file was in this chunk (ff d9 sequence was found)*/
int getChunk(int com_port, int8_t *byte_array, int len, FILE *f, int8_t *previous);

/* pass in command line argument of com port number */
int main(int argc, char *argv[]) {
	if (argc == 2) {
		#ifdef _WIN32
		com_port = atoi(argv[1]) - 1; // for windows, port 0 maps to COM1
		#else
		com_port = atoi(argv[1]);
		#endif
		
		// connect to com port, exit if error
		if (RS232_OpenComport(com_port, 19200, "8N1") == 1) {
			printf("Could not open selected com port\n");
			return 1;
		}
		
		char filename[25];
		printf("Enter filename for new image, or type 'exit' to exit\n(picture will be taken upon submission of filename):\n");
		scanf("%s",filename);
		
		while (strcmp("exit",filename)) {
			FILE *f;
			if (!(f = fopen(filename,"wb"))) {
				printf("Could not write image file.\n");
			}
			else {
				RS232_flushRXTX(com_port);

				// send command to take picture, wait for a command to be sent back saying the picture is ready to be read
				RS232_SendByte(com_port, 'T'); // Send a T to the serial port to command it to take a picture
				
				int8_t ack = 0;
				while (!RS232_PollComport(com_port, &ack, 1)) {
					#ifdef _WIN32
					Sleep(50);
					#else
					usleep(50000);
					#endif
				}
				
				int8_t byte_array[CHUNK_LENGTH];
				int8_t previous = 0; 	// stores the last byte in the chunk retrieved from serial port,
										// in case first byte of jpg stop flag was at the end of the previous chunk,
										// and the second byte is at the start of the next chunk
				while (getChunk(com_port, byte_array, CHUNK_LENGTH, f, &previous))
					; // keep getting chunks of data from serial and writing them to the image file until all data has been read
				fclose(f); // close file
			}
			
			printf("Enter filename for new image, or type 'exit' to exit\n(picture will be taken upon submission of filename):\n");
			scanf("%s",filename);
		}
	}

	return 0;
}

int getChunk(int com_port, int8_t *byte_array, int len, FILE *f, int8_t *previous) {
#ifdef _WIN32
	Sleep(65);
#else
	usleep(65000);
#endif

	int num_bytes = 0;
	int eof_flag = 0; // set if end of the jpg file is reached (ff d9 sequence encountered)
	// get the bytes from serial
	num_bytes = RS232_PollComport(com_port, byte_array, CHUNK_LENGTH);
	RS232_SendByte(com_port, 'R'); // tell com port that all data has been received

	// examine set of bytes, looking for jpg stop flag, starting by checking the last byte of previous chunk and first byte of this chunk
	if (num_bytes > 0 && *previous == (int8_t)0xff && byte_array[0] == (int8_t)0xd9) {
		eof_flag = 1;
		num_bytes = 1; // to guarantee only the d9 byte gets written to the file
	}
	else {
		int i;
		for (i = 1; i < num_bytes; i++) {
			if (byte_array[i - 1] == (int8_t)0xff && byte_array[i] == (int8_t)0xd9) {
				eof_flag = 1; // stop flag was encountered
				num_bytes = i + 1; // guarantee that only the bytes up to and including the stop flag will be written to the file
				break;
			}
		}
	}

	*previous = byte_array[num_bytes - 1]; // the last value in this chunk should be stored in case it's the first byte of a stop flag (because the first byte of next chunk could be the second half of the stop flag)

	// now write the bytes to the file
	fwrite(byte_array, sizeof(int8_t), num_bytes, f);

	printf("%d\n",num_bytes);
	
	if (eof_flag)
		return 0;
	else {
		RS232_flushRXTX(com_port);
		return 1;
	}
}