import processing.serial.*;
import g4p_controls.*;

int bgcol = 15;
Serial myPort;
String val;

public void setup() {
  size(500, 360);
  String portName = "COM4";
  myPort = new Serial(this,portName,9600);
  createGUI();
}

public void draw() {
  background(200, 200, 200);
  fill(227, 230, 255);

}