#ifndef UPDATER_H
#define UPDATER_H

#define URL "http://127.0.0.1/client/"

#include <QThread>
#include "../http/http.h"
#include "../logger/logger.h"

class Updater : public QThread
{
    Q_OBJECT

private:
    Logger* log;
    bool flagRun;
    Http* http;

    void run();
    bool isNeedUpdate(QString name, QString md5);
    void updateFile(QString name);

public:
    explicit Updater(QThread* parent = 0);
    ~Updater();
    void stopProcess();

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

signals:
    void updateProgressBarTotal(const int value);
    void updateProgressBarFile(const int value);

};

#endif // UPDATER_H
