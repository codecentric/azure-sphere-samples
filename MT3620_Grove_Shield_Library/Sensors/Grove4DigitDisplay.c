#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include <applibs/gpio.h>

#include "Grove4DigitDisplay.h"
#include "../Common/Delay.h"

#define DELAY 1

static bool _clockpoint = false;

typedef struct
{
	int ClkFd;
	int DioFd;
	float Brightness;
} Grove4DigitDisplayInstance;

////////////////////////////////////////////////////////////////////////////////
// TM1637

static void TM1637_Start(Grove4DigitDisplayInstance *this)
{
	GPIO_SetValue(this->ClkFd, GPIO_Value_High);
	GPIO_SetValue(this->DioFd, GPIO_Value_Low);
	usleep(DELAY);
}

static void TM1637_End(Grove4DigitDisplayInstance *this)
{
	GPIO_SetValue(this->ClkFd, GPIO_Value_Low);
	usleep(DELAY);
	GPIO_SetValue(this->ClkFd, GPIO_Value_High);
	usleep(DELAY);
	GPIO_SetValue(this->DioFd, GPIO_Value_High);
	usleep(DELAY);
}

static void TM1637_Write(Grove4DigitDisplayInstance *this, uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		GPIO_SetValue(this->ClkFd, GPIO_Value_Low);
		GPIO_SetValue(this->DioFd, data & 1 ? GPIO_Value_High : GPIO_Value_Low);
		data >>= 1;
		usleep(DELAY);

		GPIO_SetValue(this->ClkFd, GPIO_Value_High);
		usleep(DELAY);
	}

	GPIO_SetValue(this->DioFd, GPIO_Value_High);
	GPIO_SetValue(this->ClkFd, GPIO_Value_Low);
	usleep(DELAY);
	
	GPIO_Value_Type ack;
	GPIO_GetValue(this->DioFd, &ack);
	GPIO_SetValue(this->DioFd, GPIO_Value_Low);
	GPIO_SetValue(this->ClkFd, GPIO_Value_High);
	usleep(DELAY);
}

////////////////////////////////////////////////////////////////////////////////
// Grove4DigitDisplay

#define DATA_COMMAND_SETTING            0b01000000
#define DISPLAY_CONTROL_COMMAND_SETTING 0b10000000
#define ADDRESS_COMMAND_SETTING         0b11000000

#define DATA_COMMAND_WRITE       DATA_COMMAND_SETTING | 0b00000000
#define DATA_COMMAND_READ        DATA_COMMAND_SETTING | 0b00000010
#define DATA_COMMAND_AUTO_ADDR   DATA_COMMAND_SETTING | 0b00000000
#define DATA_COMMAND_FIXED_ADDR  DATA_COMMAND_SETTING | 0b00000100
#define DATA_COMMAND_NORMAL_MODE DATA_COMMAND_SETTING | 0b00000000
#define DATA_COMMAND_TEST_MODE   DATA_COMMAND_SETTING | 0b00001000

#define ADDR_COMMAND_C0H ADDRESS_COMMAND_SETTING | 0b00000000
#define ADDR_COMMAND_C1H ADDRESS_COMMAND_SETTING | 0b00000001
#define ADDR_COMMAND_C2H ADDRESS_COMMAND_SETTING | 0b00000010
#define ADDR_COMMAND_C3H ADDRESS_COMMAND_SETTING | 0b00000011
#define ADDR_COMMAND_C4H ADDRESS_COMMAND_SETTING | 0b00000100
#define ADDR_COMMAND_C5H ADDRESS_COMMAND_SETTING | 0b00000101

#define DISPLAY_CONTROL_1_16        (DISPLAY_CONTROL_COMMAND_SETTING | 0b00000000)
#define DISPLAY_CONTROL_2_16        (DISPLAY_CONTROL_COMMAND_SETTING | 0b00000001)
#define DISPLAY_CONTROL_4_16        (DISPLAY_CONTROL_COMMAND_SETTING | 0b00000010)
#define DISPLAY_CONTROL_10_16       (DISPLAY_CONTROL_COMMAND_SETTING | 0b00000011)
#define DISPLAY_CONTROL_11_16       (DISPLAY_CONTROL_COMMAND_SETTING | 0b00000100)
#define DISPLAY_CONTROL_12_16       (DISPLAY_CONTROL_COMMAND_SETTING | 0b00000101)
#define DISPLAY_CONTROL_13_16       (DISPLAY_CONTROL_COMMAND_SETTING | 0b00000110)
#define DISPLAY_CONTROL_14_16       (DISPLAY_CONTROL_COMMAND_SETTING | 0b00000111)
#define DISPLAY_CONTROL_DISPLAY_OFF (DISPLAY_CONTROL_COMMAND_SETTING | 0b00000000)
#define DISPLAY_CONTROL_DISPLAY_ON  (DISPLAY_CONTROL_COMMAND_SETTING | 0b00001000)

/*static const uint8_t TubeTab[] =
	{
		0x3f, 0x06, 0x5b, 0x4f, // '0', '1', '2', '3',
		0x66, 0x6d, 0x7d, 0x07, // '4', '5', '6', '7',
		0x7f, 0x6f, 0x77, 0x7c, // '8', '9', 'A', 'b',
		0x39, 0x5e, 0x79, 0x71, // 'C', 'd', 'E', 'F',
};*/

static const uint8_t TubeTab[] =
	{
		0b00111111, 0b00000110, 0b01011011, 0b01001111, // '0', '1', '2', '3',
		0b01100110, 0b01101101, 0b01111101, 0b00000111, // '4', '5', '6', '7',
		0b01111111, 0b01101111, 0b01110111, 0b01111100, // '8', '9', 'A', 'b',
		0b00111001, 0b01011110, 0b01111001, 0b01110001, // 'C', 'd', 'E', 'F',
};

