#ifndef SELFUPDATER_H
#define SELFUPDATER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class Logger;
class QSettings;

class SelfUpdater : public QThread
{
    Q_OBJECT

public:
    explicit SelfUpdater(QThread *parent = 0);
    void getCurrentVersion();
    void checkUpdate();
    bool isUpdateAsked(int argc, char *argv[]);
    void update(QString currentPath);
    void stopProcess();
    void resume();
    void run();

private:
    Logger* log;
    QSettings* settings;
    int currentLauncherVersion;
    bool continueUpgrading;
    QMutex sync;
    QWaitCondition pauseCond;

signals:
    void newUpdaterVersion();

public slots:
};

#endif // SELFUPDATER_H
