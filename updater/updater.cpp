#include "updater.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDataStream>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QDir>

#include <QDebug>

Updater::Updater(QThread* parent) :
    QThread(parent),
    continueUpgrading(true)
{
    log = &Singleton<Logger>::getInstance();
}

Updater::~Updater()
{
}

void Updater::run()
{
    getCurrentVersion();

    Http* http = new Http();

    selfUpdate(http);
    processUpdate(http);

    log->info("Le client est à jour");
    emit updateDownloadSpeed("Terminée");
    emit enablePlayButton(true);

    delete http;
    delete settings;
}

void Updater::stopProcess()
{
    continueUpgrading = false;
}

void Updater::getCurrentVersion()
{
    settings = new QSettings("config.ini", QSettings::IniFormat);

    currentClientVersion   = settings->value("client/version", QVariant(0)).toInt();
    currentLauncherVersion = settings->value("launcher/version", QVariant(0)).toInt();
}

void Updater::selfUpdate(Http* http)
{
    if(!http->get(URL "/updater"))
    {
        log->debug(http->error());
    }

    bool ok;
    int launcherVersion = http->data().toInt(&ok);

    if (ok)
    {
        if (launcherVersion > currentLauncherVersion)
        {
            // TODO: download new bin

            settings->setValue("launcher/version", launcherVersion);
        }
    }
    else
    {
        log->error("Impossible de récupérer le fichier d'information de la version du launcher");
    }
}

void Updater::processUpdate(Http* http)
{
    connect(http, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress(qint64,qint64)));

    QJsonObject infoFile = getInfoFile(http);

    if (infoFile.isEmpty())
    {
        log->error("Impossible de récupérer le fichier d'information des mises à jour");
        delete http;
        return;
    }

    int lastVersion = infoFile.value("version").toInt();

    int numberOfUpdates = lastVersion - currentClientVersion;

    if (currentClientVersion < lastVersion)
    {
        int progressStep = 100 / numberOfUpdates;
        int updateCounter = 1;

        for (int tempVersion = lastVersion; tempVersion > currentClientVersion; tempVersion--)
        {
            if(!continueUpgrading)
            {
                break;
            }

            QString url = QString("%1/%2").arg(URL).arg(tempVersion);

            QJsonObject updateFile = getUpdateFile(http, url);

            if (updateFile.isEmpty())
            {
                log->warning(QString("Impossible de récupérer le fichier de mise à jour version %1").arg(tempVersion));
                updateCounter++;
                continue;
            }

            int updateFileVersion = updateFile.value("version").toInt();

            if (tempVersion != updateFileVersion)
            {
                log->warning(QString("Le numéro de version de la mise à jour %1 ne correspond pas au numéro attendu %2").arg(updateFileVersion).arg(tempVersion));
                updateCounter++;
                continue;
            }

            QJsonArray files = updateFile.value("files").toArray();
            int filesCount = files.count();
            log->info(QString("La mise à jour %1 comporte %2 fichier(s)").arg(tempVersion).arg(filesCount));

            int fileCounter = 1;

            foreach(const QJsonValue& file, files)
            {
                if(!continueUpgrading)
                {
                    break;
                }

                QJsonObject fileObject = file.toObject();

                QString name = fileObject.value("name").toString();
                QString md5 = fileObject.value("md5").toString();

                //log->debug(QString("%1 %2").arg(name).arg(md5));

                if (checkIfFileRequireUpdate(name, md5))
                {
                    if (updateGameFile(http, name, url))
                    {
                        log->success(QString("Le fichier %1 a était mis à jour").arg(name));
                    }
                    else
                    {
                        log->error(QString("Impossible d'écrire le fichier %1 sur le disque").arg(name));
                    }
                }

                emit updateProgressBarTotal(fileCounter * (progressStep * updateCounter) / filesCount);

                fileCounter++;
            }

            updateCounter++;
        }
    }

    settings->setValue("client/version", lastVersion);
}

QJsonObject Updater::getInfoFile(Http *http)
{
    if(!http->get(URL "/info.json"))
    {
        log->debug(http->error());
        return QJsonObject();
    }

    QByteArray file = http->data();
    QJsonDocument json = QJsonDocument::fromJson(file);

    if(json.isNull())
    {
        log->debug("info file json is null");
        return QJsonObject();
    }

    return json.object();
}

QJsonObject Updater::getUpdateFile(Http *http, QString url)
{
    if(!http->get(url + "/update.json"))
    {
        log->debug(http->error());
        return QJsonObject();
    }

    QByteArray file = http->data();
    QJsonDocument json = QJsonDocument::fromJson(file);

    if(json.isNull())
    {
        log->debug("update file json is null");
        return QJsonObject();
    }

    return json.object();
}

bool Updater::checkIfFileRequireUpdate(QString path, QString md5)
{
    if (updatedFiles.contains(path))
    {
        //log->debug(QString("File %1 are already updated in previous update").arg(path));
        return false;
    }

    QCryptographicHash hash(QCryptographicHash::Md5);
    QFile file(path);

    if(!file.open(QIODevice::ReadOnly))
    {
        return true;
    }

    hash.addData(file.readAll());
    file.close();

    if(md5.compare(hash.result().toHex().data(), Qt::CaseInsensitive) == 0)
    {
        // log->debug(QString("%1 OK").arg(name));
        updatedFiles.append(path);
        return false;
    }

    return true;
}

bool Updater::updateGameFile(Http* http, QString path, QString url)
{
    downloadTime.start();

    if(!http->get(url + "/files/" + path))
    {
        log->debug(http->error());
        return false;
    }

    QByteArray data = http->data();
    QFile file(path);
    QFileInfo fileInfo(path);

    if(!fileInfo.dir().exists())
    {
        fileInfo.dir().mkpath(".");
    }

    if(!file.open(QIODevice::WriteOnly))
    {
        log->debug(file.errorString());
        return false;
    }

    QDataStream out(&file);
    out.writeRawData(data.data(), data.length());
    file.close();

    updatedFiles.append(path);

    return true;
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
