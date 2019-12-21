/*
    Credit for most of this library goes to Calum Johnston
    https://os.mbed.com/users/el13cj/code/PCA9685/

    Changes made:
     - Changed/fixed the constructor to take a pointer to i2c object
     - Locking I2C bus when transmitting
     - Changed outputs to open drain
     - Changed prescale function slightly
     - added sleep and wake functions
*/

 
#include "mbed.h"
#include "pca9685.h"

#define MODE1 0x0
#define MODE2 0x1
#define PRESCALE 0xFE
 
#define LED0_ON_L 0x6
#define LED0_ON_H 0x7
#define LED0_OFF_L 0x8
#define LED0_OFF_H 0x9
 
#define ALLLED_ON_L 0xFA
#define ALLLED_ON_H 0xFB
#define ALLLED_OFF_L 0xFC
#define ALLLED_OFF_H 0xFD
 
#define OSC_CLOCK 25e6
 
#define PWM_SCALER 0.90425 //set by manual calibration - ratio of FREQUENCY_SET:ACTUAL_OUTPUT
 
PCA9685::PCA9685(uint8_t i2c_address, I2C &i2c_object, float frequency) :
    //arm uses 8-bit addresses
    i2c_addr(i2c_address),
    freq(frequency),
    i2c_bus(&i2c_object)
{}
 
void PCA9685::init(void)
{ 
    uint8_t prescale = (uint8_t) (OSC_CLOCK / (4096 * PWM_SCALER * freq)) - 1;

    wake();

    write_8(MODE1, 0x20); //0010 0000 : AI ENABLED
    write_8(MODE2, 0x12); //0001 0010 : INVRT, CHANGE ON STOP, OPEN_DRAIN, \OE = 1, LEDn = HIGH IMP
 
    set_prescale(prescale);
 
}

 
void PCA9685::set_prescale(uint8_t prescale)
{
 
    sleep();
    write_8(PRESCALE, prescale); // set the prescaler
    wake();
}

void PCA9685::sleep() {
    uint8_t oldmode = read_8(MODE1);
    uint8_t newmode = (oldmode) | 0x10; // set the sleep bit
 
    write_8(MODE1, newmode); // send the device to sleep
    wait_us(500);
}

void PCA9685::wake() {
    uint8_t oldmode = read_8(MODE1);
    uint8_t newmode = (oldmode) & ~(0x10); // clear the sleep bit
 
    write_8(MODE1, newmode); // send the device to sleep
    wait_us(500);

    //if the restart bit is asserted, restart all pwm channels
    //by writing to it again (taken from datasheet page 15)
    if(newmode & 0x80) write_8(MODE1, newmode | 0x80);
}

 
//NB REQUIRES AUTO-INCREMENT MODE ENABLED
//0 <= pwm_output <= 15
//0 <= (count_on || count_off) <= 4095
void PCA9685::set_pwm_output(int pwm_output, uint16_t count_on, uint16_t count_off)
{
 
    char msg[5];
 
    msg[0] = LED0_ON_L + (4 * pwm_output);
    msg[1] = count_on;
    msg[2] = count_on >> 8;
    msg[3] = count_off;
    msg[4] = count_off >> 8;

    i2c_bus->lock();
    i2c_bus->write(i2c_addr, msg, 5);
    i2c_bus->unlock();
 
}
 
void PCA9685::set_pwm_output_on_0(int pwm_output, uint16_t count_off)
{
    set_pwm_output(pwm_output, 0, count_off);
}
 
 
//NB REQUIRES AUTO-INCREMENT MODE ENABLED
//0 <= pwm_output <= 15
void PCA9685::set_pwm_duty(int pwm_output, float duty_cycle)
{
 
    if (duty_cycle > 1.0) {
        duty_cycle = 1.0;
    }
    if (duty_cycle < 0.0) {
        duty_cycle = 0.0;
    }
 
    uint16_t count_off = (uint16_t) (duty_cycle * 4095);
    uint16_t count_on = 0x0000;
 
    set_pwm_output(pwm_output, count_on, count_off);
 
}
 
 
//NB REQUIRES AUTO-INCREMENT MODE ENABLED
//0 <= pwm_output <= 15
void PCA9685::set_pwm_pw(int pwm_output, float pulse_width_us)
{
 
    float period_us = (1e6/freq);
 
    float duty = pulse_width_us/period_us;
 
    set_pwm_duty(pwm_output, duty);
 
}
 
 
void PCA9685::write_8(uint8_t reg, uint8_t msg)
{
 
    char send[2]; //Store the address and data in an array
    send[0] = reg;
    send[1] = msg;

    i2c_bus->lock();
    i2c_bus->write(i2c_addr, send, 2);
    i2c_bus->unlock();
    
}
 
uint8_t PCA9685::read_8(uint8_t reg)
{
 
    char send[1] ;
    send[0] = reg;

    i2c_bus->lock();
    i2c_bus->write(i2c_addr, send, 1);
    char recieve[1];
    i2c_bus->read(i2c_addr, recieve, 1);
    i2c_bus->unlock();

    return recieve[0];
 
}
 
int PCA9685::convert_pwm_value(float pulse_width_us, float period_us)
{
 
    int result;
    float interim;
 
    interim = ((pulse_width_us / period_us) * 4095); //scale the pulse width to a 12-bit scale
    result = (int) interim; //round the value to the nearest integer
 
    return result;
 
}