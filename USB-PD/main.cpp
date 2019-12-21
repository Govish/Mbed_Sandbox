#include "mbed.h"

#include "pindefs.h"
#include "pd_negotiator.h"

#define I2C_ADDR 0x28<<1 //8-bit
#define DEVICE_ID_REG 0x2F
#define CC_STATUS_REG 0x11

#define ALERT_STATUS_1_MASK_REG 0x0C
#define ALERT_STATUS_1_REG 0x0B

#define PORT_STATUS_0_REG 0x0D
#define PORT_STATUS_1_REG 0x0E

I2C bus(SDA_PIN, SCL_PIN);
InterruptIn alt(USB_ALT, PullUp);

void printReg(uint8_t reg) {
    char send[1];
    char recv[1];
    send[0] = reg;
    bus.write(I2C_ADDR, send, sizeof(send));
    bus.read(I2C_ADDR, recv, sizeof(recv));

    printf(" contents: 0x%x\n\r", recv[0]);
}

uint8_t readReg(uint8_t reg) {
    char send[1];
    char recv[1];
    send[0] = reg;
    bus.write(I2C_ADDR, send, sizeof(send));
    bus.read(I2C_ADDR, recv, sizeof(recv));

    return recv[0];
}

void writeReg(uint8_t reg, uint8_t value){
    char send[2];
    send[0] = reg;
    send[1] = value;

    bus.write(I2C_ADDR, send, 2);
}

void i2c_handler() {
    printf("\r\n");
    printf("Handling interrupt in context %p\r\n", ThisThread::get_id());
    printf("ALERT_STATUS_1_REG");
    printReg(ALERT_STATUS_1_REG);
    printf("PORT_STATUS_0_REG");
    printReg(PORT_STATUS_0_REG);
    printf("PORT_STATUS_1_REG");
    printReg(PORT_STATUS_1_REG);

    //read up to PRT_STATUS to clear interrupt line
    for(int i = 0; i < 12; i++) {
        readReg(ALERT_STATUS_1_REG + i);
    }

    CC_Reg_Map cc_reg;
    cc_reg.data = readReg(CC_STATUS_REG);

    printf("\n\r");
    printf("CC1 Status: %d \n\r", cc_reg.map.cc1);
    printf("CC2 Status: %d \n\r", cc_reg.map.cc2);
    printf("Sink(1), Source(0): %d \n\r", cc_reg.map.conn_res);
    printf("Searching?: %d \n\r", cc_reg.map.looking);
}

int main()
{
    bus.frequency(100000);
    printf("Read DEVICE_ID: ");
    printReg(DEVICE_ID_REG);

    Alt_S1Reg_Map alert;
    alert.data = 0xFF;
    alert.map.prt_mask = 0;
    writeReg(ALERT_STATUS_1_MASK_REG, alert.data);
    printf("ALERT_STATUS_1_MASK_REG");
    printReg(ALERT_STATUS_1_MASK_REG);

    EventQueue *queue = mbed_event_queue();
    alt.fall(queue->event(i2c_handler));
    printf("Starting in context %p\r\n", ThisThread::get_id());

    while (true) {
        printf("No Interrupt\r\n");
        thread_sleep_for(1000);
    }
}

