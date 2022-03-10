#include "at_cmd.h"

static char atcmd[ATCMD_SIZE];
static uint16_t atcmd_index = 0;
char g_at_query_buf[ATQUERY_SIZE];

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