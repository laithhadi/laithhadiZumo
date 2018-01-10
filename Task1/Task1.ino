#include <ZumoMotors.h>
#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <Pushbutton.h>


ZumoMotors motors;
ZumoReflectanceSensorArray sensors;
Pushbutton button(ZUMO_BUTTON);
int left, right;

// these might need to be tuned for different motor types
#define REVERSE_SPEED     200 // 0 is stopped, 400 is full speed
#define TURN_SPEED        200
#define FORWARD_SPEED     200
#define REVERSE_DURATION  200 // ms
#define TURN_DURATION     300 // ms

void setup()
{
  sensors.init();
  // Wait for the user button to be pressed and released
  button.waitForButton();

  // Turn on LED to indicate we are in calibration mode
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  // Wait 1 second and then begin automatic sensor calibration
  // by rotating in place to sweep the sensors over the line
  delay(1000);
  int i;
  for(i = 0; i < 80; i++)
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
  motors.setSpeeds(0,0);

  // Turn off LED to indicate we are through with calibration
  digitalWrite(13, LOW);
  Serial.begin(9600);
}

void loop()
{
     while (Serial.available()>0) {
        char motor = (char) Serial.read();
        if (motor == 'w')
        {
          left = 200;
          right = 200;
        }
        else if (motor == 's')
        {
          left = -200;
          right = -200;
        }
        else if (motor == 'd')
        {
          left = 150;
          right = 0;
        }
        else if (motor == 'a')
        {
          left = 0;
          right = 150;
        }
        else if (motor == 'x')
        {
          left = 0;
          right = 0;
        }              
    } 

  unsigned int sensorValues[6];
  
  sensors.readCalibrated(sensorValues);
  Serial.println(sensors.calibratedMaximumOn[0]);
  Serial.println(sensors.calibratedMaximumOff[0]);

     if (sensorValues[0] > sensors.calibratedMaximumOff[0])
  {
    // if leftmost sensor detects line, reverse and turn to the right
    motors.setSpeeds(0, 0);
    delay(REVERSE_DURATION);
   motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
    delay(TURN_DURATION);
    motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
  }
  else if (sensorValues[5] > sensors.calibratedMaximumOff[5])
  {
    // if rightmost sensor detects line, reverse and turn to the left
    motors.setSpeeds(0, 0);
    delay(REVERSE_DURATION);
    motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
   delay(TURN_DURATION);
   motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
  }
  else
  {
    motors.setSpeeds(left, right);
  }
}
