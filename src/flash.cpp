#include "main.h"

#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

using namespace Adafruit_LittleFS_Namespace;

#define LORAWAN_CREDENTIALS_FILENAME "/credentials.txt"
#define LORAWAN_CREDENTIALS_CONTENTS "LoRaWAN Credentials File"
#define LORAWAN_SETTINGS_FILENAME "/settings.txt"
#define LORAWAN_SETTINGS_CONTENTS "LoRaWAN Settings File"

s_lorawan_credentials g_lorawan_credentials_flash_content;
s_lorawan_credentials g_lorawan_settings_flash_content;

File lorawan_credentials_file(InternalFS);
File lorawan_settings_file(InternalFS);

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

	// Check if credentials file exists
	lorawan_credentials_file.open(LORAWAN_CREDENTIALS_FILENAME, FILE_O_READ);
	if (!lorawan_credentials_file)
	{
		DEBUG_LOG("FLASH", "Credentials File doesn't exist, force format");
		delay(100);
		flash_default_credentials();
		lorawan_credentials_file.open(LORAWAN_CREDENTIALS_FILENAME, FILE_O_READ);
	}

	// Found new structure
	lorawan_credentials_file.close();
	lorawan_credentials_file.open(LORAWAN_CREDENTIALS_FILENAME, FILE_O_READ);
	lorawan_credentials_file.read((uint8_t *)&g_lorawan_credentials, sizeof(s_lorawan_credentials));
	lorawan_credentials_file.close();

	log_credentials();

	// Check if settings file exists
	lorawan_settings_file.open(LORAWAN_SETTINGS_FILENAME, FILE_O_READ);
	if (!lorawan_settings_file)
	{
		DEBUG_LOG("FLASH", "Settings File doesn't exist, force format");
		delay(100);
		flash_default_settings();
		lorawan_settings_file.open(LORAWAN_SETTINGS_FILENAME, FILE_O_READ);
	}

	// Found new structure
	lorawan_settings_file.close();
	lorawan_settings_file.open(LORAWAN_SETTINGS_FILENAME, FILE_O_READ);
	lorawan_settings_file.read((uint8_t *)&g_lorawan_settings, sizeof(s_lorawan_settings));
	lorawan_settings_file.close();

	log_settings();
	init_flash_done = true;
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
		DEBUG_LOG("FLASH", "Credentials File doesn't exist, force format");
		delay(100);
		flash_default_credentials();
		lorawan_credentials_file.open(LORAWAN_CREDENTIALS_FILENAME, FILE_O_READ);
	}
	lorawan_credentials_file.read((uint8_t *)&g_lorawan_credentials_flash_content, sizeof(s_lorawan_credentials));
	lorawan_credentials_file.close();
	if (memcmp((void *)&g_lorawan_credentials_flash_content, (void *)&g_lorawan_credentials, sizeof(s_lorawan_credentials)) != 0)
	{
		DEBUG_LOG("FLASH", "Credentials Flash content changed, writing new data");
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

boolean save_settings(void)
{
	bool result = true;
	// Read saved content
	lorawan_settings_file.open(LORAWAN_SETTINGS_FILENAME, FILE_O_READ);
	if (!lorawan_settings_file)
	{
		DEBUG_LOG("FLASH", "Settings File doesn't exist, force format");
		delay(100);
		flash_default_settings();
		lorawan_settings_file.open(LORAWAN_SETTINGS_FILENAME, FILE_O_READ);
	}
	lorawan_settings_file.read((uint8_t *)&g_lorawan_settings_flash_content, sizeof(s_lorawan_settings));
	lorawan_settings_file.close();
	if (memcmp((void *)&g_lorawan_settings_flash_content, (void *)&g_lorawan_settings, sizeof(s_lorawan_settings)) != 0)
	{
		DEBUG_LOG("FLASH", "Settings Flash content changed, writing new data");
		delay(100);

		InternalFS.remove(LORAWAN_SETTINGS_FILENAME);

		if (lorawan_settings_file.open(LORAWAN_SETTINGS_FILENAME, FILE_O_WRITE))
		{
			lorawan_settings_file.write((uint8_t *)&g_lorawan_settings, sizeof(s_lorawan_settings));
			lorawan_settings_file.flush();
		}
		else
		{
			result = false;
		}
		lorawan_settings_file.close();
	}
	log_settings();
	return result;
}

/**
 * @brief Reset content of the filesystem
 *
 */
void flash_reset(void)
{
	InternalFS.format();
}

/**
 * @brief Reset content of the filesystem
 *
 */
void flash_default_credentials(void)
{
	if (lorawan_credentials_file.open(LORAWAN_CREDENTIALS_FILENAME, FILE_O_WRITE))
	{
		s_lorawan_credentials default_credentials;
		lorawan_credentials_file.write((uint8_t *)&default_credentials, sizeof(s_lorawan_credentials));
		lorawan_credentials_file.flush();
		lorawan_credentials_file.close();
	}
}

/**
 * @brief Reset content of the filesystem
 *
 */
void flash_default_settings(void)
{
	if (lorawan_settings_file.open(LORAWAN_SETTINGS_FILENAME, FILE_O_WRITE))
	{
		s_lorawan_settings default_settings;
		lorawan_settings_file.write((uint8_t *)&default_settings, sizeof(s_lorawan_settings));
		lorawan_settings_file.flush();
		lorawan_settings_file.close();
	}
}

/**
 * @brief Printout of all credentials
 *
 */
void log_credentials(void)
{
	DEBUG_LOG("FLASH", "Saved credentials:");
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
}

/**
 * @brief Printout of all settings
 *
 */
void log_settings(void)
{
	DEBUG_LOG("FLASH", "Saved settings:");
	DEBUG_LOG("FLASH", "071 ADR %s", g_lorawan_settings.adr_enabled ? "enabled" : "disabled");
	DEBUG_LOG("FLASH", "074 Repeat time %ld", g_lorawan_settings.send_repeat_time);
	DEBUG_LOG("FLASH", "078 Join trials %d", g_lorawan_settings.join_trials);
	DEBUG_LOG("FLASH", "079 TX Power %d", g_lorawan_settings.tx_power);
	DEBUG_LOG("FLASH", "080 DR %d", g_lorawan_settings.data_rate);
	DEBUG_LOG("FLASH", "082 Subband %d", g_lorawan_settings.subband_channels);
	DEBUG_LOG("FLASH", "084 Fport %d", g_lorawan_settings.app_port);
	DEBUG_LOG("FLASH", "085 %s Message", g_lorawan_settings.confirmed_msg_enabled ? "Confirmed" : "Unconfirmed");
	DEBUG_LOG("FLASH", "086 Region %s", region_names[g_lorawan_settings.lora_region]);
}