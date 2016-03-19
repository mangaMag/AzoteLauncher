#ifndef UPDATER_H
#define UPDATER_H

//#define URL "http://cdn.arkalys.com"
#define URL "http://arkalys.s3.amazonaws.com/updater"

#include <QThread>
#include <QElapsedTimer>
#include <QJsonObject>
#include <QVector>
#include <QSettings>

#include "../http/http.h"
#include "../logger/logger.h"

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

    void run();
    bool selfUpdate(Http* http);
    void processUpdate(Http* http);
    void getCurrentVersion();
    QJsonObject getInfoFile(Http* http);
    QJsonObject getUpdateFile(Http* http, QString url);
    bool checkIfFileRequireUpdate(QString path, QString md5);
    bool updateGameFile(Http* http, QString path, QString url);

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
