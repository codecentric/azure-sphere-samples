#pragma once

#include <stdlib.h>
#include <stdint.h>

/******I2C command definitions*************/
#define MotorSpeedSet 0x82
#define PWMFrequenceSet 0x84
#define DirectionSet 0xaa
#define MotorSetA 0xa1
#define MotorSetB 0xa5
#define Nothing 0x01
/**************Motor ID**********************/
#define MOTOR1 1
#define MOTOR2 2
/**************Motor Direction***************/
#define ClockWise 0b10
#define AntiClockWise 0b01
/**************Prescaler Frequence***********/
#define F_31372Hz 0x01
#define F_3921Hz 0x02
#define F_490Hz 0x03
#define F_122Hz 0x04
#define F_30Hz 0x05

void InitMotor(int ic2Fd, uint8_t address);
void SetMotorFrequency(uint8_t frequency);
void SetMotorDirection(uint8_t _direction);
void SetMotorSpeed(uint8_t motorId, int speed);
void SetMotorSpeeds(int speed1, int speed2);
void StopMotor(uint8_t motorId);
void StopMotors(void);
