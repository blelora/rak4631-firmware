#include "main.h"

#define LED_GREEN PIN_LED1
#define LED_BLUE PIN_LED2

void setup()
{
    // Initialize the built in Green LED
    pinMode(LED_GREEN, OUTPUT);
    digitalWrite(LED_GREEN, LOW);
}

void loop()
{
    // Main loop running indicator
    digitalToggle(LED_GREEN);
    delay(200);
}