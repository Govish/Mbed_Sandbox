#include "mbed.h"
#include "pindefs.h"
#include "Charger.h"

// main() runs in its own thread in the OS
DigitalOut rst(RESET_CONTROLLER_EN);
Charger charger(ILIM_PIN, RUN_PIN, CHG_PIN);

int main()
{
    rst = 0;
    charger.start_charge(2.5);
    while (true) {
    }
}

