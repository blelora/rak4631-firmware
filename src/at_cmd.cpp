#include "at_cmd.h"

#define AT_CMD_STACK_SIZE (256 * 4)

static char atcmd[ATCMD_SIZE];
static uint16_t atcmd_index = 0;
char g_at_query_buf[ATQUERY_SIZE];

char *region_names[] = {(char *)"AS923", (char *)"AU915", (char *)"CN470", (char *)"CN779",
						(char *)"EU433", (char *)"EU868", (char *)"KR920", (char *)"IN865",
						(char *)"US915", (char *)"AS923-2", (char *)"AS923-3", (char *)"AS923-4", (char *)"RU864"};

void at_cmd_task(void *arg);
void at_serial_input(uint8_t cmd);

void at_cmd_init()
{
    DEBUG_LOG("AT CMD", "INIT");
    xTaskCreate(at_cmd_task, "AT_CMD", AT_CMD_STACK_SIZE, NULL, TASK_PRIO_LOW, NULL);
}

void at_cmd_task(void *arg)
{
    DEBUG_LOG("AT CMD", "TASK START");
    while (1)
    {
        // DEBUG_LOG("AT CMD", "TASK LOOP");
        uint8_t temp;
        while (Serial.available() > 0)
        {
            temp = Serial.read();
            // printf("%c", temp);
            at_serial_input(uint8_t(temp));
        }

		if ((g_task_event_type & BLE_DATA) == BLE_DATA)
		{
			/** BLE UART data arrived */
			g_task_event_type &= N_BLE_DATA;
			uint8_t temp;
			while (ble_uart.available() > 0)
			{
				temp = ble_uart.read();
				at_serial_input(uint8_t(temp));
				delay(5);
			}
			at_serial_input(uint8_t('\n'));
			DEBUG_LOG("BLE UART", "Received Command");
		}

        vTaskDelay(1);
    }
}
/**
 * @brief Convert Hex string into uint8_t array
 * 
 * @param hex Hex string
 * @param bin uint8_t
 * @param bin_length Length of array
 * @return int -1 if conversion failed
 */
static int hex2bin(const char *hex, uint8_t *bin, uint16_t bin_length)
{
	uint16_t hex_length = strlen(hex);
	const char *hex_end = hex + hex_length;
	uint8_t *cur = bin;
	uint8_t num_chars = hex_length & 1;
	uint8_t byte = 0;

	if (hex_length % 2 != 0)
	{
		return -1;
	}

	if (hex_length / 2 > bin_length)
	{
		return -1;
	}

	while (hex < hex_end)
	{
		if ('A' <= *hex && *hex <= 'F')
		{
			byte |= 10 + (*hex - 'A');
		}
		else if ('a' <= *hex && *hex <= 'f')
		{
			byte |= 10 + (*hex - 'a');
		}
		else if ('0' <= *hex && *hex <= '9')
		{
			byte |= *hex - '0';
		}
		else
		{
			return -1;
		}
		hex++;
		num_chars++;

		if (num_chars >= 2)
		{
			num_chars = 0;
			*cur++ = byte;
			byte = 0;
		}
		else
		{
			byte <<= 4;
		}
	}
	return cur - bin;
}

/**
 * @brief ATZ Initiate a system reset
 * 
 * @return int always 0
 */
static int at_exec_reboot(void)
{
	delay(100);
	sd_nvic_SystemReset();
	return 0;
}

/**
 * @brief ATR Restore flash defaults
 * 
 * @return int always 0
 */
static int at_exec_restore(void)
{
	flash_reset();
	return 0;
}

/**
 * @brief AT+DEVEUI=? Get current Device EUI
 * 
 * @return int always 0
 */
static int at_query_deveui(void)
{
	snprintf(g_at_query_buf, ATQUERY_SIZE,
			 "%02X%02X%02X%02X%02X%02X%02X%02X",
			 g_lorawan_credentials.node_device_eui[0],
			 g_lorawan_credentials.node_device_eui[1],
			 g_lorawan_credentials.node_device_eui[2],
			 g_lorawan_credentials.node_device_eui[3],
			 g_lorawan_credentials.node_device_eui[4],
			 g_lorawan_credentials.node_device_eui[5],
			 g_lorawan_credentials.node_device_eui[6],
			 g_lorawan_credentials.node_device_eui[7]);
	return 0;
}

