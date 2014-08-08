#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include "../serialization/singleton.h"
#include "../gui/console.h"

class Logger : public QObject
{
    Q_OBJECT

public:
    Logger();
    ~Logger();

    void showConsole();
    void closeConsole();
    void debug(QString text);
    void error(QString text);
    void success(QString text);
    void info(QString text);

private:
    Console* console;

signals:
    void close();
    void message(QString text);
};

#endif // LOGGER_H
