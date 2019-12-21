#ifndef PD_NEGOTIATOR_H
#define PD_NEGOTIATOR_H

#include "mbed.h"

/*
 ======================= PDO MESSAGE STRUCTURES ====================
 https://composter.com.ua/documents/Power-Delivery-Specification.pdf

 ###### FOR FIXED SUPPLIES (SOURCES) #######
    BITS 31:30      Fixed Supply Tag (00)
    BIT 29          Dual role power
    BIT 28          USB Suspend supported   //has something to do with suspending USB device operation after a timeout (7ms?)
    BIT 27          Externally Powered
    BIT 26          USB communications capable
    BIT 25          Dual role data
    BIIT 24:22      Reserved
    BITS 21:20      Peak Current (100%, 130%, 150%, 200%) for values 00 -> 11 respectively
    BITS 19:10      Voltage (1-bit = 50mV)
    BITS 9:0        Current (1-bit = 10mA)

###### FOR VARIABLE SUPPLIES (SOURCES) ######
    BITS 31:30      Variable Supply Tag (10)
    BITS 29:20      Max voltage (1-bit = 50mV)
    BITS 19:10      Min voltage (1-bit = 50mV)
    BITS 9:0        Max current (1-bit = 10mA)

    NOTE: this doesn't actually indicate the voltage that will be supplied, but rather just a range!!!!

###### FOR BATTERY SUPPLIES (SOURCES) ######
    BITS 31:30      Battery Supply Tag (01)
    BITS 29:20      Max voltage (1-bit = 50mV)
    BITS 19:10      Min voltage (1-bit = 50mV)
    BITS 9:0        Max power (1-bit = 250mW)

    batteries don't broadcast a fixed voltage, but rather a voltage range that they'll output  
    if powered by a battery source, be careful about not exceeding the power limit
        can do this actively by varying load current
        or just be safe and set the load current to the minimum voltage

//pretty rare PDOs for sink devices, but regardless:
###### FOR FIXED SOURCES (SINK) ######
    BITS 31:30      Fixed Supply Tag (00)
    BIT 29          Dual role power
    BIT 28          Higher Capability
    BIT 27          Externally Powered
    BIT 26          USB communications capable
    BIT 25          Dual role data
    BIIT 24:20      Reserved
    BITS 19:10      Required voltage (1-bit = 50mV)
    BITS 9:0        Operational current (1-bit = 10mA)

###### FOR VARIABLE SUPPLIES (SOURCES) ######
    BITS 31:30      Variable Supply Tag (10)
    BITS 29:20      Max voltage (1-bit = 50mV)
    BITS 19:10      Min voltage (1-bit = 50mV)
    BITS 9:0        Operational current (1-bit = 10mA)

###### FOR BATTERY SUPPLIES (SOURCES) ######
    BITS 31:30      Battery Supply Tag (01)
    BITS 29:20      Max voltage (1-bit = 50mV)
    BITS 19:10      Min voltage (1-bit = 50mV)
    BITS 9:0        Operational power (1-bit = 250mW)

========================= RDO MESSAGE STRUCTURES (SINK ONLY) =========================

##### FIXED AND VARIABLE RDO's #######
    BIT 31          Reserved (set to 0)
    BITS 30:28      PDO object position (001 - 111)
    BIT 27          Giveback flag (set to 0, has something to deal with load limiting)
    BIT 26          Capability Mismatch
    BIT 25          USB Communications Capable
    BIT 24          No USB Suspend (set to 1)
    BIT 23          Unchunked Messages Supported (just set to 0 lol)
    BITS 22:20      Reserved (set to 0)
    BITS 19:10      Operating current (1-bit = 10mA)
    BITS 9:0        Max operating current (1-bit = 10mA)

###### BATTERY RDO's ######
    BIT 31          Reserved (set to 0)
    BITS 30:28      PDO object position (001-111)
    BIT 27          Giveback flag (set to 0)
    BIT 26          Capability mismatch
    BIT 25          USB comm capable
    BIT 24          No USB Suspend (set to 1)
    BIT 23          Unchunked Extended Messages Supported (set to 0)
    BITS 22:20      Reserved (set to 0)
    BITS 19:10      Operating power (1-bit = 250mW)
    BITS 9:0        Operating power (1-bit = 250mW)

=========================== CC_STATUS_REGISTER info =========================
    BIT 5       LOOKING_FOR_CONNECTION
                    1: The device is looking for a connection (either trying to act as a sink/source or DRP toggling)
                    0: The device has FOUND a connection (or simply not looking for a connection)
    BIT 4       CONNECTION RESULT
                    1: The STUSB4500 is acting as a power SINK
                    0: The STUSB4500 is acting as a power SOURCE
    BITS 3:2    CC2 STATE
                    11: SINK MODE - Source is presenting Rp indicating it can source 3.0A
                        SOURCE MODE - Reserved
                    10: SINK MODE - Source is advertising 1.5A
                        SOURCE MODE - This particular CC pin is connected to the SINK
                    01: SINK MODE - Source is advertising 900mA current capability
                        SOURCE MODE - This particular CC pin is connected to Ra (not used in CC communication)
                    00: SINK MODE - Being pulled down by Ra
                        SOURCE MODE - This particular CC pin is OPEN (not connected)
    BITS 1:0    CC1 STATE
                    Same encodings as above 
*/

