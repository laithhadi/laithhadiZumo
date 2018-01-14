# Search and rescue operation based on Pololu Zumo robot 

The scenario motivating this idea is that the zumo robot is trying to find/rescue people trapped on a single floor in a building which is filled with smoke. The robot moves through a corridor and people are to be discovered in rooms or in the corridor. When the robot discovers a 'person' it signals back to ‘base’ so that a search party can come in to rescue that person. The robot, however, continues to search, signalling as and when it finds people in other rooms.

# Technologies used

Processing: it was the IDE that I used to create the GUI. It involved the use of serial port through an XBee connection which is responsible for sending commands to Zumo. I used the library G4P (http://www.lagers.org.uk/g4p/) in Processing as it provided me with a suitable GUI builder tool.

Arduino: the robot side was programmed in Arduino IDE, and it was responsible for processing all the commands sent by the GUI. For the most part, it received an action, and processed that action. It involved the use of sensors such as reflectance sensor array and the ultrasonic sensor. The Arduino board is connected to a Zumo robot and the XBee shield with extensions to provide more space for attaching the ultrasonic sensor at the front.

# Programming approach

My first priority was to ensure that the reflectance sensor array used accurate readings in order to detect the walls/corners based on any environment as it used calibration which I used based on LineFollower example (https://github.com/pololu/zumo-shield/tree/master/ZumoExamples/examples/LineFollower). Secondly, I needed to code a robust function that would organise the communication between the robot and the GUI while separating the interaction from other functions. This allowed me to test bugs much more efficiently. At this point, to get my robot to avoid borders and stop at corners, I used code from BorderDetect example (https://github.com/pololu/zumo-shield/tree/master/ZumoExamples/examples/BorderDetect) and modified by using multiple sensors depending on different conditions to check wall. To store room and corridor data, I used arrays which store the state at different parts of the code whenever needed, for instance, to store the next corridor, code was used at wall/corner detection. As for object detection, I utilised the use of ultrasonic sensor by moving the robot to the right, left and the initial direction at entry to scan for people; in other words, it scans 180 degrees.

# Libaries used

1. ZumoMotors https://github.com/pololu/zumo-shield 
2. ZumoBuzzer https://github.com/pololu/zumo-shield 
3. QTRSensors https://github.com/pololu/zumo-shield 
4. ZumoReflectanceSensorArray https://github.com/pololu/zumo-shield 
5. Pushbutton https://github.com/pololu/zumo-shield 
6. NewPing https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home
