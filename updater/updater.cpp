#include "updater.h"
#include <QJsonDocument>
#include <QFile>
#include <QDataStream>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QProcess>

Updater::Updater(QString _serverName, QThread* parent) :
    QThread(parent),
    continueUpgrading(true),
    currentClientVersion(0),
    bPause(false),
    serverName(_serverName)
{
    log = &Singleton<Logger>::getInstance();
    os  = System::get();
}

Updater::~Updater()
{
}

void Updater::run()
{
    getCurrentVersionFromConfig();

    Http* http = new Http();

    processUpdate(http);

    if (continueUpgrading)
    {
        log->info(QString("Le client est à jour (version: %1)").arg(currentClientVersion));
        emit updateDownloadSpeed("0 o/s");
        emit updateStatus("Le client est à jour");
        emit updateFinished();
        emit updateProgressBarTotal(100);
    }

    http->deleteLater();
    settings->deleteLater();
}

void Updater::stopProcess()
{
    continueUpgrading = false;
}

void Updater::getCurrentVersionFromConfig()
{
    settings = new QSettings(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    currentClientVersion = settings->value(serverName + "/version", 0).toInt();
}

void Updater::processUpdate(Http* http)
{
    if (!continueUpgrading)
    {
        return;
    }

    emit updateStatus("Téléchargement des informations de mise à jour");

    connect(http, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress(qint64,qint64)));

    QJsonObject infoFile = getInfoFile(http);

    if (infoFile.isEmpty())
    {
        log->error("Impossible de récupérer le fichier d'information des mises à jour");
        http->deleteLater();
        return;
    }

    int lastVersion = infoFile.value("version").toInt();
    int numberOfUpdates = lastVersion - currentClientVersion;
    bool isUpdateFailed = false;

    if (currentClientVersion < lastVersion)
    {
        progressStep = 100 / numberOfUpdates;
        updateCounter = 1;

        QFileInfo parentDir(QCoreApplication::applicationDirPath() + "/../");

        if (!parentDir.isWritable())
        {
            log->error("Vous ne disposez pas des droits d'écriture, relancez en Administrateur ou déplacez le dossier du jeu");
            return;
        }

        for (int tempVersion = lastVersion; tempVersion > currentClientVersion; tempVersion--)
        {
            sync.lock();
            if (bPause)
            {
                pauseCond.wait(&sync);
            }
            sync.unlock();

            if (!continueUpgrading)
            {
                return;
            }

            QString url = QString("%1/%2/%3").arg(URL).arg(serverName).arg(tempVersion);
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

            QString osString;

            if (os == WINDOWS)
            {
                osString = "win";
            }
            else if (os == MAC)
            {
                osString = "mac";
            }
            else
            {
                // TODO: error stop
            }

            QString prefix = updateFile.value("prefix").toObject().value(osString).toString();

            QJsonArray commonFiles = updateFile.value("common").toArray();
            QJsonArray osFiles     = updateFile.value(osString).toArray();

            filesCount  = commonFiles.count();
            filesCount += osFiles.count();

            log->info(QString("La mise à jour %1 comporte %2 fichier(s)").arg(tempVersion).arg(filesCount));

            fileCounter = 1;

            emit updateStatus("Vérification des fichiers en cours");

            bool isUpdatedCommon = updateGameFiles(http, url, commonFiles, prefix, "common");
            bool isUpdatedOS     = updateGameFiles(http, url, osFiles,     "",      osString);

            if (!isUpdatedCommon || !isUpdatedOS)
            {
                isUpdateFailed = true;
            }

            updateCounter++;
        }
    }

    sync.lock();
    if (bPause)
    {
        pauseCond.wait(&sync);
    }
    sync.unlock();

    if (continueUpgrading && !isUpdateFailed)
    {
        currentClientVersion = lastVersion;

        settings->setValue(serverName + "/version", lastVersion);
        settings->sync();
    }
    else
    {
        emit updateStatus("Echec de la mise à jour, relancez le launcher");
        log->error("Certains fichiers n'ont pas pu être mis à jour, tentez de relancer le launcher ou d'activier le mode réparation dans les paramètres");
    }
}

