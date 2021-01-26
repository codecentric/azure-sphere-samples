#ifndef UTIL_H_
#define UTIL_H_

#include <signal.h>

void TerminationHandler(int signalNumber);
sig_atomic_t IsTerminationRequested(void);
void RequestTermination(void);

#endif //UTIL_H_