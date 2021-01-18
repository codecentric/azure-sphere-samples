#include "sleep.h"

#define _BSD_SOURCE
#include <unistd.h>

void Sleep(unsigned int msec)
{
    usleep(msec * 1000);
}