/**
 * @brief AT+DEVEUI=<XXXXXXXXXXXXXXXX> Set current Device EUI
 * @return int 0 if Dev EUI has correct length and was valid HEX
 */
static int at_exec_deveui(char *str)
{
	uint8_t len;
	uint8_t buf[8];

	len = hex2bin(str, buf, 8);

	if (len != 8)
	{
		return AT_ERRNO_PARA_VAL;
	}

	memcpy(g_lorawan_credentials.node_device_eui, buf, 8);
	save_credentials();

	return 0;
}

/**
 * @brief AT+APPEUI=? Get current Application (Join) EUI
 * 
 * @return int always 0
 */
static int at_query_appeui(void)
{
	snprintf(g_at_query_buf, ATQUERY_SIZE,
			 "%02X%02X%02X%02X%02X%02X%02X%02X",
			 g_lorawan_credentials.node_app_eui[0],
			 g_lorawan_credentials.node_app_eui[1],
			 g_lorawan_credentials.node_app_eui[2],
			 g_lorawan_credentials.node_app_eui[3],
			 g_lorawan_credentials.node_app_eui[4],
			 g_lorawan_credentials.node_app_eui[5],
			 g_lorawan_credentials.node_app_eui[6],
			 g_lorawan_credentials.node_app_eui[7]);
	return 0;
}

/**
 * @brief AT+APPEUI=<XXXXXXXXXXXXXXXX> Set current Application (Join) EUI
 * 
 * @return int 0 if App EUI has correct length and was valid HEX
 */
static int at_exec_appeui(char *str)
{
	uint8_t len;
	uint8_t buf[8];

	len = hex2bin(str, buf, 8);
	if (len != 8)
	{
		return AT_ERRNO_PARA_VAL;
	}

	memcpy(g_lorawan_credentials.node_app_eui, buf, 8);
	save_credentials();

	return 0;
}

/**
 * @brief AT+APPKEY=? Get current Application Key
 * 
 * @return int always 0
 */
static int at_query_appkey(void)
{
	uint8_t i;
	uint8_t len = 0;

	for (i = 0; i < 16; i++)
	{
		len += snprintf(g_at_query_buf + len, ATQUERY_SIZE - len, "%02X", g_lorawan_credentials.node_app_key[i]);
		if (ATQUERY_SIZE <= len)
		{
			return -1;
		}
	}
	return 0;
}

/**
 * @brief AT+APPKEY=<XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX> Set current Application (Join) EUI
 * 
 * @return int 0 if App Key has correct length and was valid HEX
 */
static int at_exec_appkey(char *str)
{
	uint8_t buf[16];
	uint8_t len;

	len = hex2bin(str, buf, 16);
	if (len != 16)
	{
		return AT_ERRNO_PARA_VAL;
	}

	memcpy(g_lorawan_credentials.node_app_key, buf, 16);
	save_credentials();

	return 0;
}

/**
 * @brief AT+CFM=? Get current confirm/unconfirmed packet status
 * 
 * @return int always 0
 */
static int at_query_confirm(void)
{
	snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", g_lorawan_settings.confirmed_msg_enabled);
	return 0;
}

/**
 * @brief AT+CFM=X Set confirmed / unconfirmed packet sending
 * 
 * @param str 0 = unconfirmed 1 = confirmed packet
 * @return int 0 if correct parameter
 */
static int at_exec_confirm(char *str)
{
	int cfm;

	cfm = strtol(str, NULL, 0);
	if (cfm != 0 && cfm != 1)
	{
		return AT_ERRNO_PARA_VAL;
	}

	g_lorawan_settings.confirmed_msg_enabled = (lmh_confirm)cfm;
	save_settings();

	return 0;
}

