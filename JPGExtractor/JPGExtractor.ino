/******************************************************************************
LinkSprite_Cam
Ryan Owens @ SparkFun Electronics>
Revised by Joel Bartlett on 03/25/15 for compilation on Arduino 1.6+

This code allows you to control the LinkSprite IR Camera (SEN-11610) with an Arduino microcontroller

Development environment specifics:
Arduino 1.6.0

This code is beerware; if you see me (or any other SparkFun employee) at the local, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.

----
A few program flow modifications by Jacob Rodgers
*********************************************************************************/
#include <SoftwareSerial.h>

/* Linksprite */
byte incomingbyte;
SoftwareSerial mySerial(2, 3); // RX, TX         //Configure pin 4 and 5 as soft serial port

int addr=0x0000,j=0,k=0,count=0;                    //Read Starting address
uint8_t MH,ML;
boolean EndFlag;

void SendResetCmd();
void SendTakePhotoCmd();
void SendReadDataCmd();
void StopTakePhotoCmd();

void setup()
{
    Serial.begin(19200);
    mySerial.begin(38400);
    pinMode(12,OUTPUT);
    pinMode(11,OUTPUT);
    //ChangeBaudRate();
    SendResetCmd();
    delay(3500);  //After reset, wait 2-3 second to send take picture command
    //ChangeSizeMedium();
    ChangeCompressionRatio();
}

void loop()
{
    while (1) {
      digitalWrite(12,LOW);
        while (Serial.available() == 0)
          delay(10);
        
        if (Serial.read() == 'T') {  // Take photo
            digitalWrite(12,HIGH);
            digitalWrite(11,LOW);
            SendTakePhotoCmd();
            delay(50);
            while(mySerial.available()>0)
            {
                incomingbyte=mySerial.read();
            }
            byte a[32];
            Serial.write('A'); // send acknowledgement of request to take photo
            EndFlag = 0;
            while(!EndFlag)
            {
                j=0;
                k=0;
                count=0;
                SendReadDataCmd();
                
                delay(25);
                
                while(mySerial.available()>0)
                {
                    incomingbyte=mySerial.read();
                    k++;
                    if((k>5)&&(j<32)&&(!EndFlag))
                    {
                        a[j]=incomingbyte;
                        if((a[j-1]==0xFF)&&(a[j]==0xD9))  {    //Check if the picture is over
                          EndFlag=1;
                          addr = 0x0000;
                          digitalWrite(11,HIGH);
                        }
                        j++;
                        count++;
                    }
                }
                Serial.write(a,count);

                // wait for acknowledgement before continuing
                while (Serial.available() == 0)
                    delay(5);

                if (Serial.read() != 'R') {
                    break;
                }
            } // end while(!EndFlag)
            StopTakePhotoCmd();
        } // end big if
    } // end while(1)
}

//Send Reset command
void SendResetCmd()
{
    mySerial.write(0x56);
    mySerial.write((byte)0);
    mySerial.write(0x26);
    mySerial.write((byte)0);
}

//Send take picture command
void SendTakePhotoCmd()
{
    mySerial.write(0x56);
    mySerial.write((byte)0);
    mySerial.write(0x36);
    mySerial.write(0x01);
    mySerial.write((byte)0);
}

//Read data
void SendReadDataCmd()
{
    MH=addr/0x100;
    ML=addr%0x100;
    mySerial.write(0x56);
    mySerial.write((byte)0);
    mySerial.write(0x32);
    mySerial.write(0x0c);
    mySerial.write((byte)0);
    mySerial.write(0x0a);
    mySerial.write((byte)0);
    mySerial.write((byte)0);
    mySerial.write(MH);
    mySerial.write(ML);
    mySerial.write((byte)0);
    mySerial.write((byte)0);
    mySerial.write((byte)0);
    mySerial.write(0x20);
    mySerial.write((byte)0);
    mySerial.write(0x0a);
    addr+=0x20;                            //address increases 32£¬set according to buffer size
}

void StopTakePhotoCmd()
{
    mySerial.write(0x56);
    mySerial.write((byte)0);
    mySerial.write(0x36);
    mySerial.write(0x01);
    mySerial.write(0x03);
}

void ChangeCompressionRatio() {
    mySerial.write((byte)0x56);
    mySerial.write((byte)0x00);
    mySerial.write((byte)0x31);
    mySerial.write((byte)0x05);
    mySerial.write((byte)0x01);
    mySerial.write((byte)0x01);
    mySerial.write((byte)0x12);
    mySerial.write((byte)0x04);
    mySerial.write((byte)0x00); // default 36
}

void ChangeBaudRate() {
    mySerial.write((byte)0x56);
    mySerial.write((byte)0x00);
    mySerial.write((byte)0x24);
    mySerial.write((byte)0x03);
    mySerial.write((byte)0x01);
    mySerial.write((byte)0x2a); // last two bytes:
    mySerial.write((byte)0xf2); // 9600: ae c8
                                // 19200: 56 e4
                                // 38400: 2a f2
}

void ChangeSizeSmall() {
    mySerial.write((byte)0x56);
    mySerial.write((byte)0x00);
    mySerial.write((byte)0x31);
    mySerial.write((byte)0x05);
    mySerial.write((byte)0x04);
    mySerial.write((byte)0x01);
    mySerial.write((byte)0x00);
    mySerial.write((byte)0x19);
    mySerial.write((byte)0x22);      
}

void ChangeSizeMedium()
{
    mySerial.write((byte)0x56);
    mySerial.write((byte)0x00);
    mySerial.write((byte)0x31);
    mySerial.write((byte)0x05);
    mySerial.write((byte)0x04);
    mySerial.write((byte)0x01);
    mySerial.write((byte)0x00);
    mySerial.write((byte)0x19);
    mySerial.write((byte)0x11);      
}

void ChangeSizeBig()
{
    mySerial.write((byte)0x56);
    mySerial.write((byte)0x00);
    mySerial.write((byte)0x31);
    mySerial.write((byte)0x05);
    mySerial.write((byte)0x04);
    mySerial.write((byte)0x01);
    mySerial.write((byte)0x00);
    mySerial.write((byte)0x19);
    mySerial.write((byte)0x00);      
}
