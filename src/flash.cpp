#include "main.h"

#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

using namespace Adafruit_LittleFS_Namespace;

#define LORAWAN_CREDENTIALS_FILENAME    "/credentials.txt"
#define LORAWAN_CREDENTIALS_CONTENTS    "LoRaWAN Credentials File"

s_lorawan_credentials g_flash_content;

File lorawan_credentials_file(InternalFS);

bool init_flash_done = false;

/**
 * @brief Initialize access to nRF52 internal file system
 * 
 */
void init_flash(void)
{
	if (init_flash_done)
	{
		return;
	}

	// Initialize Internal File System
	InternalFS.begin();

	// Check if file exists
	lorawan_credentials_file.open(LORAWAN_CREDENTIALS_FILENAME, FILE_O_READ);
	if (!lorawan_credentials_file)
	{
		DEBUG_LOG("FLASH", "Credentials File doesn't exist, force format");
		delay(100);
		flash_reset();
		lorawan_credentials_file.open(LORAWAN_CREDENTIALS_FILENAME, FILE_O_READ);
	}
	uint8_t markers[2] = {0};
	lorawan_credentials_file.read(markers, 2);
	if ((markers[0] == 0xAA) && (markers[1] == LORAWAN_CREDENTIALS_MARKER))
	{
		// Found new structure
		lorawan_credentials_file.close();
		lorawan_credentials_file.open(LORAWAN_CREDENTIALS_FILENAME, FILE_O_READ);
		lorawan_credentials_file.read((uint8_t *)&g_lorawan_credentials, sizeof(s_lorawan_credentials));
		lorawan_credentials_file.close();
		// Check if it is LPWAN settings
		if ((g_lorawan_credentials.valid_mark_1 != 0xAA) || (g_lorawan_credentials.valid_mark_2 != LORAWAN_CREDENTIALS_MARKER))
		{
			// Data is not valid, reset to defaults
			DEBUG_LOG("FLASH", "Invalid data set, deleting and restart node");
			InternalFS.format();
			delay(1000);
			sd_nvic_SystemReset();
		}
		log_credentials();
		init_flash_done = true;
	}
}

/**
 * @brief Save changed settings if required
 * 
 * @return boolean 
 * 			result of saving
 */
boolean save_credentials(void)
{
	bool result = true;
	// Read saved content
	lorawan_credentials_file.open(LORAWAN_CREDENTIALS_FILENAME, FILE_O_READ);
	if (!lorawan_credentials_file)
	{
		DEBUG_LOG("FLASH", "File doesn't exist, force format");
		delay(100);
		flash_reset();
		lorawan_credentials_file.open(LORAWAN_CREDENTIALS_FILENAME, FILE_O_READ);
	}
	lorawan_credentials_file.read((uint8_t *)&g_flash_content, sizeof(s_lorawan_credentials));
	lorawan_credentials_file.close();
	if (memcmp((void *)&g_flash_content, (void *)&g_lorawan_credentials, sizeof(s_lorawan_credentials)) != 0)
	{
		DEBUG_LOG("FLASH", "Flash content changed, writing new data");
		delay(100);

		InternalFS.remove(LORAWAN_CREDENTIALS_FILENAME);

		if (lorawan_credentials_file.open(LORAWAN_CREDENTIALS_FILENAME, FILE_O_WRITE))
		{
			lorawan_credentials_file.write((uint8_t *)&g_lorawan_credentials, sizeof(s_lorawan_credentials));
			lorawan_credentials_file.flush();
		}
		else
		{
			result = false;
		}
		lorawan_credentials_file.close();
	}
	log_credentials();
	return result;
}

/**
 * @brief Reset content of the filesystem
 * 
 */
void flash_reset(void)
{
	InternalFS.format();
	if (lorawan_credentials_file.open(LORAWAN_CREDENTIALS_FILENAME, FILE_O_WRITE))
	{
		s_lorawan_credentials default_credentials;
		lorawan_credentials_file.write((uint8_t *)&default_credentials, sizeof(s_lorawan_credentials));
		lorawan_credentials_file.flush();
		lorawan_credentials_file.close();
	}
}

/**
 * @brief Printout of all settings
 * 
 */