/**
 * @brief AT+SENDFREQ=? Get current send frequency
 * 
 * @return int always 0
 */
static int at_query_sendfreq(void)
{
	// Return time in seconds, but it is saved in milli seconds
	snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", (g_lorawan_settings.send_repeat_time == 0) ? 0 : (int)(g_lorawan_settings.send_repeat_time));

	return 0;
}

/**
 * @brief AT+SENDFREQ=<value> Set current send frequency
 * 
 * @param str send frequency in seconds between 0 (disabled) and 3600
 * @return int 
 */
static int at_exec_sendfreq(char *str)
{
	long time = strtol(str, NULL, 0);

	if ((time < 0) || (time > 3600))
	{
		return AT_ERRNO_PARA_VAL;
	}

	g_lorawan_settings.send_repeat_time = time;
	save_settings();

	if ((g_lorawan_settings.send_repeat_time != 0))
	{
		// Now we are connected, start the timer that will wakeup the loop frequently
		// g_task_lora_tx_wakeup_timer.stop();
		// g_task_lora_tx_wakeup_timer.setPeriod(g_lorawan_settings.send_repeat_time);
		// g_task_lora_tx_wakeup_timer.start();
	}

	return 0;
}

/**
 * @brief AT+ADR=? Get current adaptive datarate status
 * 
 * @return int always 0
 */
static int at_query_adr(void)
{
	snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", g_lorawan_settings.adr_enabled ? 1 : 0);
	return 0;
}

/**
 * @brief AT+ADR=X Enable/disable adaptive datarate
 * 
 * @param str 0 = disable, 1 = enable ADR
 * @return int 0 if correct parameter
 */
static int at_exec_adr(char *str)
{
	int adr;

	adr = strtol(str, NULL, 0);
	if (adr != 0 && adr != 1)
	{
		return AT_ERRNO_PARA_VAL;
	}

	g_lorawan_settings.adr_enabled = (adr == 1 ? true : false);

	save_settings();

	return 0;
}

/**
 * @brief AT+DR=? Get current datarate
 * 
 * @return int always 0
 */
static int at_query_datarate(void)
{
	snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", g_lorawan_settings.data_rate);
	return 0;
}

/**
 * @brief AT+DR=X Set datarate
 * 
 * @param str 0 to 15, depending on region
 * @return int 0 if correct parameter
 */
static int at_exec_datarate(char *str)
{
	uint8_t datarate;

	datarate = strtol(str, NULL, 0);

	if (datarate > 15)
	{
		return AT_ERRNO_PARA_VAL;
	}

	g_lorawan_settings.data_rate = datarate;
	save_settings();

	return 0;
}

/**
 * @brief AT+TXP=? Get current TX power setting
 * 
 * @return int always 0
 */
static int at_query_txpower(void)
{
	snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", g_lorawan_settings.tx_power);
	return 0;
}

/**
 * @brief AT+TXP Set TX power
 * 
 * @param str TX power 0 to 10
 * @return int always 0
 */
static int at_exec_txpower(char *str)
{
	uint8_t tx_power;

	tx_power = strtol(str, NULL, 0);

	if (tx_power > 10)
	{
		return AT_ERRNO_PARA_VAL;
	}

	g_lorawan_settings.tx_power = tx_power;

	save_settings();

	return 0;
}

/**
 * @brief AT+BAND=? Get regional frequency band
 * 
 * @return int always 0
 */
static int at_query_region(void)
{
	// 0: AS923 1: AU915 2: CN470 3: CN779 4: EU433 5: EU868 6: KR920 7: IN865 8: US915 9: AS923-2 10: AS923-3 11: AS923-4 12: RU864
	snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", g_lorawan_settings.lora_region);

	return 0;
}

/**
 * @brief AT+BAND=xx Set regional frequency band
 *  Values: 0: AS923 1: AU915 2: CN470 3: CN779 4: EU433 5: EU868 6: KR920 7: IN865 8: US915 9: AS923-2 10: AS923-3 11: AS923-4 12: RU864
 * @return int 0 if valid parameter
 */
