#include "updater.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDataStream>

Updater::Updater(QThread* parent) :
    QThread(parent),
    flagRun(true)
{
    log = &Singleton<Logger>::getInstance();
}

void Updater::run()
{
    if(http.get(QString("%1%2").arg(URL).arg("update.json")))
    {
        QByteArray file = http.data();
        QJsonDocument updateJson = QJsonDocument::fromJson(file);

        if(!updateJson.isNull())
        {
            QJsonObject update = updateJson.object();

            int build = update.value("build").toInt();
            QJsonArray files = update.value("files").toArray();

            log->debug(QString("Build version: %1").arg(build));

            foreach(const QJsonValue& file, files)
            {
                QJsonObject obj = file.toObject();

                QString name = obj.value("name").toString();
                QString md5 = obj.value("md5").toString();

                log->debug(QString("File: %1 (%2)").arg(name).arg(md5));
                updateFile(name);
            }
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

void Updater::updateFile(QString fileName)
{
    if(http.get(QString("%1%2").arg(URL).arg(fileName)))
    {
        QByteArray data = http.data();

        QFile file(fileName);
        if(file.open(QIODevice::WriteOnly))
        {
            QDataStream out(&file);
            out.writeRawData(data.data(), data.length());
            file.close();

            log->success(QString("File %1 has been updated").arg(fileName));
        }
        else
            log->error(QString("unable to write file %1<br />%2").arg(fileName).arg(file.errorString()));
    }
    else
        log->error(QString("unable to download file %1<br />%2").arg(fileName).arg(http.error()));
}
