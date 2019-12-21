#include "Charger.h"

//so 1.23V corresponds to 5A
#define CURRENT_SCALAR 0.0745 //value corresponding to 1A charge current
#define CURRENT_HEADROOM 0.90 //give us 10% headroom
#define CHARGE_IMAX 5.0       //maximum charging current

Charger::Charger(PinName ilim_pin, PinName run_pin, PinName en_pin) :
    ilim(ilim_pin),
    run(run_pin),
    chg_en(en_pin)
{}

//call at pack boot
bool Charger::init() {
    bool r, c;
    r = run.is_connected();
    c = chg_en.is_connected();

    return r && c;
}

//call before deep power down
void Charger::deinit() {

}

//shutdown charging process
void Charger::shutdown() {    
}

//begin charging process with specified input current
void Charger::start_charge(float current_a) {
    //constrain current from 0 to 5A
    current_a = current_a < 0 ? 0 : (current_a > CHARGE_IMAX ? CHARGE_IMAX : current_a); 
    ilim.write(current_a * CURRENT_SCALAR * CURRENT_HEADROOM);
}

void manage_charge();   //make sure the entire charging process is going to plan