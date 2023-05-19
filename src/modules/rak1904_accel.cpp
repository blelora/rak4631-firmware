#include "modules/rak1904_accel.h"

Adafruit_LIS3DH lis = Adafruit_LIS3DH();
bool is_rak1904_init = false;

bool rak1904_init(void)
{
    if(! lis.begin(0x18))                                       // Whenever data is received the associated checksum is calculated and verified so you can be sure the data is true
    {                                                           // The checksum pass indicators are: passIDcrc, passRHcrc, and passTcrc for the ID, RH, and T readings respectively
        DEBUG_LOG("RAK1904-ACCEL", "INIT FAILED");
    }
    else
    {
        DEBUG_LOG("RAK1904-ACCEL", "INIT SUCCESS");
        is_rak1904_init = true;
        Serial.println("LIS3DH found!");

        delay(100);

        lis.setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!
        Serial.print("Range = "); Serial.print(2 << lis.getRange());
        Serial.println("G");

        // lis.setOrientation();
        delay(100);
    }
    return is_rak1904_init;
}

