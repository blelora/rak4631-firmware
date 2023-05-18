#include "main.h"
#include "system.h"
#include "version.h"

#define LED_GREEN PIN_LED1

void setup()
{
    // Initialize Green LED
    pinMode(LED_GREEN, OUTPUT);
    digitalWrite(LED_GREEN, LOW);

#if DEBUG > 0
    // Initialize Serial for debug output
    Serial.begin(115200);

    time_t serial_timeout = millis();
    // On nRF52840 the USB serial is not available immediately
    while (!Serial)
    {
        if ((millis() - serial_timeout) < 5000)
        {
            delay(100);
            digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
        }
        else
        {
            break;
        }
    }
#endif
    DEBUG_LOG("MAIN", "firmware_version: %s", device_firmware_version);
    DEBUG_LOG("MAIN", "model_code: %s", device_model_code);
    DEBUG_LOG("MAIN", "git_hash: %s", device_git_hash);
    system_init();

    delay(1000);

    digitalWrite(LED_GREEN, HIGH);

    DEBUG_LOG("MAIN", "Setup Complete");

    suspendLoop();
}

void loop()
{
}