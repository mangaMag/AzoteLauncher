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

enum TabType {
    HOME,
    SERVER
};

struct tab {
    TabType type;
    QString name;
    Updater* updater;
    QWidget* selector;
};

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
    QMap<QObject*, tab> tabs;
    tab previousTab;

    void closeEvent(QCloseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void changeEvent(QEvent* event);

    void startUpdate();
    void startGame(QString gamePath);

    void switchSelectedTab(tab selectedTab);

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
    void onChangeTab();
};

#endif // LAUNCHER_H
