#include "modules/seeed_serial_can.h"

Serial_CAN can;
bool is_serial_can_init = false;

#define PID_ENGIN_PRM 0x0C
#define PID_VEHICLE_SPEED 0x0D
#define PID_COOLANT_TEMP 0x05
#define PID_VIN_MSG_COUNT 0x01
#define PID_VIN 0x02

#define CAN_ID_PID_OK 0x7E0
#define CAN_ID_PID 0x7DF

bool serial_can_init(void)
{
    can.begin(Serial1, 38400);

    is_serial_can_init = true;

    return is_serial_can_init;
}

void sendPid(unsigned char __pid)
{
  unsigned char tmp[8] = {0x02, 0x01, __pid, 0, 0, 0, 0, 0};

  can.send(CAN_ID_PID, 0, 0, 8, tmp); // SEND TO ID:0X55
}

void sendInfoPid(unsigned char __pid)
{
  unsigned char tmp[8] = {0x02, 0x09, __pid, 0, 0, 0, 0, 0};

  can.send(CAN_ID_PID, 0, 0, 8, tmp); // SEND TO ID:0X55
}

void sendOkPid()
{
  unsigned char tmp[8] = {0x30, 0, 0, 0, 0, 0, 0, 0};

  can.send(CAN_ID_PID_OK, 0, 0, 8, tmp); // SEND TO ID:0X55
}

bool getVIN(uint8_t *buf)
{
  unsigned long __timeout = millis();
  unsigned char temp_buf[8];
  unsigned long id = 0;

  sendInfoPid(PID_VIN);

  while (millis() - __timeout < 1000) // 1s time out
  {
    if (can.recv(&id, temp_buf))
    {
      for (int i = 5; i < 8; i++)
      {
        buf[i - 5] = temp_buf[i];
      }
      break;
    }
  }

  sendOkPid();

  __timeout = millis();
  id = 0;
  while (millis() - __timeout < 1000) // 1s time out
  {
    if (can.recv(&id, temp_buf))
    {
      for (int i = 1; i < 8; i++)
      {
        buf[i + 2] = temp_buf[i];
      }
      break;
    }
  }

  __timeout = millis();
  id = 0;
  while (millis() - __timeout < 1000) // 1s time out
  {
    if (can.recv(&id, temp_buf))
    {
      for (int i = 1; i < 8; i++)
      {
        buf[i + 9] = temp_buf[i];
      }
      return 1;
    }
  }

  return 0;
}

bool getSpeed(uint8_t *s)
{
    sendPid(PID_VEHICLE_SPEED);
    unsigned long __timeout = millis();

    while(millis()-__timeout < 1000)      // 1s time out
    {
        unsigned long id  = 0;
        unsigned char buf[8];

        if (can.recv(&id, buf)) {                // check if get data

            if(buf[1] == 0x41)
            {
                *s = buf[3];
                return 1;
            }
        }
    }

    return 0;
}

bool getRPM(uint16_t *r)
{
  sendPid(PID_ENGIN_PRM);
  unsigned long __timeout = millis();

  while (millis() - __timeout < 1000) // 1s time out
  {
    unsigned long id = 0;
    unsigned char buf[8];

    if (can.recv(&id, buf))
    { // check if get data

      if (buf[1] == 0x41)
      {
        *r = (256 * buf[3] + buf[4]) / 4;
        return 1;
      }
    }
  }

  return 0;
}

