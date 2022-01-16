// Test code for Adafruit GPS modules using MTK3329/MTK3339 driver
//
// This code turns on the LOCUS built-in datalogger. The datalogger
// turns off when power is lost, so you MUST turn it on every time
// you want to use it!
//
// Tested and works great with the Adafruit GPS FeatherWing
// ------> https://www.adafruit.com/products/3133
// or Flora GPS
// ------> https://www.adafruit.com/products/1059
// but also works with the shield, breakout
// ------> https://www.adafruit.com/products/1272
// ------> https://www.adafruit.com/products/746
//
// Pick one up today at the Adafruit electronics shop
// and help support open source hardware & software! -ada

#define LED 2
#include <Adafruit_GPS.h> //Adafruit GPS Library

#include <Wire.h>
#include <SPI.h>
#include <SparkFunLSM9DS1.h>
//#include <Adafruit_LSM9DS1.h>
//#include <Adafruit_Sensor.h>
//#include <SoftwareSerial.h>
#include <math.h>

#include <Servo.h>
Servo myservo;
float currentYaw = 0;

// what's the name of the hardware serial port?

// Connect to the GPS on the hardware port

//SoftwareSerial mySerial(3, 2);
Adafruit_GPS GPS(&Serial2);

String NMEA1;
String NMEA2;
char c;
#define BUTTON1 4
#define BUTTON2 5
#define BUTTON3 6
#define BUTTON4 7
int b1_duration = 0;
int b2_duration = 0;
int b3_duration = 0;
int b4_duration = 0;
double loc1[] = { -1, -1};
double loc2[] = { -1, -1};
double loc3[] = { -1, -1};
double loc4[] = { -1, -1};

int active = -1;


// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO  true


void setup()
{
  Serial.begin(115200);
//  pinMode(LED, OUTPUT);
//  digitalWrite(LED, HIGH);
//  delay(100);
//  digitalWrite(LED, LOW);
//  delay(2000);
  //while (!Serial);  // uncomment to have the sketch wait until Serial is ready

  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
  //Serial2.begin(9600);
  Serial.println("starting");
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  pinMode(BUTTON3, INPUT);
  pinMode(BUTTON4, INPUT);

  
  Serial.println("After button decs");
  setup9DOF();
  Serial.println("After sensor set up");
  //  while (!Serial) {
  //    delay(1); // will pause Zero, Leonardo, etc until serial console opens
  //  }
  ////    //Serial.println("LSM9DS1 data read demo");
  ////
  ////  // Try to initialise and warn if we couldn't detect the chip
  //  if (!lsm.begin())
  //  {
  //    //Serial.println("Oops ... unable to initialize the LSM9DS1. Check your wiring!");
  //    while (1);
  //  }
  ////  //Serial.println("Found LSM9DS1 9DOF");
  //  setupSensor();
  //  //Serial.println("set up");
  GPS.begin(9600);
  Serial.println("GPS baud begun");
  GPS.sendCommand("$PGCMD,33,0*6D");
  Serial.println("after sent commands 1");

  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  Serial.println("after sent commands 2");

  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  Serial.println("after sent commands 3");

  delay(1000);
  clearGPS();
  //Serial.println("Adafruit GPS logging data dump!");

  // 9600 NMEA is the default baud rate for MTK - some use 4800


  myservo.attach(12);
}

uint32_t updateTime = 1000;
double latPoint = 0.0;
double longPoint = 0.0;

void loop()                     // run over and over again
{
//  clearGPS();
//  if (GPS.newNMEAreceived())
//  {
//    digitalWrite(LED, HIGH);
//    delay(1000);
//    digitalWrite(LED, LOW);
//    delay(1000);
//  }
  readGPS();

  //handleButtons();

  //Serial.print("Fix: ");
  //Serial.println(GPS.fix);

  //Serial.print("1: ");
  //Serial.print(loc1[0]);
  //Serial.print(", ");
  //Serial.println(loc1[1]);

  //Serial.print("2: ");
  //Serial.print(loc2[0]);
  //Serial.print(", ");
  //Serial.println(loc2[1]);

  //Serial.print("3: ");
  //Serial.print(loc3[0]);
  //Serial.print(", ");
  //Serial.println(loc3[1]);

  //Serial.print("4: ");
  //Serial.print(loc4[0]);
  //Serial.print(", ");
  //Serial.println(loc4[1]);

  //Serial.print("a: ");
  //Serial.println(active);

  //delay(1000);
  currentYaw = getYaw();
  //Serial.println(currentYaw);
  myservo.write(90 - (currentYaw / 2));

  //myservo.write(90 - (currentYaw + getBearingToWayPoint(lat1,long1,lat2,long2))/2);
}//loop

