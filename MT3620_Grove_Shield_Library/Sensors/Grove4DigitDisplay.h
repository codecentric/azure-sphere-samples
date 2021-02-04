//GROVE_NAME        "Grove - 4-Digit Display"
//SKU               104030003
//WIKI_URL          http://wiki.seeedstudio.com/Grove-4-Digit_Display/

#pragma once

#include "../applibs_versions.h"
#include <applibs/gpio.h>
#include <stdbool.h>

#define BLANK_DIGIT 0b00000000
#define SEGMENT_CLOCK_POINT (uint8_t)0b10000000
#define SEGMENT_NONE (uint8_t)0b00000000
#define SEGMENT_MIDDLE (uint8_t)0b01000000
#define SEGMENT_TOP_LEFT (uint8_t)0b00100000
#define SEGMENT_BOTTOM_LEFT (uint8_t)0b00010000
#define SEGMENT_BOTTOM (uint8_t)0b00001000
#define SEGMENT_BOTTOM_RIGHT (uint8_t)0b00000100
#define SEGMENT_TOP_RIGHT (uint8_t)0b00000010
#define SEGMENT_TOP (uint8_t)0b00000001

void* Grove4DigitDisplay_Open(GPIO_Id pin_clk, GPIO_Id pin_dio);
void Grove4DigitDisplay_DisplayOneSegment(void* inst, int bitAddr, int dispData);
void Grove4DigitDisplay_DisplayOneSegmentRaw(void* inst, int bitAddr, int dispData);
void Grove4DigitDisplay_DisplaySegments(void *inst, uint8_t dispData[]);
void Grove4DigitDisplay_DisplaySegmentsRaw(void *inst, uint8_t dispData[]);
void Grove4DigitDisplay_DisplayValue(void* inst, uint8_t value);
void Grove4DigitDisplay_DisplayClockPoint(bool clockpoint);
