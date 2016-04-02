#ifndef CONSOLE_H
#define CONSOLE_H

#include <QWidget>
#include <QObject>

#include "../logger/loglevel.h"
#include "../logger/logger.h"
#include "settings.h"

namespace Ui {
class Console;
}

class Console : public QWidget
{
    Q_OBJECT

public:
    Console(QWidget* parent, QObject* logger, Settings* _settings);
    ~Console();

    void show();

private:
    Ui::Console* ui;
    Settings* settgins;

private slots:
    void onMessage(LogLevel level, QString text);
    void onConsoleStateChange(bool isEnabled);
};

#endif // CONSOLE_H
