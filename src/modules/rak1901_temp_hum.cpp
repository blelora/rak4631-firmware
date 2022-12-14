#include "modules/rak1901_temp_hum.h"

SHTC3 mySHTC3;
bool is_rak1901_init = false;

bool rak1901_init(void)
{
    mySHTC3.begin();
    if(!mySHTC3.passIDcrc)                                       // Whenever data is received the associated checksum is calculated and verified so you can be sure the data is true
    {                                                           // The checksum pass indicators are: passIDcrc, passRHcrc, and passTcrc for the ID, RH, and T readings respectively
        DEBUG_LOG("RAK1901-TEMP-HUM", "INIT FAILED");
    }
    else
    {
        DEBUG_LOG("RAK1901-TEMP-HUM", "INIT SUCCESS");
        is_rak1901_init = true;
        // Set the device to use the 24hour format (default) instead of the 12 hour format
        Serial.print("ID Passed Checksum. ");
        Serial.print("Device ID: 0b"); 
        Serial.print(mySHTC3.ID, BIN);
    }
    return is_rak1901_init;
}

