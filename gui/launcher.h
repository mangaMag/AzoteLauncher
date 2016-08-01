#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "../updater/updater.h"
#include "../logger/logger.h"
#include "../others/sound.h"
#include "settings.h"
#include "console.h"

#include <QMainWindow>
#include <QPoint>
#include <QMouseEvent>
#include <QSystemTrayIcon>
#include <QMap>
#include <QUrl>

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
    QSystemTrayIcon* trayIcon;
    Settings* settings;
    Console* console;
    QMap<QObject*, QUrl> urls;

    void closeEvent(QCloseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void changeEvent(QEvent* event);

    void startUpdate();
    void startGame(QString gamePath);

private slots:
    void onClickPlayButton();
    void onClickLinkButton();
    void onClickCloseButton();
    void onClickMinimizeButton();
    void onClickSettingsButton();
    void onClickSystemTrayIcon(QSystemTrayIcon::ActivationReason reason);
    void onCloseApp();
    void onOpenApp();
    void onNewUpdaterVersion();
    void onRepairStarted();
    void onUpdateFinished();
};

#endif // LAUNCHER_H
