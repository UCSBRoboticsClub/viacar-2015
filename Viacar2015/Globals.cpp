#include "Globals.h"
#include <Arduino.h>


float vr = 0.f;
float vl = 0.f;
LowPass xr;
LowPass xl;
LowPass x;
float h = 0.06f;
float d = 0.13f;
float c1 = 0.2f;
float c2 = 4.4e3f;
float speed = 0.45f;
float controllerOut = 0.f;
bool controllerEnabled = true;
float xmax = 0.5f;
LowPass minScore;
float scoreLimit = 0.1f;
float theta = 0.f;
LowPass thetalp;
float thetaest = 0.f;
float curvature = 0.f;
float vel = 0.f;

Motor motor(motorFwdPin, motorBackPin, motorSpeedPin);
Servo servo(servoPin);
Button button(buttonPin, LOW, true);
Button switch1(switch1Pin, LOW, true);
Button switch2(switch2Pin, LOW, true);
PIDController servoController(dt);
