#include "updaterV2.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDataStream>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QDir>

UpdaterV2::UpdaterV2(QThread* parent) :
    QThread(parent),
    flagRun(true)
{
    log = &Singleton<Logger>::getInstance();
}

UpdaterV2::~UpdaterV2()
{
}

void UpdaterV2::run()
{
    http = new Http();
    connect(http, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress(qint64,qint64)));

    if(!http->get(URL"/update.json"))
    {
        log->error(http->error());
        return;
    }

    QByteArray file = http->data();
    QJsonDocument json = QJsonDocument::fromJson(file);

    if(json.isNull())
    {
        log->error("update file is empty");
        return;
    }

    QJsonObject update = json.object();

    int lastBuild = update.value("build").toInt();
    QString url   = update.value("url").toString();

    url = QString("http://%1/%2").arg(url).arg(QString::number(lastBuild));

    log->info(QString("Last build version: %1").arg(lastBuild));
    log->info(QString("URL: %1").arg(url));

    if(!http->get(url + QString("/data.json")))
    {
        log->error(http->error());
        return;
    }

    file = http->data();
    json = QJsonDocument::fromJson(file);

    if(json.isNull())
    {
        log->error("update file is empty");
        return;
    }

    QJsonObject data = json.object();

    int currentBuild = data.value("build").toInt();
    QJsonArray files = data.value("files").toArray();
    int filesCount = files.count();

    log->info(QString("Current build version: %1").arg(currentBuild));
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
        {
            updateFile(name, url + QString("/files/") + name);
        }

        emit updateProgressBarTotal(i * 100 / filesCount);
        i++;
    }

    log->info("Client is up to date");
    emit updateDownloadSpeed("Terminée");

    delete http;
}

void UpdaterV2::stopProcess()
{
    flagRun = false;
}

bool UpdaterV2::isNeedUpdate(QString name, QString md5)
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    QFile file(name);

    if(!file.open(QIODevice::ReadOnly))
    {
        return true;
    }

    hash.addData(file.readAll());
    file.close();

    if(md5.compare(hash.result().toHex().data(), Qt::CaseInsensitive) == 0)
    {
        log->debug(QString("%1 OK").arg(name));
        return false;
    }

    return true;
}

void UpdaterV2::updateFile(QString name, QString url)
{
    downloadTime.start();

    if(!http->get(url))
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

void UpdaterV2::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
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
