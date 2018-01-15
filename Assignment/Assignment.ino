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
#define NUM_SENSORS        6  // the number of sensors attached to robot
#define FORWARD_SPEED     130 // how fast the robot can go forward
#define REVERSE_SPEED     150 // how fast it takes to revsere
#define TURN_SPEED        150 // how fast it takes to turn
#define REVERSE_DURATION  150 // how long it takes to revsere in ms
#define TURN_DURATION     150 // how long it takes to turn in ms
#define MAX_DISTANCE      30  // Maximum distance we want to ping for (in centimeters)
#define TRIGGER_PIN        2  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN           6  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define STRING_TERMINATOR "!" // used as the last char to be sent over serial connection
//declration of objects
ZumoMotors motors;
ZumoReflectanceSensorArray sensors;
ZumoBuzzer buzzer;
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
//variable declrations
unsigned int sensorValues[NUM_SENSORS]; // declare number of sensors on the zumo
int calibratedValue[6];                 // the calibrated QTR_THRESHOLD of the black line 
int roomCounter = 0;                    // used to count the number of rooms
int corridorCounter = 1;                // used to count the number of corridors
int previousCorridor = 0;               // used to store previous corridor when entering side corridors
String rooms[50];                       // used to store the direction of each room
String message;                         // used to store a specific message to send over to GUI
char input, previousInput;              // used to store input
bool start = false;                     // ensures that the robot does not start instantly aftar calibration
bool roomScan = false;                  // used to store the state of scanning a room
bool objectFound = false;               // checks to see if a person/object is found during scan
bool isSideCorridor = false;            // checks whether the robot is inside a side-corridor
bool isAtEnd = false;                   // checks whether the robot is at the end of a side-corridor
/*------------------------------------------------------------------------
  Setup function
  -------------------------------------------------------------------------*/
void setup()
{
  Pushbutton button(ZUMO_BUTTON);

  Serial.begin(9600);                 // begin serial communication at 9600 bits
  buzzer.play(">g32>>c32");           // Play a little welcome song
  sensors.init();                     // Initialize the reflectance sensors module
  char input = (char) Serial.read();  // waiting for GUI to send calibrate command, 't'
  while (input != 't')
  {
    input = (char) Serial.read();
  }
  calibrateZumo();                    //calibrate the zumo to the required environment
  // store the calibrated values for each sensor into the array
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    calibratedValue[i] = sensors.calibratedMaximumOn[i];
  }
  Serial.print("Calibration completed!");
  Serial.print("Zumo is at corridor "); // start 1 as the intial corridor
  Serial.print(corridorCounter);
  Serial.print(STRING_TERMINATOR);      
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
  // check for available input 
  while (Serial.available() > 0 && previousInput != 'c')
  {
    start = true;  // start the zumo border detection as we have input
    input = (char) Serial.read();

    //used to stop zumo
    if (input == 'x')
    {
      motors.setSpeeds(0, 0);
      moveZumo(); // move the zumo manually
    }
    //used to stop zumo at side-corridor
    else if (input == 'f')
    {
      motors.setSpeeds(0, 0);
      previousCorridor = corridorCounter; // save the previous corridor
      corridorCounter++;                  // allocate a corridor ID to this new side-corridor
      message = "Zumo is at side-corridor number ";
      isAtEnd = true;                     // used to show that robot is approaching dead end
      Serial.print(message + corridorCounter);
      Serial.print(STRING_TERMINATOR);
      moveZumo(); // move the zumo manually
    }
    //used to automatically move the robot until it hits a wall
    else if (input == 'p')
    {
      motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    }
    //used for room processing
    else if (input == 'z')
    {
      motors.setSpeeds(0, 0);
      Serial.print("Please show me the direction of the room!");
      input = (char) Serial.read();
      //check the direction of the room
      while (!(input == 'l' || input == 'r'))
      {
        input = (char) Serial.read();
      }
      //if the direction of the room is left
      if (input == 'l')
      {
        rooms[roomCounter] = "left";                //save the direction into that room
        motors.setSpeeds(-TURN_SPEED, TURN_SPEED);  //manually adjust depending on input
        delay(TURN_DURATION);
        motors.setSpeeds(0, 0);
      }
      //if the direction of the room is right
      else
      {
        rooms[roomCounter] = "right";               //save the direction into that room
        motors.setSpeeds(TURN_SPEED, -TURN_SPEED);  //manually adjust depending on input
        delay(TURN_DURATION);
        motors.setSpeeds(0, 0);
      }
      message = "Room: ";
      //send the room details to GUI
      Serial.print(message + (roomCounter + 1));    
      message = " on the " + rooms[roomCounter] + " side of corridor ";
      Serial.print(message + corridorCounter);
      Serial.print(STRING_TERMINATOR);
      delay(10);
      roomScan = true;    //used to show that the room needs to be scanned
      roomCounter++;      //increment the room number
      moveZumo();         // move the zumo manually
    }
  }
  //if a complete command was requested
  if (previousInput == 'c')
  {
    if (roomScan)         //if it was requested after finding a room
    {
      scanRoom();
    }
    motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    Serial.print("Automatic zumo control!");
    delay(10);
    previousInput = ' ';  //reset the 'complete' command
  }
}
// move the zumo manually
void moveZumo()
{
  char zumoDirection = ' ';
  //check to see if GUI is sending any commands
  //for a specific direction
  while (zumoDirection != 'c')
  {
    zumoDirection = (char) Serial.read();
    //forward
    if (zumoDirection == 'w')
    {
      motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
      delay(REVERSE_DURATION);
      motors.setSpeeds(0, 0);
    }
    //backwards
    else if (zumoDirection == 's')
    {
      motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
      delay(REVERSE_DURATION);
      motors.setSpeeds(0, 0);
    }
    //right
    else if (zumoDirection == 'r')
    {
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      delay(TURN_DURATION);
      motors.setSpeeds(0, 0);
    }
    //left
    else if (zumoDirection == 'l')
    {
      motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
      delay(TURN_DURATION);
      motors.setSpeeds(0, 0);
    }
  }
  previousInput = 'c';    //request that a 'complete' command needs to be done
}

