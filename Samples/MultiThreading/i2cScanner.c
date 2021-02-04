#include "../MT3620_Grove_Shield_Library/HAL/GroveI2C.h"
#include "../MT3620_Grove_Shield_Library/HAL/GroveUART.h"

#include <stdlib.h>
#include <time.h>

#include <applibs/log.h>

void i2cScanner(int i2cFd)
{
    uint8_t i2cState;
    const struct timespec sleepTime = {0, 10000000};

    for (uint8_t addr = 1; addr < 127; addr++)
    {

        uint8_t send[4];
        send[0] = 'S';
        send[1] = (uint8_t)((addr << 1) & 0xfe);
        send[2] = (uint8_t)0;
        send[3] = 'P';

        Log_Debug("0x%02X ", addr);
        GroveUART_Write(i2cFd, send, (int)sizeof(send));
        Log_Debug("W");
        SC18IM700_ReadReg(i2cFd, 0x0A, &i2cState);
        Log_Debug("R");
        if (i2cState == I2C_OK)
        {
            Log_Debug(" = I2C_OK\r\n");
        }
        else if (i2cState == I2C_TIME_OUT)
        {
            Log_Debug(" = I2C_TIME_OUT\r\n");
        }
        else if (i2cState == I2C_NACK_ON_ADDRESS)
        {
            Log_Debug(" = I2C_NACK_ON_ADDRESS\r\n");
        }
        else if (i2cState == I2C_NACK_ON_DATA)
        {
            Log_Debug(" = I2C_NACK_ON_DATA\r\n");
        }

        nanosleep(&sleepTime, NULL);
    }
}