static int at_exec_region(char *str)
{
	char *param;
	uint8_t region;

	param = strtok(str, ",");
	if (param != NULL)
	{
		region = strtol(param, NULL, 0);
		// RAK4630 0: AS923 1: AU915 2: CN470 3: CN779 4: EU433 5: EU868 6: KR920 7: IN865 8: US915 9: AS923-2 10: AS923-3 11: AS923-4 12: RU864
		if (region > 12)
		{
			return AT_ERRNO_PARA_VAL;
		}
		g_lorawan_settings.lora_region = region;
		save_settings();
	}
	else
	{
		return AT_ERRNO_PARA_VAL;
	}

	return 0;
}

/**
 * @brief AT+MASK=? Get channel mask
 *  Only available for regions 1: AU915 2: CN470 8: US915
 * @return int always 0
 */
static int at_query_mask(void)
{
	if ((g_lorawan_settings.lora_region == 1) || (g_lorawan_settings.lora_region == 2) || (g_lorawan_settings.lora_region == 8))
	{
		snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", g_lorawan_settings.subband_channels);

		return 0;
	}
	return AT_ERRNO_PARA_VAL;
}

/**
 * @brief AT+MASK=xx Set channel mask
 *  Only available for regions 1: AU915 2: CN470 8: US915
 * @return int 0 if valid parameter
 */
static int at_exec_mask(char *str)
{
	char *param;
	uint8_t mask;

	param = strtok(str, ",");
	if (param != NULL)
	{
		mask = strtol(param, NULL, 0);

		uint8_t maxBand = 1;
		switch (g_lorawan_settings.lora_region)
		{
		case LORAMAC_REGION_AU915:
			maxBand = 9;
			break;
		case LORAMAC_REGION_CN470:
			maxBand = 12;
			break;
		case LORAMAC_REGION_US915:
			maxBand = 9;
			break;
		default:
			return AT_ERRNO_PARA_VAL;
		}
		if ((mask == 0) || (mask > maxBand))
		{
			return AT_ERRNO_PARA_VAL;
		}
		g_lorawan_settings.subband_channels = mask;
		save_settings();
	}
	else
	{
		return AT_ERRNO_PARA_NUM;
	}

	return 0;
}

/**
 * @brief AT+JT=? Get current join trials
 * 
 * @return int always 0
 */
static int at_query_jointrials(void)
{
	snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", g_lorawan_settings.join_trials);
	return 0;
}

/**
 * @brief AT+JT=X Set join trials
 * 
 * @param str 1 to 10
 * @return int 0 if correct parameter
 */
static int at_exec_jointrials(char *str)
{
	uint8_t jointrials;

	jointrials = strtol(str, NULL, 0);

	if (jointrials <= 0 || jointrials > 100)
	{
		return AT_ERRNO_PARA_VAL;
	}

	g_lorawan_settings.join_trials = jointrials;
	save_settings();

	return 0;
}

/**
 * @brief AT+AP=? Get current app port
 * 
 * @return int always 0
 */
static int at_query_appport(void)
{
	snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", g_lorawan_settings.app_port);
	return 0;
}

/**
 * @brief AT+AP=X Set app port
 * 
 * @param str 1 to 1223
 * @return int 0 if correct parameter
 */
static int at_exec_appport(char *str)
{
	uint8_t appport;

	appport = strtol(str, NULL, 0);

	if (appport <= 0 || appport > 224)
	{
		return AT_ERRNO_PARA_VAL;
	}

	g_lorawan_settings.app_port = appport;
	save_settings();

	return 0;
}

static int at_exec_list_all(void);

/**
 * @brief List of all available commands with short help and pointer to functions
 * 
 */
