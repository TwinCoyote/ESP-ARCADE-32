#include "millis.h"

void millis(unsigned long durationMs)
{
    unsigned long start = ::millis();
    while (::millis() - start < durationMs)
    {
        }
}
