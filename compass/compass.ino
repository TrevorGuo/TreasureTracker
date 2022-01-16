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

#include <Adafruit_GPS.h> //Adafruit GPS Library
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <Servo.h>

Servo myservo;
float currentYaw = 0;

// what's the name of the hardware serial port?

// Connect to the GPS on the hardware port

SoftwareSerial mySerial(3, 2);
Adafruit_GPS GPS(&mySerial);

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
double currPos[2];

double locs[4][2] = {{90, 135}, {90, 135}, {90, 135}, {90, 135}};
int active = 0;


// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO  true

Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

void setup()
{
  //while (!Serial);  // uncomment to have the sketch wait until Serial is ready

  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
  Serial.begin(115200);
  Serial.println("starting");
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  pinMode(BUTTON3, INPUT);
  pinMode(BUTTON4, INPUT);

  while (!Serial) {
    delay(1); // will pause Zero, Leonardo, etc until serial console opens
  }
  setupSensor();
  Serial.println("set up");
  GPS.begin(9600);
  GPS.sendCommand("$PGCMD,33,0*6D");
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  delay(1000);
  Serial.println("Adafruit GPS logging data dump!");

  // 9600 NMEA is the default baud rate for MTK - some use 4800
}

uint32_t updateTime = 1000;
double latPoint = 0.0;
double longPoint = 0.0;

void loop()                     // run over and over again
{//  Serial.print("Deg: ");
  Serial.print("Deg: ");
  currentYaw = getYaw();
  Serial.println(currentYaw);
  myservo.write(90 - (currentYaw / 2));
  clearGPS();
  currPos[0] = getLon();
  currPos[1] = getLat();
  if(GPS.fix==1)
  {
    Serial.print(GPS.latitude);
    Serial.print(GPS.lat);
    Serial.print(GPS.longitude);
    Serial.println(GPS.lon);
    Serial.println("NEXT\n");
  }
}//loop

void handleButtons()
{
  if(digitalRead(BUTTON1))
  {
    active = 0;
    if(b1_duration-millis() > 4000)
    {
      setPosition();
    }
  }else if(digitalRead(BUTTON2))
  {
    active = 1;
    if(b2_duration-millis() > 4000)
    {
      setPosition();
    }
  }else if(digitalRead(BUTTON3))
  {
    active = 2;
    if(b3_duration-millis() > 4000)
    {
      setPosition();
    }
  }else if(digitalRead(BUTTON4))
  {
    active = 3;
    if(b4_duration-millis() > 4000)
    {
      setPosition();
    }
  }else
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
  if(GPS.fix==1)
  {
    retval = GPS.longitude;
    if(GPS.lon == 'W'){retval*=-1;};
  }
  return retval;
}

double getLat()
{
  double retval = -1;
  if(GPS.fix==1)
  {
    retval = GPS.latitude;
    if(GPS.lat == 'S'){retval*=-1;};
  }
  return retval;
}

void setPosition() {
  clearGPS();
  locs[active][0] = getLon();
  locs[active][1] = getLat();
}

//https://stackoverflow.com/questions/3932502/calculate-angle-between-two-latitude-longitude-points
//The math/code to find the bearing between two coordinates was found at the above link. 

float toRadians(float degree) {
  return degree * M_PI / 180;
}

float getBearingToWaypoint(float lat1, float long1, float lat2, float long2) {
    lat1 = toRadians(lat1);
    long1 = toRadians(long1);
    lat2 = toRadians(lat2);
    long2 = toRadians(long2);

    float dLon = (long2 - long1);

    float y = sin(dLon) * cos(lat2);
    float x = cos(lat1) * sin(lat2) - sin(lat1)
            * cos(lat2) * cos(dLon);

    float brng = atan2(y, x);

    return brng / M_PI * 180;
}

void savePoint(double lat, double lon) {
    latPoint = lat;
    longPoint = lon;
}

void readGPS()
{
  clearGPS();
  while(!GPS.newNMEAreceived())
  {
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA());
  NMEA1=GPS.lastNMEA();
  
  while(!GPS.newNMEAreceived())
  {
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA());
  NMEA2=GPS.lastNMEA();
  Serial.println(NMEA1);
  Serial.println(NMEA2);
  Serial.println("--");
}

void clearGPS() //clear old data from serial port
{
  while(!GPS.newNMEAreceived())
  {
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA());
  while(!GPS.newNMEAreceived())
  {
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA());
  while(!GPS.newNMEAreceived())
  {
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA());
}
