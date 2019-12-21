#ifndef CHARGER_H
#define CHARGER_H

#include "mbed.h"

class Charger {

    public:
        Charger(PinName ilim_pin, PinName run_pin, PinName en_pin);
        bool init();    //call at pack boot, return if init was successful
        void deinit();  //call before deep power down

        void shutdown();    //shutdown charging process
        void start_charge(float current_a); //begin charging process with specified input current

    private:
        AnalogOut ilim;     //input current control DAC
        DigitalOut run;     //pin to enable converter
        DigitalOut chg_en;  //"backward" NFET on AFE (prevents charger output from charging battery)

        //output enable should be connected to a hardware comparator
        //DigitalOut out_en;  //"forward" PFET (prevents battery output from getting to charger)

        void manage_charge();   //make sure the entire charging process is going to plan

};

#endif