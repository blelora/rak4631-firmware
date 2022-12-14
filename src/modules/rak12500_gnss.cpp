#include "modules/rak12500_gnss.h"

SFE_UBLOX_GNSS g_myGNSS;
bool is_rak12500_init = false;

long g_lastTime = 0; // Simple local timer. Limits amount of I2C traffic to u-blox module.

bool rak12500_init(void)
{
    int baud[7] = {9600, 14400, 19200, 38400, 56000, 57600, 115200};

    DEBUG_LOG("RAK12500-GNSS", "INIT");
    pinMode(WB_IO2, OUTPUT);
    digitalWrite(WB_IO2, 0);
    delay(1000);
    digitalWrite(WB_IO2, 1);
    delay(1000);

    for (int i = 0; i < sizeof(baud) / sizeof(int); i++)
    {
        Serial1.begin(baud[i]);
        while (!Serial1)
            ; // Wait for user to open terminal
        if (g_myGNSS.begin(Serial1) == true)
        {
            is_rak12500_init = true;
            // Serial.printf("GNSS baund rate: %d \n", baud[i]); // GNSS baund rate
            break;
        }
        Serial1.end();
        delay(200);
    }

    if (!is_rak12500_init)
    {
        DEBUG_LOG("RAK12500-GNSS", "INIT FAILED");
    }
    else
    {
        g_myGNSS.setUART1Output(COM_TYPE_UBX); // Set the UART port to output UBX only
        g_myGNSS.setI2COutput(COM_TYPE_UBX);   // Set the I2C port to output UBX only (turn off NMEA noise)
        g_myGNSS.saveConfiguration();          // Save the current settings to flash and BBR
        DEBUG_LOG("RAK12500-GNSS", "INIT SUCCESS");

        // This isn't working as expected yet
        // if (g_myGNSS.powerSaveMode()) // Defaults to true
        // {
        //     DEBUG_LOG("RAK12500-GNSS", "Power Save Mode enabled.");
        // }
        // else
        // {
        //     DEBUG_LOG("RAK12500-GNSS", "*** Power Save Mode FAILED ***");
        // }

        // Read and print the new low power mode
        uint8_t lowPowerMode = g_myGNSS.getPowerSaveMode();
        if (lowPowerMode == 255)
        {
            DEBUG_LOG("RAK12500-GNSS", "*** getPowerSaveMode FAILED ***");
        }
        else
        {
            DEBUG_LOG("RAK12500-GNSS", "The low power mode is: ");
            Serial.print(lowPowerMode);
            if (lowPowerMode == 0)
            {
                DEBUG_LOG("RAK12500-GNSS", " (Continuous)");
            }
            else if (lowPowerMode == 1)
            {
                DEBUG_LOG("RAK12500-GNSS", " (Power Save)");
            }
            else if (lowPowerMode == 4)
            {
                DEBUG_LOG("RAK12500-GNSS", " (Continuous)");
            }
            else
            {
                DEBUG_LOG("RAK12500-GNSS", " (Unknown!)");
            }
        }
    }
    return is_rak12500_init;
}