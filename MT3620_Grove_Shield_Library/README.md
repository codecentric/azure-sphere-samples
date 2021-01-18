# MT3620 Grove Shield

This is a copy of [Seeed Studio's MT3620 Grove Shield Library](https://github.com/Seeed-Studio/MT3620_Grove_Shield/tree/master/MT3620_Grove_Shield_Library) with minor changes.

# Changes from the original source code

## 4-Digit-Display Driver

Additionally to `Grove4DigitDisplay_DisplayOneSegment` there is a `Grove4DigitDisplay_DisplayOneSegmentRaw` which accepts any value for `dispData`. `dispData` is limited to 0-15 which map to the numbers 0 to 1 and letters A to F. `Grove4DigitDisplay_DisplayOneSegmentRaw`'s `dispData` can be set to any of the 256 variants. See the [main file of multi-threading example](../Samples/MultiThreading/main.c):

```c
#define SEGMENT_CLOCK_POINT 0b10000000
#define SEGMENT_NONE 0b00000000
#define SEGMENT_MIDDLE 0b01000000
#define SEGMENT_TOP_LEFT 0b00100000
#define SEGMENT_BOTTOM_LEFT 0b00010000
#define SEGMENT_BOTTOM 0b00001000
#define SEGMENT_BOTTOM_RIGHT 0b00000100
#define SEGMENT_TOP_RIGHT 0b00000010
#define SEGMENT_TOP 0b00000001
```