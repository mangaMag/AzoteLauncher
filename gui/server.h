#ifndef SERVER_H
#define SERVER_H

#include <QWidget>
#include <QMap>
#include <QString>

#include "utils/system.h"

class Launcher;
class Updater;
class Logger;
class Sound;

enum State {
    NO_STARTED,
    PAUSE,
    RESUME,
    FINISHED
};

namespace Ui {
class Server;
}

class Server : public QWidget
{
    Q_OBJECT

public:
    explicit Server(QWidget* parent, Launcher* _launcher, QString _name, float _version);
    ~Server();

private:
    Launcher* launcher;
    Ui::Server* ui;
    Updater* updater;
    Logger* log;
    QMap<QObject*, QUrl> urls;
    State state;
    QString name;
    float version;
    QString stylePlay;
    Sound* sound;
    int port;
    bool isRegStarted;
    OperatingSystem os;

    void startProcess(QString processName, QStringList args, bool forceDetached);
    void startGame();
    void startSound();
    void startUpdate();
    void pauseUpdater();
    void createUpdater();
    void checkUpdate();

private slots:
    void onUpdateFinished();
    void onClickPlayButton();
    void onClickSettingsButton();
    void onClickLinkButton();
    void onClickResumePauseButton();
    void onRepairStarted();
};

#endif // SERVER_H