void handleButtons()
{
  if (digitalRead(BUTTON1))
  {
    active = 1;
    if (b1_duration - millis() > 4000)
    {
      clearGPS();
      loc1[0] = getLon();
      loc1[1] = getLat();
    }
  } else if (digitalRead(BUTTON2))
  {
    active = 2;
    if (b2_duration - millis() > 4000)
    {
      clearGPS();
      loc2[0] = getLon();
      loc2[1] = getLat();
    }
  } else if (digitalRead(BUTTON3))
  {
    active = 3;
    if (b3_duration - millis() > 4000)
    {
      clearGPS();
      loc3[0] = getLon();
      loc3[1] = getLat();
    }
  } else if (digitalRead(BUTTON4))
  {
    active = 4;
    if (b4_duration - millis() > 4000)
    {
      clearGPS();
      loc4[0] = getLon();
      loc4[1] = getLat();
    }
  } else
  {
    b1_duration = millis();
    b2_duration = millis();
    b3_duration = millis();
    b4_duration = millis();
  }
}

double getLon()
{
  double retval = -1;

  if (GPS.fix == 1)
  {
    retval = GPS.longitude;
    if (GPS.lon == 'W') {
      retval *= -1;
    };
  }

  return retval;
}

double getLat()
{
  double retval = -1;

  if (GPS.fix == 1)
  {
    retval = GPS.latitude;
    if (GPS.lat == 'S') {
      retval *= -1;
    };
  }

  return retval;
}

//https://stackoverflow.com/questions/3932502/calculate-angle-between-two-latitude-longitude-points
//The math/code to find the bearing between two coordinates was found at the above link.
float getBearingToWaypoint(double lat1, double long1, double lat2, double long2) {
  float dLon = (long2 - long1);

  float y = sin(dLon) * cos(lat2);
  float x = cos(lat1) * sin(lat2) - sin(lat1)
            * cos(lat2) * cos(dLon);

  float brng = atan2(y, x);

  brng = brng / M_PI * 180;
  brng = fmod((brng + 360), 360);
  brng = 360 - brng; //This line might not be needed?

  return brng;
}

void savePoint(double lat, double lon) {
  latPoint = lat;
  longPoint = lon;
}

double changeInDegree(double oldBrng, double newBrng) { //Positive rotates CW, negative rotates CCW
  /* Continuous Circuit
    double degreeDelta = newBrng - oldBrng;
    if (degreeDelta < 180 && degreeDelta >= 0) //newBrng > oldBrng, and shortest rotation is CW
      return degreeDelta;
    else if (degreeDelta > 180 && degreeDelta < 360) //newBrng > oldBrng, and shortest rotation is CCW
      return degreeDelta - 360;
    else if (degreeDelta > -180 && degreeDelta < 0) //newBrng < oldBrng, and shortest rotation is CCW
      return degreeDelta;
    else //newBrng < oldBrng, and shortest rotation is CW
      return 360 + degreeDelta;
  */
  //N is 90, S is 0 and 180
  double oldDegree = 90 - (oldBrng / 2);
  double newDegree = 90 - (newBrng / 2);

  return newDegree - oldDegree;
}

void readGPS()
{
  Serial.println("begining run");
  clearGPS();
  Serial.println("after clear");
  while (!GPS.newNMEAreceived())
  {
    c = GPS.read();
  }
  digitalWrite(LED, HIGH);
  GPS.parse(GPS.lastNMEA());
  NMEA1 = GPS.lastNMEA();

  while (!GPS.newNMEAreceived())
  {
    c = GPS.read();
  }
  GPS.parse(GPS.lastNMEA());
  NMEA2 = GPS.lastNMEA();

  Serial.print(NMEA1);
  Serial.print(NMEA2);
  Serial.println("--");
}

void clearGPS() //clear old data from serial port
{
  Serial.println("in Clear");
  //  while(!GPS.newNMEAreceived())
  //  {
  //    c=GPS.read();
  //  }
  GPS.parse(GPS.lastNMEA());
  Serial.println("after one parse");
  //  while(!GPS.newNMEAreceived())
  //  {
  //    c=GPS.read();
  //  }
  GPS.parse(GPS.lastNMEA());
  //  while(!GPS.newNMEAreceived())
  //  {
  //    c=GPS.read();
  //  }
  GPS.parse(GPS.lastNMEA());
}
