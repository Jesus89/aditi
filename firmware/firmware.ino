/*
 * Firmware for Aditi Project
 *
 * Date: May 2015
 * Author: Jesus Arroyo Torrens <jesus.arroyo@bq.com>
 *
 */


#include "I2C.h"
#include <Servo.h>

int r;
int phi;
int theta;

int phiInc = 1;
int phiMin = 0;
int phiMax = 180;
int thetaInc = 1;
int thetaMin = 0;
int thetaMax = 180;

Servo servoPhi;
Servo servoTheta;

bool scanning = false;

//unsigned long time;


void setup()
{
  // Init Serial
  Serial.begin(115200);

  // Init I2C
  I2c.begin();
  delay(100);
  I2c.timeOut(50);

  //// Reset device to defaults
  llWriteAndWait(0x00,0x00);
  delay(100);

  // Setup Measurement Period
  llWriteAndWait(0x68, 0x14);
  delay(100);
}


void loop()
{
  // Read serial to start
  if(Serial.available() > 0)
    scanning = Serial.read() == 's';

  if (scanning)
  {
    // Init Servo Motors
    servoPhi.attach(9, 430, 2230);
    servoTheta.attach(10, 430, 2230);
    scan();
  }
  else
  {
    // Disable Servo Motors
    servoPhi.detach();
    servoTheta.detach();
  }
}

int _phiMin(bool dir)
{
  return dir ? phiMin : phiMax;
}

bool _phiCond(bool dir)
{
  return dir ? phi <= phiMax : phi >= phiMin;
}

int _phiInc(bool dir)
{
  return dir ? phiInc : -phiInc;
}

int _readable(int theta, int phi)
{
  int inc = round(phiMax/(((theta < 90) ? theta : 180 - theta) + 1));
  return phi % inc != 0;
}

void scan()
{
  bool dir;

  // Initialize position
  servoPhi.write(0);
  servoTheta.write(0);
  delay(1000);

  for(theta = thetaMin; theta <= thetaMax; theta += thetaInc)
  {
    servoTheta.write(theta);

    dir = theta % 2 == 0;

    for(phi = _phiMin(dir); _phiCond(dir); phi += _phiInc(dir))
    {
      if (_readable(theta,phi))
      {
        servoPhi.write(phi);

        //time = millis();

        // Read distance
        r = llGetDistance(); // llGetDistanceAverage(2);

        //Serial.println(millis()-time);

        // Send data
        Serial.print(theta);
        Serial.print(",");
        Serial.print(phi);
        Serial.print(",");
        Serial.println(r);
      }

      // Read serial
      if (Serial.available() > 0)
        if (Serial.read() == 'q')
        {
          scanning = false;
          return;
        }
    }
  }

  scanning = false;
}


/* ==========================================================================================================================================
Basic read and write functions for LIDAR-Lite, waits for success message (0 or ACK) before proceeding
=============================================================================================================================================*/

// Write a register and wait until it responds with success
void llWriteAndWait(char myAddress, char myValue){
  uint8_t nackack = 100; // Setup variable to hold ACK/NACK resopnses
  while (nackack != 0){ // While NACK keep going (i.e. continue polling until sucess message (ACK) is received )
    nackack = I2c.write(0x62,myAddress, myValue); // Write to LIDAR-Lite Address with Value
    //delay(2); // Wait 2 ms to prevent overpolling
  }
}

// Read 1-2 bytes from a register and wait until it responds with sucess
byte llReadAndWait(char myAddress, int numOfBytes, byte arrayToSave[2]){
  uint8_t nackack = 100; // Setup variable to hold ACK/NACK resopnses
  while (nackack != 0){ // While NACK keep going (i.e. continue polling until sucess message (ACK) is received )
    delay(2); // Wait 2 ms to prevent overpolling
    nackack = I2c.read(0x62,myAddress, numOfBytes, arrayToSave); // Read 1-2 Bytes from LIDAR-Lite Address and store in array
  }
  return arrayToSave[2]; // Return array for use in other functions
}

/* ==========================================================================================================================================
Get 2-byte distance from sensor and combine into single 16-bit int
=============================================================================================================================================*/

int llGetDistance(){
  llWriteAndWait(0x00,0x04); // Write 0x04 to register 0x00 to start getting distance readings
  byte myArray[2]; // array to store bytes from read function
  llReadAndWait(0x8f,2,myArray); // Read 2 bytes from 0x8f
  int distance = (myArray[0] << 8) + myArray[1];  // Shift high byte [0] 8 to the left and add low byte [1] to create 16-bit int
  return(distance);
}

/* ==========================================================================================================================================
Get raw velocity readings from sensor and convert to signed int
=============================================================================================================================================*/

int llGetVelocity(){
  llWriteAndWait(0x00,0x04); // Write 0x04 to register 0x00 to start getting distance readings
  llWriteAndWait(0x04,0x80); // Write 0x80 to 0x04 to switch on velocity mode
  byte myArray[1]; // Array to store bytes from read function
  llReadAndWait(0x09,1,myArray); // Read 1 byte from register 0x09 to get velocity measurement
  return((int)((char)myArray[0])); // Convert 1 byte to char and then to int to get signed int value for velocity measurement
}

/* ==========================================================================================================================================
Average readings from velocity and distance
int numberOfReadings - the number of readings you want to average (0-9 are possible, 2-9 are reccomended)
=============================================================================================================================================*/

int llGetDistanceAverage(int numberOfReadings){
  if(numberOfReadings < 2){
    numberOfReadings = 2; // If the number of readings to be taken is less than 2, default to 2 readings
  }
  int sum = 0; // Variable to store sum
  for(int i = 0; i < numberOfReadings; i++){
      sum = sum + llGetDistance(); // Add up all of the readings
  }
  sum = sum/numberOfReadings; // Divide the total by the number of readings to get the average
  return(sum);
}
