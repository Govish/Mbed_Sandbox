#include "mbed.h"
#include "pindefs.h"

I2C i2c(SDA_PIN, SCL_PIN);        // sda, scl
 
int main()
{
    i2c.frequency(100000);
    printf("Printing this for debug's sake\r\n");
    printf("RUN\r\n");
    for(int i = 0; i < 128 ; i++) {
        //i2c.start();
        printf("Testing address 0x%x :", i);
        if(i2c.write(i << 1, NULL, 0, false) == 0) printf("0x%x ACK \r\n",i); // Send command string
        else printf("NACK\r\n");
        //i2c.stop();
    }
}