void log_credentials(void)
{
	DEBUG_LOG("FLASH", "Saved settings:");
	DEBUG_LOG("FLASH", "000 Marks: %02X %02X", g_lorawan_credentials.valid_mark_1, g_lorawan_credentials.valid_mark_2);
	DEBUG_LOG("FLASH", "002 Dev EUI %02X%02X%02X%02X%02X%02X%02X%02X", g_lorawan_credentials.node_device_eui[0], g_lorawan_credentials.node_device_eui[1],
			g_lorawan_credentials.node_device_eui[2], g_lorawan_credentials.node_device_eui[3],
			g_lorawan_credentials.node_device_eui[4], g_lorawan_credentials.node_device_eui[5],
			g_lorawan_credentials.node_device_eui[6], g_lorawan_credentials.node_device_eui[7]);
	DEBUG_LOG("FLASH", "010 App EUI %02X%02X%02X%02X%02X%02X%02X%02X", g_lorawan_credentials.node_app_eui[0], g_lorawan_credentials.node_app_eui[1],
			g_lorawan_credentials.node_app_eui[2], g_lorawan_credentials.node_app_eui[3],
			g_lorawan_credentials.node_app_eui[4], g_lorawan_credentials.node_app_eui[5],
			g_lorawan_credentials.node_app_eui[6], g_lorawan_credentials.node_app_eui[7]);
	DEBUG_LOG("FLASH", "018 App Key %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
			g_lorawan_credentials.node_app_key[0], g_lorawan_credentials.node_app_key[1],
			g_lorawan_credentials.node_app_key[2], g_lorawan_credentials.node_app_key[3],
			g_lorawan_credentials.node_app_key[4], g_lorawan_credentials.node_app_key[5],
			g_lorawan_credentials.node_app_key[6], g_lorawan_credentials.node_app_key[7],
			g_lorawan_credentials.node_app_key[8], g_lorawan_credentials.node_app_key[9],
			g_lorawan_credentials.node_app_key[10], g_lorawan_credentials.node_app_key[11],
			g_lorawan_credentials.node_app_key[12], g_lorawan_credentials.node_app_key[13],
			g_lorawan_credentials.node_app_key[14], g_lorawan_credentials.node_app_key[15]);
	DEBUG_LOG("FLASH", "034 Dev Addr %08lX", g_lorawan_credentials.node_dev_addr);
	DEBUG_LOG("FLASH", "038 NWS Key %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
			g_lorawan_credentials.node_nws_key[0], g_lorawan_credentials.node_nws_key[1],
			g_lorawan_credentials.node_nws_key[2], g_lorawan_credentials.node_nws_key[3],
			g_lorawan_credentials.node_nws_key[4], g_lorawan_credentials.node_nws_key[5],
			g_lorawan_credentials.node_nws_key[6], g_lorawan_credentials.node_nws_key[7],
			g_lorawan_credentials.node_nws_key[8], g_lorawan_credentials.node_nws_key[9],
			g_lorawan_credentials.node_nws_key[10], g_lorawan_credentials.node_nws_key[11],
			g_lorawan_credentials.node_nws_key[12], g_lorawan_credentials.node_nws_key[13],
			g_lorawan_credentials.node_nws_key[14], g_lorawan_credentials.node_nws_key[15]);
	DEBUG_LOG("FLASH", "054 Apps Key %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
			g_lorawan_credentials.node_apps_key[0], g_lorawan_credentials.node_apps_key[1],
			g_lorawan_credentials.node_apps_key[2], g_lorawan_credentials.node_apps_key[3],
			g_lorawan_credentials.node_apps_key[4], g_lorawan_credentials.node_apps_key[5],
			g_lorawan_credentials.node_apps_key[6], g_lorawan_credentials.node_apps_key[7],
			g_lorawan_credentials.node_apps_key[8], g_lorawan_credentials.node_apps_key[9],
			g_lorawan_credentials.node_apps_key[10], g_lorawan_credentials.node_apps_key[11],
			g_lorawan_credentials.node_apps_key[12], g_lorawan_credentials.node_apps_key[13],
			g_lorawan_credentials.node_apps_key[14], g_lorawan_credentials.node_apps_key[15]);
}