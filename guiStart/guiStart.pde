import processing.serial.*;
import g4p_controls.*;

int bgcol = 15;
Serial myPort;
String val;

public void setup() {
  size(500, 320);
  String portName = "COM4";
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
     showText.appendText(val);
     }
   }
}

public void modify()
{
   showCommands.setTextBold();
   showText.setTextBold();
}