typedef union {
    uint8_t data;        
    struct {       
        //union goes from LSB -> MSB
        uint8_t cc1 : 2; //cc1 state information
        uint8_t cc2 : 2; //cc2 state information
        uint8_t conn_res: 1; //connection result
        uint8_t looking : 1; //if it's looking for a connection
        uint8_t reserved : 2; //upper 2 bits
    } map;
} CC_Reg_Map;


typedef union {
    uint8_t data;
    struct {
        uint8_t phy_mask : 1;
        uint8_t prt_mask : 1;
        uint8_t reserved : 1;
        uint8_t tcstat_mask : 1;
        uint8_t ccfault_mask : 1;
        uint8_t tcmon_mask : 1;
        uint8_t ccdetect_mask : 1;
        uint8_t hreset_mask : 1;
    } map;
} Alt_S1Reg_Map;

/*
============================ HEADER Structure ============================
https://www.embedded.com/usb-type-c-and-power-delivery-101-power-delivery-protocol/

    BIT 15      EXTENDED FLAG
                    1: Extended length PD message
                    0: Normal length PD message 
    BITS 14:12  NUMBER OF DATA OBJECTS
                    0: Indicates a control header
                    1-7: Indicates a data header
    BITS 11:9   MESSAGE ID
                    some rolling counter value
                    not sure if STUSB4500 handles it
    BIT 8       POWER PORT ROLE
                    0: SINK
                    1: SOURCE
    BIT 7:6     SPEC REVISION
                    00: Rev 1.0
                    01: Rev 2.0
                    10: Rev 3.0
    BIT 5       DATA ROLE
                    0: UFP
                    1: DFP
    BIT 4:0     MESSAGE TYPE
                for control messages:
                    00001: GOODCRC
                    00010: GOTO_MIN
                    00011: ACCEPT
                    00100: REJECT
                    00101: PING (sink can ignore)
                    00110: PS_READY
                    00111: GET_SOURCE_CAPABILITY
                    01000: GET_SINK_CAPABILITY
                    01001: DATA_ROLE_SWAP
                    01010: POWER_ROLE_SWAP
                    01011: VCONN_SWAP
                    01100: WAIT
                    01101: SOFT_RESET
                    10000: NOT_SUPPORTED
                    10001: GET_SOURCE_CAPABILITY_EXTENDED
                    10010: GET_STATUS
                    10011: FAST_ROLE_SWAP
                for data messages:
                    00001: SOURCE_CAPABILITIES
                    00010: REQUEST
                    00011: BUILT_IN_SELF_TEST
                    00100: SINK_CAPABILITIES
                    00101: BATTERY_STATUS
                    00110: ALERT
                    01111: VENDOR_DEFINED

*/

/*
void init()

Following the INIT procedure in the sample code

to initialize, we'll first read the device ID register (0x2F apparently)
if the result isn't 0x21 or 0x25 something's fucked possibly

unmask the 
    - PRT_STATUS_AL_MASK (bit 1)
    - PD_TYPEC_STATUS_AL_MASK (bit 3)
    - CC_DETECTION_STATUS_AL_MASK (bit 6) typo in datasheet
    - HARD_RESET_AL_MASK (bit 7)
in the ALERT_STATUS_1_MASK register (0x0C)

clear the ALERT_STATUS_1 register (0x0B) by reading from it

update local copies of the
    - PORT_STATUS_1 register (0x0E)
    - CC_STATUS register (0x11)
    - CC_HW_FAULT_STATUS register (0x13)
    - TYPE_C_MONITORING_STATUS register (0x10)

init function reads 12 bytes for some reason starting at 0x0B so try that if reading these doesn't work
*/

/*
void hardware_reset()
{
    write 1 to the reset pin
    wait 15 ms
    write 0 to the reset pin
    wait 15 ms
    void init();
} */


/*
void software_reset() //register based reset
{

    write '1' to the LSB of the RESET_CTRL_REGISTER (0x23)
    clear the ALERT_STATUS_1 register (0x0B) by reading from it
        sample code reads 12 bytes so uhhhhhh do that if it isn't working???
    wait like 25-30 ms
    write '0' to the LSB of the RESET_CTRL_REGISTER (0x23) 
}
*/


/*
void send_soft_reset() {
    use this to reset the contract between the device and source
    we don't lose VBUS doing this

    send 0x0D to the TX_HEADER register (0x51, not in datasheet)
    send 0x26 to the STUSB_GEN1S_CMD_CTRL register (0x1A, not in datasheet)
    
    reset PDO_FROM_SRC_Valid array/struct if you want
}
*/


