#include <motor.h>

Servo Sg90servo;    // create servo object to control a servo
int angle = 90;    // variable to store the servo position
Sg90servo.attach(14);  // attaches the servo on pin 4 to the servo object


void motor(const char* tempo) {
  for (angle = 90; angle >= 1; angle--) 
  {
    Sg90servo.write(angle);              
    delay(20);                       
  }
  delay(atoi(tempo));
  for (angle = 0; angle < 90; angle++) 
  { 
    Sg90servo.write(angle);              
    delay(20);                       
  }
  
}