#include "GroveI2CMotorDriver.h"
#include "../HAL/GroveI2C.h"
#include "../Common/Delay.h"

typedef struct
{
    int i2cFd;
    uint8_t address;
    uint8_t motor1Direction;
    uint8_t motor2Direction;
    uint8_t motor1Speed;
    uint8_t motor2Speed;
} GroveI2CMotorDriverInstance;

#define delay(ms) usleep(ms * 1000)
#define min(a, b) (a > b ? b : a)

uint8_t mapSpeed(int speed)
{
    return (uint8_t)(min(100, abs(speed)) * 255 / 100);
}

void* GroveI2CMotorDriver_Open(int ic2Fd, uint8_t address)
{
    GroveI2CMotorDriverInstance *this = (GroveI2CMotorDriverInstance *)malloc(sizeof(GroveI2CMotorDriverInstance));

    this->i2cFd = ic2Fd;
    this->address = (uint8_t)(address << 1);
    this->motor1Direction = ClockWise;
    this->motor2Direction = ClockWise;
    this->motor1Speed = 0;
    this->motor2Speed = 0;

    delay(10);
    GroveI2CMotorDriver_SetMotorFrequency(this, F_3921Hz);
    delay(4);

    return this;
}

void GroveI2CMotorDriver_SetMotorFrequency(void *inst, uint8_t frequency)
{
    GroveI2CMotorDriverInstance *this = (GroveI2CMotorDriverInstance *)inst;
    uint8_t data[] = {PWMFrequenceSet, frequency, Nothing};
    GroveI2C_WriteBytes(this->i2cFd, this->address, data, sizeof(data));
    delay(4);
}

void GroveI2CMotorDriver_SetMotorDirection(void *inst, uint8_t direction)
{
    GroveI2CMotorDriverInstance *this = (GroveI2CMotorDriverInstance *)inst;
    uint8_t data[] = {DirectionSet, direction, Nothing};
    GroveI2C_WriteBytes(this->i2cFd, this->address, data, sizeof(data));
    delay(4);
}

void GroveI2CMotorDriver_SetMotorSpeed(void *inst, uint8_t motorId, int speed)
{
    GroveI2CMotorDriverInstance *this = (GroveI2CMotorDriverInstance *)inst;

    if (motorId != MOTOR1 && motorId != MOTOR2)
    {
        return;
    }

    if (motorId == MOTOR1)
    {
        this->motor1Direction = speed >= 0 ? ClockWise : AntiClockWise;
        this->motor1Speed = mapSpeed(speed);
    }
    else if (motorId == MOTOR2)
    {
        this->motor2Direction = speed >= 0 ? ClockWise : AntiClockWise;
        this->motor2Speed = mapSpeed(speed);
    }

    GroveI2CMotorDriver_SetMotorDirection(this, (uint8_t)(this->motor1Direction | this->motor2Direction << 2));

    uint8_t data[] = {MotorSpeedSet, this->motor1Speed, this->motor2Speed};
    GroveI2C_WriteBytes(this->i2cFd, this->address, data, sizeof(data));
    delay(4);
}

void GroveI2CMotorDriver_SetMotorSpeeds(void *inst, int speed1, int speed2)
{
    GroveI2CMotorDriverInstance *this = (GroveI2CMotorDriverInstance *)inst;

    this->motor1Direction = speed1 >= 0 ? ClockWise : AntiClockWise;
    this->motor2Direction = speed2 >= 0 ? ClockWise : AntiClockWise;
    this->motor1Speed = mapSpeed(speed1);
    this->motor2Speed = mapSpeed(speed2);

    GroveI2CMotorDriver_SetMotorDirection(this, (uint8_t)(this->motor1Direction | this->motor2Direction << 2));

    uint8_t data[] = {MotorSpeedSet, this->motor1Speed, this->motor2Speed};
    GroveI2C_WriteBytes(this->i2cFd, this->address, data, sizeof(data));
    delay(4);
}

void GroveI2CMotorDriver_StopMotor(void *inst, uint8_t motorId)
{
    GroveI2CMotorDriver_SetMotorSpeed(inst, motorId, 0);
}

void GroveI2CMotorDriver_StopMotors(void *inst)
{
    GroveI2CMotorDriver_SetMotorSpeeds(inst, 0, 0);
}