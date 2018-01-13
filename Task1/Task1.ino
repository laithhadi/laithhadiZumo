/*------------------------------------------------------------------------
  external libraries
  -------------------------------------------------------------------------*/
#include <ZumoMotors.h>
#include <ZumoBuzzer.h>
#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <Pushbutton.h>
#include <NewPing.h>
/*------------------------------------------------------------------------
  global variable declarations
  -------------------------------------------------------------------------*/
#define NUM_SENSORS 6
#define REVERSE_SPEED     150
#define TURN_SPEED        150
#define FORWARD_SPEED     100
#define REVERSE_DURATION  150 // ms
#define TURN_DURATION     150 // ms
#define MAX_DISTANCE      200 // Maximum distance we want to ping for (in centimeters)
#define TRIGGER_PIN        2  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN           6  // Arduino pin tied to echo pin on the ultrasonic sensor.
ZumoMotors motors;
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
ZumoReflectanceSensorArray sensors;
ZumoBuzzer buzzer;
int calibratedValue[6];
unsigned int sensorValues[NUM_SENSORS]; //declare number of sensors on the zumo
bool start = false;
bool roomScan = false;
char input;
/*------------------------------------------------------------------------
  Setup function
  -------------------------------------------------------------------------*/
void setup()
{
  Pushbutton button(ZUMO_BUTTON);

  Serial.begin(9600);     //begin serial communication at 9600 bits
  buzzer.play(">g32>>c32");  // Play a little welcome song
  sensors.init();            //Initialize the reflectance sensors module
  char input = (char) Serial.read();
  while (input != 't')
  {
    input = (char) Serial.read();
  }
  calibrateZumo();           //calibrate the zumo to the required environment

  for (int i = 0; i < NUM_SENSORS; i++)
  {
    calibratedValue[i] = sensors.calibratedMaximumOn[i];
  }
  Serial.print("Calibration completed!");
}
/*------------------------------------------------------------------------
  Loop function
  -------------------------------------------------------------------------*/
void loop()
{
  receiveInput();
  if (start)
  {
    detectWall();
  }
}
/*------------------------------------------------------------------------
  Supporting function
  -------------------------------------------------------------------------*/
void calibrateZumo()
{
  // Turn on LED to indicate we are in calibration mode
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  // Wait 1 second and then begin automatic sensor calibration
  delay(1000);

  //loop used for rotating the zumo in place to sweep the sensors over the line
  for (int i = 0; i < 80; i++)
  {
    if ((i > 10 && i <= 30) || (i > 50 && i <= 70))
      motors.setSpeeds(-200, 200);
    else
      motors.setSpeeds(200, -200);
    sensors.calibrate();

    // Since our counter runs to 80, the total delay will be
    // 80*20 = 1600 ms.
    delay(20);
  }
  motors.setSpeeds(0, 0);   //stop it from moving

  // Turn off LED and play buzzer to indicate we are through with calibration
  digitalWrite(13, LOW);
  buzzer.play(">g32>>c32");
}
//recieve input from GUI
void receiveInput()
{
  while (Serial.available() > 0)
  {
    start = true;
    input = (char) Serial.read();
    if (input == 'x')
    {
      motors.setSpeeds(0, 0);
    }
    else if (input == 'c')
    {
      if (roomScan)
      {
        scanRoom();        
      }
      else
      {
      motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
      Serial.print("Resuming automatic zumo control!")
      }
    }
    else if (input == 'p')
    {
      motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    }
    else if (input == 'z')
    {
      motors.setSpeeds(0, 0);
      Serial.print("Please show me the direction of the room!");
      char input = (char) Serial.read();
      while (!(input == 'l' || input == 'r'))
      {
        input = (char) Serial.read();
      }
      if (input == 'l')
      {
        Serial.print("Room is on the left!");
      }
      else
      {
        Serial.print("Room is on the right!");
      }
      roomScan = true;
      moveZumo();
    }
  }
}
void moveZumo()
{
  //check to see if GUI is sending any commands
  //for a specific direction
  while (input != 'c')
  {
    input = (char) Serial.read();
    if (input == 'w')
    {
      motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
      delay(REVERSE_DURATION);
      motors.setSpeeds(0, 0);
    }
    else if (input == 's')
    {
      motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
      delay(REVERSE_DURATION);
      motors.setSpeeds(0, 0);
    }
    else if (input == 'r')
    {
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      delay(TURN_DURATION);
      motors.setSpeeds(0, 0);
    }
    else if (input == 'l')
    {
      motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
      delay(TURN_DURATION);
      motors.setSpeeds(0, 0);
    }
  }
}

  //used to detect wall/corner and set behaviour
  void detectWall()
  {
    sensors.read(sensorValues);   //read the raw values from sensors
    if (checkCorner())
    {
      Serial.print("Corner ahead. Manual mode activated!");    //Display a message showing a corner has been found
      moveZumo();
    }
    else
    {
      if (sensorValues[0] >= calibratedValue[0])
      {
        motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
        delay(REVERSE_DURATION);
        motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
        delay(TURN_DURATION);
        motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
      }
      if (sensorValues[5] >=  calibratedValue[5])
      {
        motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
        delay(REVERSE_DURATION);
        motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
        delay(TURN_DURATION);
        motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
      }
    }
  }
  bool checkCorner()
  {
    if (sensorValues[0] >= calibratedValue[0] || sensorValues[5] >= calibratedValue[5])
    {
      delay(30);
      sensors.read(sensorValues);
      delay(5);
      if ((sensorValues[1] >= calibratedValue[1]) || (sensorValues[4] >= calibratedValue[4])
          || (sensorValues[3] >= calibratedValue[3]) || (sensorValues[2] >= calibratedValue[2]))
      {
        motors.setSpeeds(0, 0);
        motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
        delay(REVERSE_DURATION);
        motors.setSpeeds(0, 0);
        buzzer.playNote(NOTE_A(5), 200, 15);
        return true;
      }
      return false;
    }
  }
  void scanRoom()
  {
    
  }

