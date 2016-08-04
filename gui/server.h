#ifndef SERVER_H
#define SERVER_H

#include <QWidget>
#include <QMap>
#include <QString>

class Launcher;
class Updater;
class Logger;

namespace Ui {
class Server;
}

class Server : public QWidget
{
    Q_OBJECT

public:
    explicit Server(QWidget* parent, Launcher* _launcher, QString name);
    ~Server();

private:
    Launcher* launcher;
    Ui::Server *ui;
    Updater* updater;
    Logger* log;
    QMap<QObject*, QUrl> urls;

    void startGame(QString gamePath);
    void startUpdate();

private slots:
    void onUpdateFinished();
    void onNewUpdaterVersion();
    void onClickPlayButton();
    void onClickSettingsButton();
    void onClickLinkButton();
};

#endif // SERVER_H
