#ifndef UPDATER_H
#define UPDATER_H

#define URL "http://cdn.azendar.fr/updater"

#include <QThread>
#include <QElapsedTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QSettings>
#include <QMutex>
#include <QWaitCondition>

#include "../http/http.h"
#include "../logger/logger.h"
#include "../utils/system.h"

enum SelfUpdateStatus
{
    OK_TO_CONTINUE,
    UPDATE_FAILED,
    UPDATE_IN_PROGRESS
};

class Updater : public QThread
{
    Q_OBJECT

private:
    Logger* log;
    volatile bool continueUpgrading;
    QElapsedTimer downloadTime;
    QVector<QString> updatedFiles;
    int currentClientVersion;
    int currentLauncherVersion;
    QString updateFileName;
    QSettings* settings;
    OperatingSystem os;
    QMutex sync;
    QWaitCondition pauseCond;

    int filesCount;
    int progressStep;
    int updateCounter;
    int fileCounter;

    void run();
    SelfUpdateStatus selfUpdate(Http* http);
    void processUpdate(Http* http);
    void getCurrentVersion();
    QJsonObject getInfoFile(Http* http);
    QJsonObject getUpdateFile(Http* http, QString url);
    void updateGameFiles(Http* http, QString url, QJsonArray files, QString pathPrefix, QString urlPrefix);
    bool checkIfFileRequireUpdate(QString path, QString md5);
    bool updateGameFile(Http* http, QString url, QString name, QString urlName);

public:
    explicit Updater(QThread* parent = 0);
    ~Updater();
    void stopProcess();
    void resume();
    void pause();

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

signals:
    void updateProgressBarTotal(const int value);
    void updateProgressPercent(const QString value);
    void updateDownloadSpeed(const QString speed);
    void updateStatus(const QString status);
    void enablePlayButton(bool state);
    void newUpdaterVersion();
};

#endif // UPDATER_H
