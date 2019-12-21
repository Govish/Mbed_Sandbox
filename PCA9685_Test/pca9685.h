#ifndef PCA9685_LIBRARY_H
#define PCA9685_LIBRARY_H
 
#include "mbed.h" 
 
class PCA9685 {
    
    public:
        PCA9685(uint8_t i2c_addr, I2C &i2c_object, float frequency);
        void init(void);
        void set_pwm_output(int pwm_output, uint16_t count_on, uint16_t count_off);
        void set_pwm_output_on_0(int pwm_output, uint16_t count_off);
        void set_pwm_duty(int pwm_output, float duty_cycle);
        void set_pwm_pw(int pwm_output, float pulse_width_us);
        void sleep();
        void wake();
        
    private: 
        void write_8(uint8_t reg, uint8_t msg);
        uint8_t read_8(uint8_t reg);
        void set_prescale(uint8_t prescale);
        int convert_pwm_value(float pulse_width_us, float period_us);
    
    private:
        int i2c_addr;
        float freq;
        I2C *i2c_bus;
};        
 
#endif
            