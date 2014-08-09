#include "updater.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDataStream>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QDir>

Updater::Updater(QThread* parent) :
    QThread(parent),
    flagRun(true)
{
    log = &Singleton<Logger>::getInstance();
}

Updater::~Updater()
{
}

void Updater::run()
{
    http = new Http();
    connect(http, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress(qint64,qint64)));

    if(!http->get(URL"update.json"))
    {
        log->error(http->error());
        return;
    }

    QByteArray file = http->data();
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

        if(isNeedUpdate(name, md5))
            updateFile(name);

        emit updateProgressBarTotal(i * 100 / filesCount);
        i++;
    }

    delete http;
}

void Updater::stopProcess()
{
    flagRun = false;
}

bool Updater::isNeedUpdate(QString name, QString md5)
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    QFile file(name);

    if(!file.open(QIODevice::ReadOnly))
        return true;

    hash.addData(file.readAll());
    file.close();

    if(md5.compare(hash.result().toHex().data(), Qt::CaseInsensitive) == 0)
        return false;

    return true;
}

void Updater::updateFile(QString name)
{
    downloadTime.start();

    if(!http->get(QString(URL"%2").arg(name)))
    {
        log->error(QString("unable to download file %1<br />%2").arg(name).arg(http->error()));
        return;
    }

    QByteArray data = http->data();
    QFile file(name);
    QFileInfo fileInfo(name);

    if(!fileInfo.dir().exists())
        fileInfo.dir().mkpath(".");

    if(!file.open(QIODevice::WriteOnly))
    {
        log->error(QString("unable to write file %1<br />%2").arg(name).arg(file.errorString()));
        return;
    }

    QDataStream out(&file);
    out.writeRawData(data.data(), data.length());
    file.close();

    log->success(QString("File %1 has been updated").arg(name));
}

void Updater::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if(bytesTotal <= 0)
        return;

    double speed = bytesReceived * 1000.0 / downloadTime.elapsed();
    QString unit;

    if (speed < 1024)
    {
        unit = "o/s";
    }
    else if (speed < 1024*1024)
    {
        speed /= 1024;
        unit = "Ko/s";
    }
    else
    {
        speed /= 1024*1024;
        unit = "Mo/s";
    }

    emit updateDownloadSpeed(QString("%1 %2").arg(speed, 3, 'f', 1).arg(unit));
    emit updateProgressBarFile(bytesReceived * 100 / bytesTotal);
}
