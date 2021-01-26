#include "motorDriver.h"
#include "../../MT3620_Grove_Shield_Library/HAL/GroveI2C.h"
#include "../../MT3620_Grove_Shield_Library/Common/Delay.h"

#define DEFAULT_ADDRESS 0x0f

static int _i2cFd = -1;
static uint8_t _address = 0x0f;
static uint8_t motor1Direction = ClockWise;
static uint8_t motor2Direction = ClockWise;
static uint8_t motor1Speed;
static uint8_t motor2Speed;

#define delay(ms) usleep(ms * 1000)
#define min(a, b) (a > b ? b : a)

uint8_t mapSpeed(int speed)
{
    return (uint8_t)(min(100, abs(speed)) * 255 / 100);
}

void InitMotor(int ic2Fd, uint8_t address)
{
    _i2cFd = ic2Fd;
    _address = (uint8_t)(address << 1);

    delay(10);
    SetMotorFrequency(F_3921Hz);
    delay(4);
}

void SetMotorFrequency(uint8_t frequency)
{
    uint8_t data[] = {PWMFrequenceSet, frequency, Nothing};
    GroveI2C_WriteBytes(_i2cFd, _address, data, sizeof(data));
    delay(4);
}

void SetMotorDirection(uint8_t direction)
{
    uint8_t data[] = {DirectionSet, direction, Nothing};
    GroveI2C_WriteBytes(_i2cFd, _address, data, sizeof(data));
    delay(4);
}

void SetMotorSpeed(uint8_t motorId, int speed)
{
    if (motorId != MOTOR1 && motorId != MOTOR2)
    {
        return;
    }

    if (motorId == MOTOR1)
    {
        motor1Direction = speed >= 0 ? ClockWise : AntiClockWise;
        motor1Speed = mapSpeed(speed);
    }
    else if (motorId == MOTOR2)
    {
        motor2Direction = speed >= 0 ? ClockWise : AntiClockWise;
        motor2Speed = mapSpeed(speed);
    }

    SetMotorDirection((uint8_t)(motor1Direction | motor2Direction << 2));

    uint8_t data[] = {MotorSpeedSet, motor1Speed, motor2Speed};
    GroveI2C_WriteBytes(_i2cFd, _address, data, sizeof(data));
    delay(4);
}

void SetMotorSpeeds(int speed1, int speed2)
{
    motor1Direction = speed1 >= 0 ? ClockWise : AntiClockWise;
    motor2Direction = speed2 >= 0 ? ClockWise : AntiClockWise;
    motor1Speed = mapSpeed(speed1);
    motor2Speed = mapSpeed(speed2);

    SetMotorDirection((uint8_t)(motor1Direction | motor2Direction << 2));

    uint8_t data[] = {MotorSpeedSet, motor1Speed, motor2Speed};
    GroveI2C_WriteBytes(_i2cFd, _address, data, sizeof(data));
    delay(4);
}

void StopMotor(uint8_t motorId)
{
    SetMotorSpeed(motorId, 0);
}

void StopMotors(void)
{
    SetMotorSpeeds(0, 0);
}