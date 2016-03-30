#include "utils/system.h"

#include <QtGlobal>

OperatingSystem System::operatingSystem = NOTSET;

OperatingSystem System::get()
{
    if (operatingSystem != NOTSET)
    {
        return operatingSystem;
    }

#ifdef Q_OS_WIN
    operatingSystem = WINDOWS;
#elif defined Q_OS_MAC
    operatingSystem = MAC;
#elif defined Q_OS_LINUX
    operatingSystem = LINUX;
#else
    operatingSystem = UNKNOWN;
#endif

    return operatingSystem;
}