bool Updater::updateGameFiles(Http* http, QString url, QJsonArray files, QString pathPrefix, QString urlPrefix)
{
    int fails = 0;

    foreach (const QJsonValue& file, files)
    {
        sync.lock();
        if (bPause)
        {
            pauseCond.wait(&sync);
        }
        sync.unlock();

        if (!continueUpgrading)
        {
            return false;
        }

        QJsonObject fileObject = file.toObject();

        QString name = fileObject.value("name").toString();
        QString md5  = fileObject.value("md5").toString();
        QString nameWithPrefix = pathPrefix + name;

        //log->debug(QString("%1 %2").arg(nameWithPrefix).arg(md5));

        if (checkIfFileRequireUpdate(nameWithPrefix, md5))
        {
            emit updateStatus(QString("Mise à jour de %1").arg(nameWithPrefix));

            if (updateGameFile(http, url, nameWithPrefix, urlPrefix + "/" + name))
            {
                //emit updateStatus(QString("Le fichier %1 a été mis à jour").arg(nameWithPrefix));
            }
            else
            {
                if (nameWithPrefix == QString("%1.app/Contents/MacOS/Dofus").arg(serverName) ||
                    nameWithPrefix == QString("%1.app/Contents/Resources/Reg.app/Contents/MacOS/Reg").arg(serverName))
                {
                    log->warning(QString("Fichier binaire %1 non mis à jour").arg(nameWithPrefix));
                }
                else
                {
                    log->error(QString("Impossible de télécharger ou d'écrire le fichier %1 sur le disque").arg(nameWithPrefix));
                    fails++;
                }
            }
        }

        emit updateProgressBarTotal(fileCounter * (progressStep * updateCounter) / filesCount);

        fileCounter++;
    }

    return (fails > 0 ? false : true);
}

QJsonObject Updater::getInfoFile(Http *http)
{
    QString url = QString("%1/%2/%3").arg(URL).arg(serverName).arg("info.json");

    if (!http->get(url))
    {
        log->debug(http->error());
        return QJsonObject();
    }

    QByteArray file = http->data();
    QJsonDocument json = QJsonDocument::fromJson(file);

    if (json.isNull())
    {
        log->debug("info file json is null");
        return QJsonObject();
    }

    return json.object();
}

QJsonObject Updater::getUpdateFile(Http *http, QString url)
{
    if (!http->get(url + "/update.json"))
    {
        log->debug(http->error());
        return QJsonObject();
    }

    QByteArray file = http->data();
    QJsonDocument json = QJsonDocument::fromJson(file);

    if (json.isNull())
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
    QFile file(QCoreApplication::applicationDirPath() + "/" + path);

    if (!file.open(QIODevice::ReadOnly))
    {
        return true;
    }

    hash.addData(file.readAll());
    file.close();

    if(md5.compare(hash.result().toHex().data(), Qt::CaseInsensitive) == 0)
    {
        //log->debug(QString("%1 OK").arg(path));
        updatedFiles.append(path);
        return false;
    }

    return true;
}

bool Updater::updateGameFile(Http* http, QString url, QString name, QString urlName)
{
    downloadTime.start();

    if (!http->get(url + "/files/" + urlName))
    {
        log->debug(http->error());
        return false;
    }

    QByteArray data = http->data();
    QFile file(QCoreApplication::applicationDirPath() + "/" + name);
    QFileInfo fileInfo(QCoreApplication::applicationDirPath() + "/" + name);

    if (!fileInfo.dir().exists())
    {
        if (!fileInfo.dir().mkpath("."))
        {
            stopProcess();
            log->error("Vous ne disposez pas des droits d'écriture, relancez en Administrateur ou déplacez le dossier du jeu");
            return false;
        }
    }

    if(!file.open(QIODevice::WriteOnly))
    {
        log->debug(file.errorString());
        return false;
    }

    QDataStream out(&file);
    out.writeRawData(data.data(), data.length());
    file.close();

    updatedFiles.append(name);

    return true;
}

void Updater::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bPause)
    {
        emit updateDownloadSpeed(QString("0 o/s"));
        return;
    }

    if (bytesTotal <= 0)
    {
        return;
    }

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
}

void Updater::resume()
{
    sync.lock();
    bPause = false;
    sync.unlock();
    pauseCond.wakeAll();
}

void Updater::pause()
{
    sync.lock();
    emit updateStatus("Téléchargement mis en pause");
    bPause = true;
    sync.unlock();
}

bool Updater::isNeedUpdate()
{
    bool isNeedUpdate = true;
    getCurrentVersionFromConfig();
    Http* http = new Http();

    QJsonObject infoFile = getInfoFile(http);

    if (infoFile.isEmpty())
    {
        log->error("Impossible de récupérer le fichier d'information des mises à jour");
        http->deleteLater();
        return true;
    }

    int lastVersion = infoFile.value("version").toInt();

    if (currentClientVersion >= lastVersion)
    {
        isNeedUpdate = false;
    }

    http->deleteLater();
    return isNeedUpdate;
}

int Updater::getCurrentVersion()
{
    return currentClientVersion;
}
