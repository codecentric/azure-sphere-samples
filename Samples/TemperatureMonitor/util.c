#include "util.h"

#include <stdbool.h>

static volatile sig_atomic_t terminationRequested = false;

void TerminationHandler(int signalNumber)
{
    // Don't use Log_Debug here, as it is not guaranteed to be async signal safe
    terminationRequested = true;
}

sig_atomic_t IsTerminationRequested()
{
    return terminationRequested;
}

void RequestTermination()
{
    terminationRequested = true;
}