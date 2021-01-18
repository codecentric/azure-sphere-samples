#include "gpio.h"
#include "exit.h"

#include <applibs/log.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

int OpenGPIO(int gpioId, GPIO_Value_Type initialValue)
{
    int fd = GPIO_OpenAsOutput(gpioId, GPIO_OutputMode_PushPull, initialValue);
    if (fd < 0)
    {
        Log_Debug(
            "Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n",
            strerror(errno), errno);
        return ExitCode_Main_Led;
    }
    return fd;
}