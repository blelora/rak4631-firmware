#include "main.h"

#define LED_GREEN PIN_LED1
#define LED_BLUE PIN_LED2

void setup()
{
    // Initialize the built in Green LED
    pinMode(LED_GREEN, OUTPUT);
    digitalWrite(LED_GREEN, LOW);

#if MY_DEBUG > 0
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

    digitalWrite(LED_GREEN, HIGH);

    MYLOG("APP", "=====================================");
    MYLOG("APP", "RAK4631 Firmware Starting");
    MYLOG("APP", "=====================================");
}

void loop()
{
    MYLOG("APP", "============= Main Loop =============");
    delay(1000);
}