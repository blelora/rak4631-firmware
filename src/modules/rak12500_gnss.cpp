#include "modules/rak12500_gnss.h"

SFE_UBLOX_GNSS g_myGNSS;
bool is_rak12500_init = false;

long g_lastTime = 0; // Simple local timer. Limits amount of I2C traffic to u-blox module.

bool rak12500_init(void)
{
    int baud[7]={9600,14400,19200,38400,56000,57600,115200};
    
    DEBUG_LOG("RAK12500-GNSS", "INIT");
    pinMode(WB_IO2, OUTPUT);
    digitalWrite(WB_IO2, 0);
    delay(1000);
    digitalWrite(WB_IO2, 1);
    delay(1000);

    for (int i = 0; i < sizeof(baud) / sizeof(int); i++)
    {
        Serial2.begin(baud[i]);
        while (!Serial2)
            ; // Wait for user to open terminal
        if (g_myGNSS.begin(Serial2) == true)
        {
            is_rak12500_init = true;
            // Serial.printf("GNSS baund rate: %d \n", baud[i]); // GNSS baund rate
            break;
        }
        Serial2.end();
        delay(200);
    }

    if(!is_rak12500_init)
    {
        DEBUG_LOG("RAK12500-GNSS", "INIT FAILED");
    }
    else
    {
        g_myGNSS.setUART1Output(COM_TYPE_UBX); // Set the UART port to output UBX only
        g_myGNSS.setI2COutput(COM_TYPE_UBX);   // Set the I2C port to output UBX only (turn off NMEA noise)
        g_myGNSS.saveConfiguration();          // Save the current settings to flash and BBR
        DEBUG_LOG("RAK12500-GNSS", "INIT SUCCESS");
    }
    return is_rak12500_init;
}