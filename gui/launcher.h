#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QMainWindow>
#include "../updater/updaterv2.h"
#include "../logger/logger.h"

namespace Ui {
class Launcher;
}

class Launcher : public QMainWindow
{
    Q_OBJECT

public:
    explicit Launcher(QWidget *parent = 0);
    ~Launcher();

private:
    Ui::Launcher* ui;
    UpdaterV2* updater;
    Logger* log;

    void closeEvent(QCloseEvent* event);
};

#endif // LAUNCHER_H
