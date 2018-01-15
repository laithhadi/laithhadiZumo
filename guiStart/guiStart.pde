import processing.serial.*;
import g4p_controls.*;

int bgcol = 15;
Serial myPort;
String val;
GCustomSlider sdr4;

public void setup() {
  size(500, 320);
  String portName = "COM12";
  myPort = new Serial(this,portName,9600);
  createGUI();
}

public void draw() {
  background(15);
  fill(227, 230, 255);
  modify();
  while (myPort.available() > 0) 
   {
     val = myPort.readStringUntil('!');
     if (val != null) {
       if(val.startsWith("R"))
       {
         showRoom.setText(val);
       }
       else
       {
         showText.appendText(val);
       }
     }
   }
}

public void modify()
{
   showCommands.setTextBold();
   showText.setTextBold();
   showRoom.setTextBold();
}