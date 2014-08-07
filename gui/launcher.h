#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QMainWindow>
#include "../updater/updater.h"
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
    Updater* updater;
};

#endif // LAUNCHER_H