static atcmd_t g_at_cmd_list[] = {
	/*|    CMD    |     AT+CMD?      |    AT+CMD=?    |  AT+CMD=value |  AT+CMD  |*/
	// General commands
	{"?", "AT commands", NULL, NULL, at_exec_list_all},
	{"R", "Restore default", NULL, NULL, at_exec_restore},
	{"Z", "ATZ Trig a MCU reset", NULL, NULL, at_exec_reboot},
	// LoRaWAN keys, ID's EUI's
	{"+APPEUI", "Get or set the application EUI", at_query_appeui, at_exec_appeui, NULL},
	{"+APPKEY", "Get or set the application key", at_query_appkey, at_exec_appkey, NULL},
	{"+DEVEUI", "Get or set the device EUI", at_query_deveui, at_exec_deveui, NULL},
	// Joining and sending data on LoRa network
	{"+JT", "Get or Set the Join Trials", at_query_jointrials, at_exec_jointrials, NULL},
	{"+AP", "Get or Set the App Port", at_query_appport, at_exec_appport, NULL},
	{"+CFM", "Get or set the confirm mode", at_query_confirm, at_exec_confirm, NULL},
	{"+SENDFREQ", "Get or Set the automatic send time", at_query_sendfreq, at_exec_sendfreq, NULL},
	// LoRa network management
	{"+ADR", "Get or set the adaptive data rate setting", at_query_adr, at_exec_adr, NULL},
	{"+DR", "Get or Set the Tx DataRate=[0..7]", at_query_datarate, at_exec_datarate, NULL},
	{"+TXP", "Get or set the transmit power", at_query_txpower, at_exec_txpower, NULL},
	{"+BAND", "Get and Set number corresponding to active regions", at_query_region, at_exec_region, NULL},
	{"+MASK", "Get and Set channels mask", at_query_mask, at_exec_mask, NULL},
};

/**
 * @brief Handle received AT command
 * 
 */
static void at_cmd_handle(void)
{
	uint8_t i;
	int ret = 0;
	const char *cmd_name;
	char *rxcmd = atcmd + 2;
	int16_t tmp = atcmd_index - 2;
	uint16_t rxcmd_index;

	if (atcmd_index < 2 || rxcmd[tmp] != '\0')
	{
		atcmd_index = 0;
		memset(atcmd, 0xff, ATCMD_SIZE);
		return;
	}

	// Serial.printf("atcmd_index==%d=%s==\n", atcmd_index, atcmd);
	if (atcmd_index == 2 && strncmp(atcmd, "AT", atcmd_index) == 0)
	{
		atcmd_index = 0;
		memset(atcmd, 0xff, ATCMD_SIZE);
		DEBUG_LOG("AT", "\r\nOK\r\n");
		return;
	}

	rxcmd_index = tmp;

	// Check for standard AT commands
	for (i = 0; i < sizeof(g_at_cmd_list) / sizeof(atcmd_t); i++)
	{
		cmd_name = g_at_cmd_list[i].cmd_name;
		// Serial.printf("===rxcmd========%s================cmd_name=====%s====%d===\n", rxcmd, cmd_name, strlen(cmd_name));
		if (strlen(cmd_name) && strncmp(rxcmd, cmd_name, strlen(cmd_name)) != 0)
		{
			continue;
		}

		// Serial.printf("===rxcmd_index========%d================strlen(cmd_name)=====%d=======\n", rxcmd_index, strlen(cmd_name));

		if (rxcmd_index == (strlen(cmd_name) + 1) &&
			rxcmd[strlen(cmd_name)] == '?')
		{
			/* test cmd */
			if (g_at_cmd_list[i].cmd_desc)
			{
				if (strncmp(g_at_cmd_list[i].cmd_desc, "OK", 2) == 0)
				{
					snprintf(atcmd, ATCMD_SIZE, "\r\nOK\r\n");
				}
				else
				{
					snprintf(atcmd, ATCMD_SIZE, "\r\n%s:\"%s\"\r\nOK\r\n",
							 cmd_name, g_at_cmd_list[i].cmd_desc);
				}
			}
			else
			{
				snprintf(atcmd, ATCMD_SIZE, "\r\n%s\r\nOK\r\n", cmd_name);
			}
		}
		else if (rxcmd_index == (strlen(cmd_name) + 2) &&
				 strcmp(&rxcmd[strlen(cmd_name)], "=?") == 0)
		{
			/* query cmd */
			if (g_at_cmd_list[i].query_cmd != NULL)
			{
				ret = g_at_cmd_list[i].query_cmd();

				if (ret == 0)
				{
					snprintf(atcmd, ATCMD_SIZE, "\r\n%s:%s\r\nOK\r\n",
							 cmd_name, g_at_query_buf);
				}
			}
			else
			{
				ret = AT_ERRNO_NOALLOW;
			}
		}
		else if (rxcmd_index > (strlen(cmd_name) + 1) &&
				 rxcmd[strlen(cmd_name)] == '=')
		{
			/* exec cmd */
			if (g_at_cmd_list[i].exec_cmd != NULL)
			{
				ret = g_at_cmd_list[i].exec_cmd(rxcmd + strlen(cmd_name) + 1);
				if (ret == 0)
				{
					snprintf(atcmd, ATCMD_SIZE, "\r\nOK\r\n");
				}
				else if (ret == -1)
				{
					ret = AT_ERRNO_SYS;
				}
			}
			else
			{
				ret = AT_ERRNO_NOALLOW;
			}
		}
		else if (rxcmd_index == strlen(cmd_name))
		{
			/* exec cmd without parameter*/
			if (g_at_cmd_list[i].exec_cmd_no_para != NULL)
			{
				ret = g_at_cmd_list[i].exec_cmd_no_para();
				if (ret == 0)
				{
					snprintf(atcmd, ATCMD_SIZE, "\r\nOK\r\n");
				}
				else if (ret == -1)
				{
					ret = AT_ERRNO_SYS;
				}
			}
			else
			{
				ret = AT_ERRNO_NOALLOW;
			}
		}
		else
		{
			continue;
		}
		break;
	}

	if (ret != 0 && ret != AT_CB_PRINT)
	{
		snprintf(atcmd, ATCMD_SIZE, "\r\n%s%x\r\n", AT_ERROR, ret);
	}

	if (ret != AT_CB_PRINT)
	{
		DEBUG_LOG("AT", atcmd);
	}

	atcmd_index = 0;
	memset(atcmd, 0xff, ATCMD_SIZE);
	return;
}

