#ifndef UPDATER_H
#define UPDATER_H

#define URL "http://cache.arkalys.com/updater"

#include <QThread>
#include <QElapsedTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QSettings>

#include "../http/http.h"
#include "../logger/logger.h"
#include "../utils/system.h"

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

    int filesCount;
    int progressStep;
    int updateCounter;
    int fileCounter;

    void run();
    bool selfUpdate(Http* http);
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

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

signals:
    void updateProgressBarTotal(const int value);
    void updateProgressPercent(const QString value);
    void updateDownloadSpeed(const QString speed);
    void updateStatus(const QString status);
    void enablePlayButton(bool state);
};

#endif // UPDATER_H