uint8_t ToSegmentData(int dispData, bool isRaw)
{
	uint8_t segData;
	if (dispData == -1)
	{
		segData = BLANK_DIGIT;
	}
	else
	{
		if (isRaw)
		{
			segData = (uint8_t)dispData;
		}
		else
		{
			if (0 <= dispData && dispData <= 15)
			{
				segData = TubeTab[dispData];
			}
			else
			{
				segData = BLANK_DIGIT;
			}
		}
	}
	if (_clockpoint)
	{
		segData |= SEGMENT_CLOCK_POINT;
	}
	return segData;
}

void *Grove4DigitDisplay_Open(GPIO_Id pin_clk, GPIO_Id pin_dio)
{
	Grove4DigitDisplayInstance *this = (Grove4DigitDisplayInstance *)malloc(sizeof(Grove4DigitDisplayInstance));

	this->Brightness = 0.5f;

	this->ClkFd = GPIO_OpenAsOutput(pin_clk, GPIO_OutputMode_PushPull, GPIO_Value_High);
	this->DioFd = GPIO_OpenAsOutput(pin_dio, GPIO_OutputMode_OpenDrain, GPIO_Value_High);
	usleep(DELAY);

	return this;
}

void Grove4DigitDisplay_DisplayOneSegment(void *inst, int bitAddr, int dispData)
{
	Grove4DigitDisplayInstance *this = (Grove4DigitDisplayInstance *)inst;

	uint8_t segData = ToSegmentData(dispData, false);

	TM1637_Start(this);
	TM1637_Write(this, DATA_COMMAND_FIXED_ADDR);
	TM1637_End(this);

	TM1637_Start(this);
	TM1637_Write(this, (uint8_t)(bitAddr | ADDRESS_COMMAND_SETTING));
	TM1637_Write(this, segData);
	TM1637_End(this);

	TM1637_Start(this);
	TM1637_Write(this, (uint8_t)(DISPLAY_CONTROL_DISPLAY_ON + this->Brightness * 7));
	TM1637_End(this);
}

void Grove4DigitDisplay_DisplayOneSegmentRaw(void *inst, int bitAddr, int dispData)
{
	Grove4DigitDisplayInstance *this = (Grove4DigitDisplayInstance *)inst;

	uint8_t segData = ToSegmentData(dispData, true);

	TM1637_Start(this);
	TM1637_Write(this, DATA_COMMAND_FIXED_ADDR);
	TM1637_End(this);

	TM1637_Start(this);
	TM1637_Write(this, (uint8_t)(bitAddr | ADDRESS_COMMAND_SETTING));
	TM1637_Write(this, segData);
	TM1637_End(this);

	TM1637_Start(this);
	TM1637_Write(this, (uint8_t)(DISPLAY_CONTROL_DISPLAY_ON + this->Brightness * 7));
	TM1637_End(this);
}

void Grove4DigitDisplay_DisplaySegments(void *inst, uint8_t dispData[])
{
	Grove4DigitDisplayInstance *this = (Grove4DigitDisplayInstance *)inst;

	uint8_t segData[4];

	for (uint8_t digit = 0; digit < 4; digit++)
	{
		segData[digit] = ToSegmentData(dispData[digit], false);
	}

	TM1637_Start(this);
	TM1637_Write(this, DATA_COMMAND_AUTO_ADDR);
	TM1637_End(this);

	TM1637_Start(this);
	TM1637_Write(this, (uint8_t)(ADDR_COMMAND_C0H));

	for (uint8_t digit = 0; digit < 4; digit++)
	{
		TM1637_Write(this, segData[digit]);
	}

	TM1637_End(this);

	TM1637_Start(this);
	TM1637_Write(this, (uint8_t)(DISPLAY_CONTROL_DISPLAY_ON + this->Brightness * 7));
	TM1637_End(this);
}

void Grove4DigitDisplay_DisplaySegmentsRaw(void *inst, uint8_t dispData[])
{
	Grove4DigitDisplayInstance *this = (Grove4DigitDisplayInstance *)inst;

	uint8_t segData[4];

	for (uint8_t digit = 0; digit < 4; digit++)
	{
		segData[digit] = ToSegmentData(dispData[digit], true);
	}

	TM1637_Start(this);
	TM1637_Write(this, DATA_COMMAND_AUTO_ADDR);
	TM1637_End(this);

	TM1637_Start(this);
	TM1637_Write(this, (uint8_t)(ADDR_COMMAND_C0H));

	for (uint8_t digit = 0; digit < 4; digit++)
	{
		TM1637_Write(this, segData[digit]);
	}

	TM1637_End(this);

	TM1637_Start(this);
	TM1637_Write(this, (uint8_t)(DISPLAY_CONTROL_DISPLAY_ON + this->Brightness * 7));
	TM1637_End(this);
}

void Grove4DigitDisplay_DisplayValue(void *inst, uint8_t value)
{
	uint8_t digit1 = (uint8_t)((value / 1000) % 10);
	uint8_t digit2 = (uint8_t)((value / 100) % 10);
	uint8_t digit3 = (uint8_t)((value / 10) % 10);
	uint8_t digit4 = (uint8_t)(value % 10);
	uint8_t displayData[] = {digit1, digit2, digit3, digit4};
	Grove4DigitDisplay_DisplaySegments(inst, displayData);
}

void Grove4DigitDisplay_DisplayClockPoint(bool clockpoint)
{
	_clockpoint = clockpoint;
}