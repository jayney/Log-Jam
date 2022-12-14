/*
 * ResponsiveAnalogRead.h
 * Arduino library for eliminating noise in analogRead inputs without decreasing responsiveness
 *
 * Copyright (c) 2016 Damien Clarke
 * 
 * Modified 2020 by Jayne Atkinson for STM32 and multiple A-D Convertors
 * Tested with PlatformIO & LibopenCM3 and STM32F446RET6
 * 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.  
 *  
 */
 
#ifndef RESPONSIVE_ANALOG_READ_H
#define RESPONSIVE_ANALOG_READ_H

//#include <Arduino.h>

class ResponsiveAnalogRead
{
  public:

    // pin - the pin to read
    // sleepEnable - enabling sleep will cause values to take less time to stop changing and potentially stop changing more abruptly,
    //   where as disabling sleep will cause values to ease into their correct position smoothly
    // snapMultiplier - a value from 0 to 1 that controls the amount of easing
    //   increase this to lessen the amount of easing (such as 0.1) and make the responsive values more responsive
    //   but doing so may cause more noise to seep through if sleep is not enabled
    
    ResponsiveAnalogRead(){};  //default constructor must be followed by call to begin function
    ResponsiveAnalogRead(int adc, int pin, bool sleepEnable, float snapMultiplier){
        begin(adc, pin, sleepEnable, snapMultiplier);
    };

    void begin(int adc, int pin, bool sleepEnable, float snapMultiplier);  // use with default constructor to initialize 
    
    inline int getValue() { return responsiveValue; } // get the responsive value from last update
    inline int getRawValue() { return rawValue; } // get the raw analogRead() value from last update
    inline bool hasChanged() { return responsiveValueHasChanged; } // returns true if the responsive value has changed during the last update
    inline bool isSleeping() { return sleeping; } // returns true if the algorithm is currently in sleeping mode
    void update(); // updates the value by performing an analogRead() and calculating a responsive value based off it
    void update(int rawValueRead); // updates the value accepting a value and calculating a responsive value based off it

    void setSnapMultiplier(float newMultiplier);
    inline void enableSleep() { sleepEnable = true; }
    inline void disableSleep() { sleepEnable = false; }
    inline void enableEdgeSnap() { edgeSnapEnable = true; }
    // edge snap ensures that values at the edges of the spectrum (0 and 1023) can be easily reached when sleep is enabled
    inline void disableEdgeSnap() { edgeSnapEnable = false; }
    inline void setActivityThreshold(float newThreshold) { activityThreshold = newThreshold; }
    // the amount of movement that must take place to register as activity and start moving the output value. Defaults to 4.0
    inline void setAnalogResolution(int resolution) { analogResolution = resolution; }
    // if your ADC is something other than 10bit (1024), set that here

  private:
    int adc;
    int pin;
    int analogResolution = 4096;
    float snapMultiplier;
    bool sleepEnable;
    float activityThreshold = 4.0;
    bool edgeSnapEnable = true;

    float smoothValue;
    unsigned long lastActivityMS;
    float errorEMA = 0.0;
    bool sleeping = false;

    int rawValue;
    int responsiveValue;
    int prevResponsiveValue;
    bool responsiveValueHasChanged;

    int getResponsiveValue(int newValue);
    float snapCurve(float x);
};

#endif