/**
 * @brief Get Serial input and start parsing
 * 
 * @param cmd received character
 */
void at_serial_input(uint8_t cmd)
{
	Serial.printf("%c", cmd);

	// Handle backspace
	if (cmd == '\b')
	{
		atcmd[atcmd_index--] = '\0';
		Serial.printf(" \b");
	}

	if ((cmd >= '0' && cmd <= '9') || (cmd >= 'a' && cmd <= 'z') ||
		(cmd >= 'A' && cmd <= 'Z') || cmd == '?' || cmd == '+' || cmd == ':' ||
		cmd == '=' || cmd == ' ' || cmd == ',')
	{
		atcmd[atcmd_index++] = cmd;
	}
	else if (cmd == '\r' || cmd == '\n')
	{
		atcmd[atcmd_index] = '\0';
		at_cmd_handle();
	}

	if (atcmd_index >= ATCMD_SIZE)
	{
		atcmd_index = 0;
	}
}

/**
 * @brief List all available commands with short help
 * 
 * @return int always 0
 */
static int at_exec_list_all(void)
{
	DEBUG_LOG("AT", "\r\n+++++++++++++++\r\n");
	DEBUG_LOG("AT", "AT command list\r\n");
	DEBUG_LOG("AT", "+++++++++++++++\r\n");

	for (unsigned int idx = 0; idx < sizeof(g_at_cmd_list) / sizeof(atcmd_t); idx++)
	{
		if (idx < 4)
		{
			DEBUG_LOG("AT", "AT%s\t\t%s\r\n", g_at_cmd_list[idx].cmd_name, g_at_cmd_list[idx].cmd_desc);
		}
		else
		{
			DEBUG_LOG("AT", "AT%s\t%s\r\n", g_at_cmd_list[idx].cmd_name, g_at_cmd_list[idx].cmd_desc);
		}
	}

	DEBUG_LOG("AT", "+++++++++++++++\r\n");
	return 0;
}