/*
void read_sink_pdo() {
    I think this has to deal with the number of _SINK_ PDO's programmed into the device 
    read the number of PDO's available by reading from DPM_PDO_NUMB (0x70)
        there can be any number of them between 0 and 3
    depending on how many there are, read the PDO messages
        start at DPM_SNK_PDO1 (0x85)
        each message is 4 bytes
        read upto 3 PDO's
} 
*/

/*
void read_RDO(*struct) {
    reads back what we're tryna request from the PD source i.e. the RDO
    located at 0x91 - 0x94
    formatted just like a normal RDO i think
}
 */


/*
void update_PDO() {
    update the sink's PDO registers (overwrites whatever is currently in RAM)
    we can't change PDO 1 (voltage at least); if we do, write to addres 0x85
        - make sure to explicitly set this to 5V
    if we want to change PDO2, write to address 0x89
    and for PDO3, address 0x8D
 */


/*
void update_active_PDOs() {
    write to the DPM_PDO_NUMB register (0x70)
    number basically says which PDOs are active
        if 3, then all are active,
        if 2, only PDO 1 and 2 are active
        if 1, only PDO 1 is active
}
 */


/*
void type_c_info() {
    the port is only a non-pd port if it hasn't sent any PD relevant info after 500ms
    Read from the CC_STATUS register (0x11)
    see the tippy top for human-understandable documentation on what the different values mean
}
 */



/*
void request_pdo_number(uint8_t pdo_index) {
    the way this function works is it looks at the PDO object received by the STUSB4500 at the particular index
    and just copies it into the PDO2 register 

    feels kinda dumb but apparently it works
}
 */


/*
void find_high_power_PDO(min_voltage, min_current, min_power) {
    send a get_src_pdo message via the TX_HEADER and the STUSB_GEN1S_CMD_CTRL reg
    wait until we get a response (clear a flag before sending and wait till its asserted by ISR)
    compare all PDOs to our requirements
        if we get a match, pop our selected PDO into PDO2 and set active PDOs to 2
        else just set active PDOs to 1 (default 5v) and just return something
    send a soft reset command to the host
    wait until we get an accept response(clear a flag before sending and wait till its asserted by ISR)
    return appropriately    
}
 */



/*
void alert_isr() { //call when alert gets driven low
    read the ALERT_STATUS_1 (0x0B) and the ALERT_STATUS_1_MASK (0x0C) registers
        only pay attention to the bits in the ALERT_STATUS_REGISTER that we unmasked (set to 0 in the MASK register)
        just a reminder, the ALERT_STATUS_1 is standard logic (1 means asserted)
    
    hardware reset (7th bit) - note that if you care about it
    
    if the 6th bit (CC_DETECTION_STATUS aka PORT_STATUS) is high
        read the PORT_STATUS_0 (0x0D) and PORT_STATUS_1 (0x0E) registers
        if the ATTACH bit (0) of PORT_STATUS_1 is high
            log the timestamp (if you want, might be worth considering)
            read the CC_STATUS register (0x11) - tells you info about what the CC pins and the state machine is doing
        else
            the device is Detached
            maybe update an internal state variable

    if the 5th bit (TYPEC_MONITORING_STATUS) is high
        read the TYPEC_MONITORING_STATUS_0 (0x0F) and TYPEC_MONITORING_STATUS_1 (0x10) registers
            tells us information about the state of vbus (whether it's valid or whether it's 0)

    //read the CC status registers just because???
    
    if the 4th bit (CC_HW_FAULT_STATUS bit) is asserted 
        read the CC_HW_FAULT_STATUS_0 (0x12) and CC_HW_FAULT_STATUS_1 (0x13) registers
            tells us information about CC line pull ups/overvoltage conditions
             ... and VBUS and VSRC discharge faults
        store this fault information if you want 
    

    if the 1st bit (PRT_STATUS bit) is asserted
        read the PRT_STATUS register (0x16)
        if the MSG_RECEIVED bit is asserted (bit 2)
            read the RX_HEADER registers (2 regs starting at 0x31)
            if the message is a data message (see tippy top):
                get the number of data objects from the RX_HEADER (see top as well)
                    - compare this to the RX_BYTE_COUNT reg >> 2 (0x30) if you wanna verify
                if it's a SOURCE_CAPABILITIES message (0x01)
                    real quick read them out from their respective registers
                    - start at 0x33, each 32 bits long
                    - end at 0x4E
                basically ignore all other message types?
                    - Request (0x02)
                    - BIST (0x03)
                    - Sink Capabilities (0x04)
                    - Battery Status (0x05)
                    - Alert (0x06)
                    - Vendor Defined messages (0x0F)
            else it's a control message
                don't reeeeeallyyyyy need to pay attention to these, BUT MAYBE:
                - GotoMin (0x02)
                - Accept (0x03)
                - Reject (0x04)
                - PS_RDY (0x06)
*/

 //PE_FSM register is in 0x29 or 0x15 which might be a cool debugging/info tool

#endif
