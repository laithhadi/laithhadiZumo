import processing.serial.*;
import g4p_controls.*;

int bgcol = 15;
Serial myPort;
String val;
GCustomSlider sdr4;

public void setup() {
  size(500, 320);
  String portName = "COM4";
  myPort = new Serial(this,portName,9600);
  createGUI();
  
  sdr4 = new GCustomSlider(this, 290, 60, 200, 50, "blue18px");
  sdr4.setShowDecor(false, true, true, true);
  sdr4.setNbrTicks(6);
  sdr4.setStickToTicks(true);
  sdr4.setEasing(20); 
  sdr4.setLimits(100, 400);
  sdr4.setLocalColorScheme(7); 
  sdr4.setValue(100); 
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
void handleSliderEvents(GSlider slider) {
  int speed = slider.getValueI();
  myPort.write('j');
  myPort.write(speed);
}