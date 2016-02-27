#ifndef UPDATER_H
#define UPDATER_H

#define URL "http://127.0.0.1/client"

#include <QThread>
#include <QElapsedTimer>
#include "../http/http.h"
#include "../logger/logger.h"

class UpdaterV2 : public QThread
{
    Q_OBJECT

private:
    Logger* log;
    bool flagRun;
    Http* http;
    QElapsedTimer downloadTime;

    void run();
    bool isNeedUpdate(QString name, QString md5);
    void updateFile(QString name);

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
