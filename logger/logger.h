#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include "../serialization/singleton.h"

class Logger
{
public:
    Logger();
    void showConsole();
    void debug(QString text);
    void error(QString text);
};

#endif // LOGGER_H
