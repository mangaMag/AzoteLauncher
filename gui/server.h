#ifndef SERVER_H
#define SERVER_H

#include <QWidget>
#include <QMap>
#include <QString>

class Launcher;
class Updater;
class Logger;

enum State {
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
    explicit Server(QWidget* parent, Launcher* _launcher, QString _name);
    ~Server();

private:
    Launcher* launcher;
    Ui::Server* ui;
    Updater* updater;
    Logger* log;
    QMap<QObject*, QUrl> urls;
    State state;
    QString name;
    QString stylePlay;

    void startGame(QString gamePath);
    void startUpdate();
    void pauseUpdater();

private slots:
    void onUpdateFinished();
    void onClickPlayButton();
    void onClickSettingsButton();
    void onClickLinkButton();
    void onClickResumePauseButton();
};

#endif // SERVER_H
