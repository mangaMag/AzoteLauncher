#ifndef UPDATER_H
#define UPDATER_H

#define URL "https://cdn.arkalys.com"

#include <QThread>
#include <QElapsedTimer>
#include <QJsonObject>
#include <QVector>

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

    void run();
    void processUpdate();
    QJsonObject getInfoFile(Http* http);
    int getCurrentVersion();
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
    void updateProgressBarFile(const int value);
    void updateDownloadSpeed(const QString speed);
};

#endif // UPDATER_H
