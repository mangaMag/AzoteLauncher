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
    if(!http.get(URL"update.json"))
    {
        log->error(http.error());
        return;
    }

    QByteArray file = http.data();
    QJsonDocument updateJson = QJsonDocument::fromJson(file);

    if(updateJson.isNull())
    {
        log->error("update file is empty");
        return;
    }

    QJsonObject update = updateJson.object();

    int build = update.value("build").toInt();
    QJsonArray files = update.value("files").toArray();
    int filesCount = files.count();

    log->info(QString("Build version: %1").arg(build));
    log->info(QString("Number of files: %1").arg(filesCount));

    int i = 1;

    foreach(const QJsonValue& file, files)
    {
        if(!flagRun)
            break;

        QJsonObject obj = file.toObject();

        QString name = obj.value("name").toString();
        QString md5 = obj.value("md5").toString();

        //log->debug(QString("File: %1 (%2)").arg(name).arg(md5));
        updateFile(name);

        emit updateProgressBar(i * 100 / filesCount);
        i++;
    }
}

void Updater::stopProcess()
{
    flagRun = false;
}

void Updater::updateFile(QString fileName)
{
    if(!http.get(QString(URL"%2").arg(fileName)))
    {
        log->error(QString("unable to download file %1<br />%2").arg(fileName).arg(http.error()));
        return;
    }

    QByteArray data = http.data();
    QFile file(fileName);

    if(!file.open(QIODevice::WriteOnly))
    {
        log->error(QString("unable to write file %1<br />%2").arg(fileName).arg(file.errorString()));
        return;
    }

    QDataStream out(&file);
    out.writeRawData(data.data(), data.length());
    file.close();

    log->success(QString("File %1 has been updated").arg(fileName));
}