//used to detect wall/corner and set behaviour
void detectWall()
{
  sensors.read(sensorValues);   //read the raw values from sensors
  if (!checkCorner())           //if we did not detect a wall
  {
    //if the leftmost sensor detects line
    if (sensorValues[0] >= calibratedValue[0])
    {
      motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
      delay(REVERSE_DURATION);
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      delay(TURN_DURATION);
      motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    }
    //if the rightmost sensor detects line
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
//used to check for wall
bool checkCorner()
{
  //check leftmost and rightmost sensors first
  if (sensorValues[0] >= calibratedValue[0] || sensorValues[5] >= calibratedValue[5])
  {
    delay(30);
    sensors.read(sensorValues);
    delay(5);
    //if any of the other 4 sensors detect the line
    if ((sensorValues[1] >= calibratedValue[1]) || (sensorValues[4] >= calibratedValue[4])
        || (sensorValues[3] >= calibratedValue[3]) || (sensorValues[2] >= calibratedValue[2]))
    {
      motors.setSpeeds(0, 0);
      motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
      delay(REVERSE_DURATION);
      motors.setSpeeds(0, 0);
      buzzer.playNote(NOTE_A(5), 200, 15);
      //check if the robot is inside a side-corridor
      if (isSideCorridor)
      {
        isSideCorridor = false;
        Serial.print("Exisiting side corridor number ");
        Serial.print(corridorCounter);
        Serial.print(STRING_TERMINATOR);
        delay(10);
        message = "Zumo is at corridor number ";
        Serial.print(message + previousCorridor);
        Serial.print(STRING_TERMINATOR);
        delay(10);
        previousCorridor = 0;   //reset the previous corridor
      }
      else if (isAtEnd)
      {
        isSideCorridor = true;
        isAtEnd = false;
        Serial.print("End of side-corridor number ");
        Serial.print(corridorCounter);
        Serial.print(STRING_TERMINATOR);
        delay(10);
      }
      else
      {
        Serial.print("Corner ahead. Manual mode activated!");    //Display a message showing a corner has been found
        delay(10);
        corridorCounter++;
        message = "Zumo is at corridor number ";
        Serial.print(message + corridorCounter);
        Serial.print(STRING_TERMINATOR);
        delay(10);
      }
      moveZumo(); // move the zumo manually
      return true;
    }
    return false;
  }
}
void scanRoom()
{
  objectFound = false;
  motors.setSpeeds(200, 200);
  delay(450);
  //turn right
  for (int i = 0; i < 24 && objectFound == false; i++)
  {
    motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
    delay(40);
    motors.setSpeeds(0, 0);
    //scan for object
    if (sonar.ping_cm() > 0)
    {
      //send message to GUI
      personFoundMessage();
      break;
    }
  }
  //turn left similar to 180 degrees
  for (int i = 0; i < 48 && objectFound == false; i++)
  {
    motors.setSpeeds(-TURN_SPEED, +TURN_SPEED);
    delay(40);
    motors.setSpeeds(0, 0);
    if (sonar.ping_cm() > 0)
    {
      personFoundMessage();
      break;
    }
  }
  if (objectFound == false)
  {
    Serial.print("No object detected!");
    delay(10);
  }
  Serial.print("Please drive me outside the room!");
  delay(10);
  moveZumo(); // move the zumo manually
}
//used to reset global variables and communicate with GUI
void personFoundMessage()
{
  delay(5);
  objectFound = true;
  roomScan = false;
  message = "Found a person at room ";
  Serial.print(message + roomCounter);
  Serial.print(STRING_TERMINATOR);
  delay(10);
  buzzer.play("! V10 cdefgab>cbagfedc");
}

