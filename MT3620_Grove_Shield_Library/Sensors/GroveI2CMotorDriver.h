#pragma once

#include <stdlib.h>
#include <stdint.h>

#define MotorSpeedSet 0x82
#define PWMFrequenceSet 0x84
#define DirectionSet 0xaa
#define MotorSetA 0xa1
#define MotorSetB 0xa5
#define Nothing 0x01

#define MOTOR1 1
#define MOTOR2 2

#define ClockWise 0b10
#define AntiClockWise 0b01

#define F_31372Hz 0x01
#define F_3921Hz 0x02
#define F_490Hz 0x03
#define F_122Hz 0x04
#define F_30Hz 0x05

#define DEFAULT_MOTOR_DRIVER_ADDRESS 0x0F

void* GroveI2CMotorDriver_Open(int ic2Fd, uint8_t address);
void GroveI2CMotorDriver_SetMotorFrequency(void* inst, uint8_t frequency);
void GroveI2CMotorDriver_SetMotorDirection(void* inst, uint8_t _direction);
void GroveI2CMotorDriver_SetMotorSpeed(void* inst, uint8_t motorId, int speed);
void GroveI2CMotorDriver_SetMotorSpeeds(void* inst, int speed1, int speed2);
void GroveI2CMotorDriver_StopMotor(void* inst, uint8_t motorId);
void GroveI2CMotorDriver_StopMotors(void* inst);
