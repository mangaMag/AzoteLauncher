#include "updater.h"
#include "../logger/logger.h"
#include "../http/http.h"
#include <QJsonDocument>
#include <QJsonObject>

Updater::Updater(QThread* parent) :
    QThread(parent),
    flagRun(true)
{
}

void Updater::run()
{
    Logger* log = &Singleton<Logger>::getInstance();
    Http http;

    if(http.get("http://127.0.0.1/client/update.json"))
    {
        QByteArray file = http.data();
        QJsonDocument updateJson = QJsonDocument::fromJson(file);

        if(!updateJson.isNull())
        {
            QJsonObject update = updateJson.object();
            log->debug(QString::number(update.value("build").toInt()));
        }
        else
            log->error("update file is empty");
    }
    else
        log->error(http.error());

    for(int i = 1; i <= 100; i++)
    {
        if(flagRun)
        {
            log->debug(QString("%1%").arg(i));
            emit updateProgressBar(i);
            msleep(100);
        }
    }
}

void Updater::stopProcess()
{
    flagRun = false;
}
