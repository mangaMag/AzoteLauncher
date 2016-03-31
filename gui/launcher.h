#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "../updater/updater.h"
#include "../logger/logger.h"
#include "../others/sound.h"

#include <QMainWindow>
#include <QPoint>
#include <QMouseEvent>
#include <QProcess>
#include <QSystemTrayIcon>

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
    Logger* log;
    QPoint mpos;
    int port;
    bool isRegStarted;
    QProcess* reg;
    Sound* sound;
    QSystemTrayIcon* trayIcon;
    bool isTrayIconMessageDisplayed;

    void closeEvent(QCloseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void changeEvent(QEvent* event);

private slots:
    void onClickPlayButton();
    void onPressedPlayButton();
    void onReleasedPlayButton();
    void onClickCloseButton();
    void onClickMinimizeButton();
    void onClickSettingsButton();
    void onClickSystemTrayIcon(QSystemTrayIcon::ActivationReason reason);
    void onCloseApp();
    void onOpenApp();
};

#endif // LAUNCHER_H
