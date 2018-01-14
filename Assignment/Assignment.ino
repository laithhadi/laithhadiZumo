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
#define REVERSE_DURATION  150 // ms
#define TURN_DURATION     150 // ms
#define MAX_DISTANCE      30  // Maximum distance we want to ping for (in centimeters)
#define TRIGGER_PIN        2  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN           6  // Arduino pin tied to echo pin on the ultrasonic sensor.
ZumoMotors motors;
ZumoReflectanceSensorArray sensors;
ZumoBuzzer buzzer;
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
int calibratedValue[6];
String rooms[10];
String message;
int FORWARD_SPEED = 130;
int roomCounter = 0;
int corridorCounter = 0;
unsigned int sensorValues[NUM_SENSORS]; //declare number of sensors on the zumo
bool start = false;
bool roomScan = false;
bool objectFound;
char input, previousInput;
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
  Serial.print("Zumo is at corridor number 1!");
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
  while (Serial.available() > 0 && previousInput != 'c')
  {
    start = true;    
    input = (char) Serial.read();
    
    if (input == 'x')
    {
      motors.setSpeeds(0, 0);
    }
    else if (input == 'j')
    {
      int motorSpeed = Serial.parseInt();
      FORWARD_SPEED = motorSpeed; 
      Serial.print(motorSpeed);     
    }
    else if (input == 'p')
    {
      motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    }
    else if (input == 'z')
    {
      motors.setSpeeds(0, 0);
      Serial.print("Please show me the direction of the room!");
      input = (char) Serial.read();
      while (!(input == 'l' || input == 'r'))
      {
        input = (char) Serial.read();
      }
      if (input == 'l')
      {
        rooms[roomCounter] = "left";
        motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
        delay(TURN_DURATION);
        motors.setSpeeds(0, 0);
      }
      else
      {
        rooms[roomCounter] = "right";
        motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
        delay(TURN_DURATION);
        motors.setSpeeds(0, 0);
      }
      message = "Room: ";
      Serial.print(message + (roomCounter + 1));
      message = " on the " + rooms[roomCounter] + " side of corridor ";
      Serial.print(message +(corridorCounter + 1));
      Serial.print("!");
      roomScan = true;
      roomCounter++;
      moveZumo();
    }
  }
  if (previousInput == 'c')
  {
    if (roomScan)
    {
      scanRoom();
    }
    motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    Serial.print("Automatic zumo control!");
    previousInput = ' ';
  }
}
void moveZumo()
{
  char zumoDirection = ' ';
  //check to see if GUI is sending any commands
  //for a specific direction
  while (zumoDirection != 'c')
  {
    zumoDirection = (char) Serial.read();
    if (zumoDirection == 'w')
    {
      motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
      delay(REVERSE_DURATION);
      motors.setSpeeds(0, 0);
    }
    else if (zumoDirection == 's')
    {
      motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
      delay(REVERSE_DURATION);
      motors.setSpeeds(0, 0);
    }
    else if (zumoDirection == 'r')
    {
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      delay(TURN_DURATION);
      motors.setSpeeds(0, 0);
    }
    else if (zumoDirection == 'l')
    {
      motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
      delay(TURN_DURATION);
      motors.setSpeeds(0, 0);
    }
  }
  previousInput = 'c';
}

//used to detect wall/corner and set behaviour
void detectWall()
{
  sensors.read(sensorValues);   //read the raw values from sensors
  if (!checkCorner())
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
      Serial.print("Corner ahead. Manual mode activated!");    //Display a message showing a corner has been found
      corridorCounter++;
      message = "Zumo is at corridor number ";
      Serial.print(message + (corridorCounter + 1));
      Serial.print("!");
      moveZumo();
      return true;
    }
    return false;
  }
}
void scanRoom()
{
  objectFound = false;
  motors.setSpeeds(200, 200);
  delay(500);

  for (int i = 0; i < 4 && objectFound == false; i++)
  {
    if (i == 0)
    {
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      delay(700);
      motors.setSpeeds(0, 0);
      delay(30);
      if (sonar.ping_cm() > 0)
      {
        personFoundMessage();
        break;
      }
    }
    else if (i == 1)
    {
      motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
      delay(700);
      motors.setSpeeds(0, 0);
      delay(30);
      if (sonar.ping_cm() > 0)
      {
        personFoundMessage();
        break;
      }
    }
    else if (i == 2)
    {
      motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
      delay(700);
      motors.setSpeeds(0, 0);
      delay(30);
      if (sonar.ping_cm() > 0)
      {
        personFoundMessage();
        break;
      }
    }
    else if (i == 3)
    {
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      delay(700);
      motors.setSpeeds(0, 0);
      delay(30);
      if (sonar.ping_cm() > 0)
      {
        personFoundMessage();
        break;
      }
    }
  }
  if (objectFound == false)
  {
    Serial.print("No object detected!");
  }
  Serial.print("Please drive me outside the room!");
  moveZumo();
}

void personFoundMessage()
{
  delay(5);
  objectFound = true;
  roomScan = false;
  message = "Found a person at room ";
  Serial.print(message + roomCounter);
  Serial.print("!");
}

