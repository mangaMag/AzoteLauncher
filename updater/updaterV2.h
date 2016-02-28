#ifndef UPDATER_H
#define UPDATER_H

#define URL "http://127.0.0.1/client/info.json"

#include <QThread>
#include <QElapsedTimer>
#include <QJsonObject>
#include "../http/http.h"
#include "../logger/logger.h"

class UpdaterV2 : public QThread
{
    Q_OBJECT

private:
    Logger* log;
    bool continueUpgrading;
    //Http* http;
    QElapsedTimer downloadTime;

    void run();
    /*bool isNeedUpdate(QString name, QString md5);
    void updateFile(QString name, QString url);*/

    ///////////////////////////////////

    void processUpdate();
    QJsonObject getInfoFile(Http* http);
    int getCurrentVersion();
    QJsonObject getUpdateFile(Http* http, QString url);
    bool checkIfFileRequireUpdate(QString path, QString md5);
    bool updateGameFile(Http* http, QString path, QString url);

public:
    explicit UpdaterV2(QThread* parent = 0);
    ~UpdaterV2();
    void stopProcess();

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

signals:
    void updateProgressBarTotal(const int value);
    void updateProgressBarFile(const int value);
    void updateDownloadSpeed(const QString speed);

};

#endif // UPDATER_H
