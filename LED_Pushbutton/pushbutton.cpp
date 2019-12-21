#include "pushbutton.h"

#define LONG_PRESS_DURATION 2000

Pushbutton::Pushbutton(PinName led, PinName button) :
    led_pin(led),
    button_pin(button)
{}

bool Pushbutton::init() {
    led_pin.output();
    led_pin.mode(OpenDrain);

    //attach the blink/debouce timestamp
    debounceTick.attach_us(callback(this, &Pushbutton::sampleButton), 25000);

    //start the timer for the long press timer 
    bounceTimer.start();

    bool success_led = led_pin.is_connected();
    bool success_button = button_pin.is_connected();
    bool isBlinking = false;

    return success_button && success_led;
}

void Pushbutton::toggle() {
    if(isBlinking) {
        blinkTick.detach();
        isBlinking = false;
    }
    led_pin = !led_pin;
}

void Pushbutton::set() {
    if(isBlinking) {
        blinkTick.detach();
        isBlinking = false;
    }
    led_pin = false;
}

void Pushbutton::clear() {
    if(isBlinking) {
        blinkTick.detach();
        isBlinking = false;
    }
    led_pin = true;
}

//pass millis to tell you how frequently to blink
void Pushbutton::start_blink(uint32_t millis) {
    //blink every half a second
    if(isBlinking) return;
    blinkTick.attach_us(callback(this, &Pushbutton::doBlink), (millis >> 1) * 1000);
    isBlinking = true;
}

void Pushbutton::stop_blink() {
    if(!isBlinking) return;
    blinkTick.detach();
    isBlinking = false;
}

b_states Pushbutton::state() {
    return buttonState;
}

//=========== private functions ===========
//run indefinitely, but only toggle when we enable
void Pushbutton::doBlink() {
    led_pin = !led_pin;
}

//run this indefinitely as well
void Pushbutton::sampleButton() {
    //a little memory element
    //if lastButton is the same as the current button, update the state
    static bool lastButton;

    //if the button is done bouncing
    if(lastButton == button_pin) {
        //if the button is released
        if(!button_pin) {
            buttonState = RELEASED;
            bounceTimer.reset();
        }

        //if the button is released
        else {
            if(bounceTimer.read_ms() > LONG_PRESS_DURATION) buttonState = LONG_PRESSED;
            else buttonState = SHORT_PRESSED;
        }
    }
    lastButton = button_pin;
}
