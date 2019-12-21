#include "mbed.h"
#include "pindefs.h"
#include "pushbutton.h"

Pushbutton pb(LED_PIN, BUTTON_PIN);
DigitalOut rst_cont(RESET_CONTROLLER_EN);

int main()
{
    rst_cont = 0; //disable the ATTiny24 reset controller
    pb.init();

    printf("Printing this for debug's sake\r\n");

    while (true) {
        if(pb.state() == RELEASED) pb.start_blink(500);
        else if(pb.state() == SHORT_PRESSED) {
            pb.clear();
            printf("Short press\r\n");
        }
        else if(pb.state() == LONG_PRESSED) {
            pb.set();
            printf("Long press\r\n");
        }
    }
}

