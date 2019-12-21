#include "mbed.h"
#include "pindefs.h"

#define FUSB_ADDR 0x22<<1

#define DEVICE_ID_REG 0x01
#define SWITCHES_BASE 0x02 //can write to these in 16-bit succession switches[0] -> switches[1]
#define SWITCHES_0_REG 0x02
#define SWITCHES_1_REG 0x03

#define MEASURE_REG 0x04
#define SLICE_REG 0x05

#define CONTROL_REG_BASE 0x06 //write 32 bits consecutively from control0 [0] -> control 3 [3]
#define CONTROL_0_REG 0x06
#define CONTROL_1_REG 0x07
#define CONTROL_2_REG 0x08
#define CONTROL_3_REG 0x09
#define CONTROL_4_REG 0x10 //LOLOLOL be careful here not consecutive!!!

//============= INTERRUPT MASKS ============
#define MASK_REG 0x0A
#define MASK_A_REG 0x0E
#define MASK_B_REG 0x0F

//============= INTERRUPT LATCHES ============
#define INTERRUPT_REG_BASE 0x3E //for consecutive reads
#define INTERRUPT_A_REG 0x3E
#define INTERRUPT_B_REG 0x3F
#define INTERRUPT_REG 0x42 //the other weird interrupt reg

#define POWER_REG 0x0B
#define RESET_REG 0x0C
#define OCP_REG 0x0D

#define STATUS_01_REG_BASE 0x40 //for consecutive reads
#define STATUS_0_REG 0x40
#define STATUS_1_REG 0x41
#define STATUS_0A_REG 0x3C
#define STATUS_1A_REG 0x3D

#define FIFO_REG 0x43

DigitalIn int_pin(USB_ALT);
DigitalIn sda(SDA_PIN);
DigitalIn scl(SCL_PIN);

DigitalOut led(PA_5);

I2C bus(SDA_PIN, SCL_PIN);
InterruptIn fusb_alt(USB_ALT);

volatile bool int_triggered;

volatile uint8_t int_a, int_b, int_reg;

void printReg(uint8_t reg) {
    char send[1];
    char recv[1];
    send[0] = reg;
    bus.write(FUSB_ADDR, send, sizeof(send));
    bus.read(FUSB_ADDR, recv, sizeof(recv));

    printf(" contents: 0x%x\n\r", recv[0]);
}

uint8_t readReg(uint8_t reg) {
    char send[1];
    char recv[1];
    send[0] = reg;
    bus.write(FUSB_ADDR, send, sizeof(send));
    bus.read(FUSB_ADDR, recv, sizeof(recv));

    return recv[0];
}

void writeReg(uint8_t reg, uint8_t value){
    char send[2];
    send[0] = reg;
    send[1] = value;

    bus.write(FUSB_ADDR, send, 2);
}

void print_int(void) {
    int_triggered = true;
    led = !led;
}

// main() runs in its own thread in the OS
int main()
{
    printf("\n\r");

    int_pin.mode(PullUp);
    sda.mode(PullUp);
    scl.mode(PullUp);

    bus.frequency(100000);
    printf("DEVICE_ID_REG");
    printReg(DEVICE_ID_REG);

    fusb_alt.fall(callback(print_int));


    /*
    writeReg(SWITCHES_1_REG, 0x27); //enable AUTO_CRC
    printf("SWITHCES_1_REG");
    printReg(SWITCHES_1_REG);

    writeReg(CONTROL_3_REG, 0b00011111); //enable all auto reset/crc functions
    printf("CONTROL_3_REG");
    printReg(CONTROL_3_REG);
    */

    /*
    detect attach by reading the VBUSOK interrupt\
        VBUSOK = 1 -> ATTACHED
        VBUSOK = 0 -> DETACHED
    BC_LVL and COMP interupt and status bits tell orientation of cable
        BC_LVL = 00 -> vRA (particular CC pin is pulled down)
        BC_LVL = 01 -> vRD is connected at default current
        BC_LVL = 10 -> vRD is connected at 1.5A current
        detecting 3A VBUS current requires setting the comparator (not worth messing with)

    If detecting attach through toggle
        set HOST_CUR1 = 0       x
            HOST_CUR0 = 1       x
            VCONN_CC1 = 0       x
            VCONN_CC2 = 0       x
            MASK_REG = 0xFE     x
            MASK_A_REG = 0xBF   x  
            MASK_B_REG = 0x01   x
            POWER_REG = 0x01    x
        and clear all interrupts
        enable the autonomous detection by setting TOGGLE to 1

        The thing is, detecting attach through toggle takes 40uA of current
        Lets try to detect attach through vbus interrupts
    */
    writeReg(CONTROL_0_REG, 0b00000100); //enable interrupts and set current to 0
    printf("CONTROL_0_REG");
    printReg(CONTROL_0_REG);

    writeReg(SWITCHES_0_REG, 0b00000011); //disconnect VCONN
    printf("SWITCHES_0_REG");
    printReg(SWITCHES_0_REG);

    writeReg(MASK_REG, 0xFF); //unmask BC_LVL 
    printf("MASK_REG");
    printReg(MASK_REG);

    writeReg(MASK_B_REG, 0x01); //mask everything (GCRC) 
    printf("MASK_B_REG");
    printReg(MASK_B_REG);

    writeReg(MASK_A_REG, 0xBF); //unmask TOG_DONE 
    printf("MASK_A_REG");
    printReg(MASK_A_REG);

    writeReg(POWER_REG, 0x01); //enable wake and bandgap circuit
    printf("POWER_REG");
    printReg(POWER_REG);

    //clear and print all interrupt regs
    printf("INTERRUPT regs (A, B, _)\r\n");
    printReg(INTERRUPT_A_REG);
    printReg(INTERRUPT_B_REG);
    printReg(INTERRUPT_REG);

    writeReg(CONTROL_2_REG, 0b11000100); //disable toggling
    printf("CONTROL_2_REG");
    printReg(CONTROL_2_REG);

    writeReg(CONTROL_2_REG, 0b11000101); //enable toggling
    printf("CONTROL_2_REG");
    printReg(CONTROL_2_REG);

    while (true) {
        if(int_triggered) {
            fusb_alt.disable_irq();
            int_triggered = false;
            do {
                int_a = readReg(INTERRUPT_A_REG);
                int_b = readReg(INTERRUPT_B_REG);
                int_reg = readReg(INTERRUPT_REG);
                printf("interrupt received\n\r");
                printf("int_a: 0x%x\n\r", int_a);
                printf("int_b: 0x%x\n\r", int_b);
                printf("int_reg: 0x%x\n\r", int_reg);
            } while ((int_a != 0) || (int_b != 0) || (int_reg != 0));
            fusb_alt.enable_irq();
        }
    }
}