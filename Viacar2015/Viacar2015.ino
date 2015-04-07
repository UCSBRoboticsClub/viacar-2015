#include <IntervalTimer.h>
#include <SPI.h>
#include "Globals.h"
#include "RadioTerminal.h"
#include "Commands.h"
#include "ADC.h"
#include <cmath>


IntervalTimer controlTimer;
ADC adc;

void controlLoop();
float getPosition();
float volt2dist(float v);


void setup()
{
    asm(".global _printf_float");

    pinMode(led1Pin, OUTPUT);
    pinMode(led2Pin, OUTPUT);
    pinMode(led3Pin, OUTPUT);
    pinMode(led4Pin, OUTPUT);
    
    x.setCutoffFreq(50.f, dt);

    adc.setResolution(12);
    adc.setConversionSpeed(ADC_HIGH_SPEED);
    adc.setSamplingSpeed(ADC_HIGH_SPEED);
    adc.setAveraging(16);

    RadioTerminal::initialize();
    setupCommands();

    servoController.setOutputLimits(-60.f, 60.f);
    servoController.setTuning(1000.f, 0.f, 100.f);

    controlTimer.begin(controlLoop, controlPeriodUs);
    controlTimer.priority(144);
}

float buzFreq = 110.f;

void loop()
{
    delay(10);

    button.update();
    switch1.update();
    switch2.update();
    
    if (switch1.pressEdge())
        c2 = 0.f;

    const float pi = 3.14159f;
    const float rate = 2.f*pi/1000.f;
    analogWrite(led1Pin, int(65535.f * (std::sin(millis()*rate) + 1.f) * 0.5f));
    analogWrite(led2Pin, int(65535.f * (std::sin(millis()*rate - pi*0.5f) + 1.f) * 0.5f));
    analogWrite(led3Pin, int(65535.f * (std::sin(millis()*rate - pi) + 1.f) * 0.5f));
    analogWrite(led4Pin, int(65535.f * (std::sin(millis()*rate - pi*1.5f) + 1.f) * 0.5f));

    if (switch2.pressEdge())
    {
        buzFreq = 110.f;
        tone(buzzerPin, buzFreq);
    }

    if (button.releaseEdge())
    {
        buzFreq *= 1.0594631f;
        tone(buzzerPin, buzFreq);
    }

    if (switch2.releaseEdge())
        noTone(buzzerPin);
        
}


void controlLoop()
{
    x.push(getPosition());
    controllerOut = servoController.update(x);

    if (controllerEnabled)
        servo = controllerOut;

    motor = speed;
}


float getPosition()
{
    vr = adc.analogRead(sensorRPin) / 4096.f * 3.3f;
    vl = adc.analogRead(sensorLPin) / 4096.f * 3.3f;

    xr = volt2dist(vr);
    xl = volt2dist(vl);

    // Possible locations of the wire based on the two readings
    float candidates[] =
    {
        ( xr + xl) * 0.5f,
        (-xr + xl) * 0.5f,
        (-xr - xl) * 0.5f
    };

    // Difference between right and left distance measurements
    float rlDiff[] =
    {
         xr - xl + d,
        -xr - xl + d,
        -xr + xl + d
    };

    // Score assigned to each candidate wire location (lower is better)
    float score[3];
    for (int i = 0; i < 3; ++i)
        score[i] = rlDiff[i]*rlDiff[i] + (x-candidates[i])*(x-candidates[i]); 

    // Choose candidate with the lowest score
    int imin = 0;
    for (int i = 1; i < 3; ++i)
        imin = score[i] < score[imin] ? i : imin;

    return candidates[imin];
}


float volt2dist(float v)
{
    float a = h*std::exp(v/c1);

    if (switch1.pressed())
        c2 = a > c2 ? a : c2;

    float b = c2/a - 1.f;
    return h*std::sqrt(b > 0.f ? b : 0.f);
}

