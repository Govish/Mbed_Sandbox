#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include "mbed.h"

typedef enum {
    RELEASED, 
    SHORT_PRESSED, 
    LONG_PRESSED
} b_states ;

class Pushbutton {
    public:
        Pushbutton(PinName led, PinName button);
        bool init();    
        void toggle();  // LED function
        void set();     // turn LED on
        void clear();   // turn LED off

        //pass millis to tell you how frequently to blink
        void start_blink(uint32_t millis); //blink the pushbutton LED at 1Hz
        void stop_blink();  //stop blinking the LED

        b_states state();   // get the state of the Pushbutton

    private:
        bool isBlinking;
        int bounceTick;    //delta timer for detecting long button presses

        DigitalIn button_pin;
        DigitalInOut led_pin;

        //using timer interrupts to run the debouncing and blink code
        //avoids a bunch of RAM usage by RTOS overhead
        LowPowerTicker debounceTick;
        LowPowerTicker blinkTick;

        //timer object to detect long press
        LowPowerTimer bounceTimer;

        //button state as an enumerated type (see above)
        //can change in interrupt context so declare as volatile
        volatile b_states buttonState;

        void sampleButton();
        void doBlink();
};

#endif 