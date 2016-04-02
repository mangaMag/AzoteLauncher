#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>

#include "../serialization/singleton.h"
#include "loglevel.h"

class Logger : public QObject
{
    Q_OBJECT

public:
    Logger();
    ~Logger();
    void debug(QString text);
    void error(QString text);
    void success(QString text);
    void info(QString text);
    void warning(QString text);

signals:
    void close();
    void message(LogLevel, QString text);
};

#endif // LOGGER_H
