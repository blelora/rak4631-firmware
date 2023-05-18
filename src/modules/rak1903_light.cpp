#include "modules/rak1903_light.h"

#define OPT3001_ADDRESS 0x44

ClosedCube_OPT3001 opt3001;
bool is_rak1903_init = false;

void printError(String text, OPT3001_ErrorCode error)
{
    Serial.print(text);
    Serial.print(": [ERROR] Code #");
    Serial.println(error);
}

void printResult(String text, OPT3001 result)
{
    if (result.error == NO_ERROR)
    {
        Serial.print(text);
        Serial.print(": ");
        Serial.print(result.lux);
        Serial.println(" lux");
    }
    else
    {
        printError(text, result.error);
    }
}

bool rak1903_init(void)
{
    if (opt3001.begin(OPT3001_ADDRESS) != NO_ERROR) // Whenever data is received the associated checksum is calculated and verified so you can be sure the data is true
    {                                               // The checksum pass indicators are: passIDcrc, passRHcrc, and passTcrc for the ID, RH, and T readings respectively
        DEBUG_LOG("RAK1903-LIGHT", "INIT FAILED");
    }
    else
    {
        DEBUG_LOG("RAK1903-LIGHT", "INIT SUCCESS");
        delay(100);
        is_rak1903_init = true;
        Serial.print("\nOPT3001 Manufacturer ID");
        Serial.println(opt3001.readManufacturerID());
        Serial.print("OPT3001 Device ID");
        Serial.println(opt3001.readDeviceID());

        delay(100);

        OPT3001_Config newConfig;

        newConfig.RangeNumber = B1100;
        newConfig.ConvertionTime = B0;
        newConfig.Latch = B1;
        newConfig.ModeOfConversionOperation = B11;

        OPT3001_ErrorCode errorConfig = opt3001.writeConfig(newConfig);
        if (errorConfig != NO_ERROR)
            printError("OPT3001 configuration", errorConfig);
        else
        {
            OPT3001_Config sensorConfig = opt3001.readConfig();
            Serial.println("OPT3001 Current Config:");
            Serial.println("------------------------------");

            Serial.print("Conversion ready (R):");
            Serial.println(sensorConfig.ConversionReady, HEX);

            Serial.print("Conversion time (R/W):");
            Serial.println(sensorConfig.ConvertionTime, HEX);

            Serial.print("Fault count field (R/W):");
            Serial.println(sensorConfig.FaultCount, HEX);

            Serial.print("Flag high field (R-only):");
            Serial.println(sensorConfig.FlagHigh, HEX);

            Serial.print("Flag low field (R-only):");
            Serial.println(sensorConfig.FlagLow, HEX);

            Serial.print("Latch field (R/W):");
            Serial.println(sensorConfig.Latch, HEX);

            Serial.print("Mask exponent field (R/W):");
            Serial.println(sensorConfig.MaskExponent, HEX);

            Serial.print("Mode of conversion operation (R/W):");
            Serial.println(sensorConfig.ModeOfConversionOperation, HEX);

            Serial.print("Polarity field (R/W):");
            Serial.println(sensorConfig.Polarity, HEX);

            Serial.print("Overflow flag (R-only):");
            Serial.println(sensorConfig.OverflowFlag, HEX);

            Serial.print("Range number (R/W):");
            Serial.println(sensorConfig.RangeNumber, HEX);

            Serial.println("------------------------------");
        }

        printResult("High-Limit", opt3001.readHighLimit());
        printResult("Low-Limit", opt3001.readLowLimit());
        Serial.println("----");

    }
    return is_rak1903_init;
}
