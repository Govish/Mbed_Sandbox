#include "mbed.h"
#include "pca9685.h"
#include "pindefs.h"

#define MODE1 0x0
#define MODE2 0x1
#define PCA_ADDRESS 0x80

I2C bus(SDA_PIN, SCL_PIN);
PCA9685 device(PCA_ADDRESS, bus, 1000);

// main() runs in its own thread in the OS
int main()
{
    
    printf("Starting...\r\n");
    bus.frequency(100000);
    thread_sleep_for(1000);
    device.init();

    while (true) {
    }
}
