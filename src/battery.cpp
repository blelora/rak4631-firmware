#include "battery.h"

#define BATTERY_STACK_SIZE (256 * 4)
#define PIN_VBAT WB_A0

uint32_t vbat_pin = PIN_VBAT;

#define VBAT_MV_PER_LSB (0.73242188F) // 3.0V ADC range and 12 - bit ADC resolution = 3000mV / 4096
#define VBAT_DIVIDER_COMP (1.73)      // Compensation factor for the VBAT divider, depend on the board

#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)

void battery_task(void *arg);
void init_vbat(void);
uint8_t mvToPercent(float mvolts);

void battery_init()
{
    DEBUG_LOG("BATTERY", "INIT");
    xTaskCreate(battery_task, "BATTERY", BATTERY_STACK_SIZE, NULL, TASK_PRIO_LOW, NULL);
}

void battery_task(void *arg)
{
    DEBUG_LOG("BATTERY", "TASK START");

    init_vbat();

    while (1)
    {
        uint8_t vbat_per = mvToPercent(readVBAT());
        gnss_location.battery = vbat_per;
        ble_bas.write(mvToPercent(readVBAT()));
        ble_bas.notify(vbat_per);
        DEBUG_LOG("BATTERY", "Battery Percentage %d, USB Mounted Status: %d",vbat_per, TinyUSBDevice.mounted());

        vTaskDelay(30000);
    }
}

void init_vbat(void)
{
    // Set the analog reference to 3.0V (default = 3.6V)
    analogReference(AR_INTERNAL_3_0);

    // Set the resolution to 12-bit (0..4095)
    analogReadResolution(12); // Can be 8, 10, 12 or 14

    // Set the sampling time to 10us
	analogSampleTime(10);

    vTaskDelay(1);

    readVBAT();
}

float readVBAT(void)
{
    float raw;

    // Get the raw 12-bit, 0..3000mV ADC value
    raw = analogRead(vbat_pin);

    return raw * REAL_VBAT_MV_PER_LSB;
}

/**
 * @brief Convert from raw mv to percentage
 * @param mvolts
 *    RAW Battery Voltage
 */
uint8_t mvToPercent(float mvolts)
{
    if (TinyUSBDevice.mounted())
        return 111;

	if (mvolts < 3300)
		return 0;

	if (mvolts < 3600)
	{
		mvolts -= 3300;
		return mvolts / 30;
	}

	if (mvolts > 4200)
	{
		return 100;
	}

	mvolts -= 3600;
	return 10 + (mvolts * 0.15F); // thats mvolts /6.66666666
}

/**
 * @brief get LoRaWan Battery value
 * @param mvolts
 *    Raw Battery Voltage
 */
uint8_t mvToLoRaWanBattVal(float mvolts)
{
	uint16_t read_val = 0;
	for (int i = 0; i < 10; i++)
	{
		read_val += readVBAT();
	}
	return (mvToPercent(read_val / 10) * 2.54);
}