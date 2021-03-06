#include "Commands.h"
#include "Globals.h"
#include "RadioTerminal.h"
#include <IntervalTimer.h>
#undef min
#undef max
#include <functional>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>

#define GET(expr) [&]{ return float(expr); }
#define SETFLOAT(name) [&](float f){ name = f; }


struct Setter
{
    const char* name;
    std::function<void(float)> func;
};

struct Getter
{
    const char* name;
    std::function<float(void)> func;
};


const Setter setList[] =
{
    {"h", SETFLOAT(h)},
    {"d", SETFLOAT(d)},
    {"c1", SETFLOAT(c1)},
    {"c2", SETFLOAT(c2)},
    {"speed", SETFLOAT(speed)},
    {"kp", SETFLOAT(servoController.kp)},
    {"ki", SETFLOAT(servoController.ki)},
    {"kd", SETFLOAT(servoController.kd)},
    {"xmax", SETFLOAT(xmax)},
    {"srv.cen", SETFLOAT(servo.center)},
    {"srv.upd", SETFLOAT(servo.usPerDegree)},
    {"srv.ul", SETFLOAT(servo.upperLimit)},
    {"srv.ll", SETFLOAT(servo.lowerLimit)},
    {"srv.deg", [&](float f){ servo.write(f); }},
    {"en", [&](float f){ controllerEnabled = f > 0.f; }},
    {"sclim", SETFLOAT(scoreLimit)},
};


const Getter getList[] =
{
    {"h", GET(h)},
    {"d", GET(d)},
    {"c1", GET(c1)},
    {"c2", GET(c2)},
    {"speed", GET(speed)},
    {"sctrl", GET(speedCtrl)},
    {"sref", GET(speedRef)},
    {"kp", GET(servoController.kp)},
    {"ki", GET(servoController.ki)},
    {"kd", GET(servoController.kd)},
    {"srv.cen", GET(servo.center)},
    {"srv.upd", GET(servo.usPerDegree)},
    {"srv.ul", GET(servo.upperLimit)},
    {"srv.ll", GET(servo.lowerLimit)},
    {"srv.deg", GET(servo.read())},
    {"srv.pw", GET(servo.pulseWidth)},
    {"ctrl", GET(controllerOut)},
    {"en", GET(controllerEnabled)},
    {"vr", GET(vr)},
    {"vl", GET(vl)},
    {"xr", GET(xr)},
    {"xl", GET(xl)},
    {"xmax", GET(xmax)},
    {"x", GET(x)},
    {"but", GET(button.pressed())},
    {"sw1", GET(switch1.pressed())},
    {"sw2", GET(switch2.pressed())},
    {"scmin", GET(minScore)},
    {"sclim", GET(scoreLimit)},
    {"theta", GET(theta)},
    {"thetalp", GET(thetalp)},
    {"thetaest", GET(thetaest)},
    {"k", GET(curvature)},
    {"vel", GET(velocity)},
    {"acc", GET(accel)},
    {"enc", GET(encCounts)},
};


class WatchHandler : public CmdHandler
{
public:
    WatchHandler(std::function<float(void)> wf);
    virtual void sendChar(char c) { timer.end(); RadioTerminal::terminateCmd(); }
    static std::function<float(void)> watchFun;
    static IntervalTimer timer;
    static void refresh();
};

std::function<float(void)> WatchHandler::watchFun;
IntervalTimer WatchHandler::timer;


CmdHandler* watch(const char* input)
{
    char buf[32];
    const int getListSize = (sizeof getList) / (sizeof getList[0]);

    const char* s = std::strchr(input, ' ');
    if (s != nullptr)
    {
        ++s;
        for (int i = 0; i < getListSize; ++i)
        {
            if (std::strncmp(s, getList[i].name, 32) == 0)
                return new WatchHandler(getList[i].func);
        }
    }

    RadioTerminal::write("Usage: w <var>\nValid vars:");
    for (int i = 0; i < getListSize; ++i)
    {
        snprintf(buf, 32, "\n  %s", getList[i].name);
        RadioTerminal::write(buf);
    }
    return nullptr;
}


WatchHandler::WatchHandler(std::function<float(void)> wf)
{
    watchFun = wf;
    timer.begin(&WatchHandler::refresh, 200000);
}


void WatchHandler::refresh()
{
    char output[256];

    sprintf(output, "\r         \r\r\r%4.4f", watchFun());
    RadioTerminal::write(output);
}


CmdHandler* print(const char* input)
{
    char buf[32];
    const int getListSize = (sizeof getList) / (sizeof getList[0]);
    
    const char* s = std::strchr(input, ' ');
    if (s != nullptr)
    {
        ++s;
        for (int i = 0; i < getListSize; ++i)
        {
            if (std::strncmp(s, getList[i].name, 32) == 0)
            {
                snprintf(buf, 32, "%s = %4.4f",
                         getList[i].name,
                         getList[i].func());
                RadioTerminal::write(buf);
                return nullptr;
            }
        }
    }
    
    RadioTerminal::write("Usage: p <var>\nValid variables:");
    for (int i = 0; i < getListSize; ++i)
    {
        snprintf(buf, 32, "\n  %s", getList[i].name);
        RadioTerminal::write(buf);
    }
    return nullptr;
}


CmdHandler* set(const char* input)
{
    char buf[32];
    const int setListSize = (sizeof setList) / (sizeof setList[0]);
    
    const char* s = std::strchr(input, ' ');
    if (s != nullptr)
    {
        const char* s2 = std::strchr(++s, ' ');
        if (s2 != nullptr)
        {
            int pslen = s2 - s;
            float value = strtof(++s2, nullptr);
            
            for (int i = 0; i < setListSize; ++i)
            {
                if (std::strncmp(s, setList[i].name, pslen) == 0)
                {
                    setList[i].func(value);
                    snprintf(buf, 32, "%s = %4.4f",
                             setList[i].name,
                             value);
                    RadioTerminal::write(buf);
                    return nullptr;
                }
            }
        }
    }
    
    RadioTerminal::write("Usage: s <var> <value>\nValid variables:");
    for (int i = 0; i < setListSize; ++i)
    {
        snprintf(buf, 32, "\n  %s", setList[i].name);
        RadioTerminal::write(buf);
    }
    return nullptr;
}


void setupCommands()
{
    RadioTerminal::addCommand("w", &watch);
    RadioTerminal::addCommand("p", &print);
    RadioTerminal::addCommand("s", &set);
}


// Workaround for teensyduino/libstdc++ bug
void std::__throw_bad_function_call() { while(true) {} };
