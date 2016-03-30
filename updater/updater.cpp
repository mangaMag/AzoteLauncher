#include "updater.h"
#include <QJsonDocument>
#include <QFile>
#include <QDataStream>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QProcess>

Updater::Updater(QThread* parent) :
    QThread(parent),
    continueUpgrading(true)
{
    log = &Singleton<Logger>::getInstance();
    os  = System::get();

    updateFileName = "update";

    if (os == WINDOWS)
    {
        updateFileName.append(".exe");
    }

}

Updater::~Updater()
{
}

void Updater::run()
{
    log->debug("DEBUG");
    QFile::remove(updateFileName);

    getCurrentVersion();

    Http* http = new Http();

    if (selfUpdate(http))
    {
        processUpdate(http);
    }

    log->info(QString("Le client est à jour (client: %1 launcher: %2)").arg(currentClientVersion).arg(currentLauncherVersion));
    emit updateDownloadSpeed("0 o/s");
    emit updateStatus("Le client est à jour");
    emit enablePlayButton(true);
    emit updateProgressBarTotal(100);

    http->deleteLater();
    settings->deleteLater();
}

void Updater::stopProcess()
{
    continueUpgrading = false;
}

void Updater::getCurrentVersion()
{
    settings = new QSettings("./config.ini", QSettings::IniFormat);

    currentClientVersion   = settings->value("client/version", 0).toInt();
    currentLauncherVersion = LAUNCHER_VERSION;
}

bool Updater::selfUpdate(Http* http)
{
    QString url;

    if (os == WINDOWS)
    {
        url = URL "/win";
    }
    else if (os == MAC)
    {
        url = URL "/mac";
    }
    else
    {
        // TODO: error stop
    }

    if(!http->get(url + "/updater.dat"))
    {
        log->debug(http->error());
    }

    bool ok;
    int launcherVersion = http->data().toInt(&ok);

    if (ok)
    {
        if (launcherVersion > currentLauncherVersion)
        {
            if(!http->get(url + "/" + updateFileName))
            {
                log->debug(http->error());
                return false;
            }

            QByteArray data = http->data();
            QFile file(QCoreApplication::applicationDirPath() + "/" + updateFileName);

            if(!file.open(QIODevice::WriteOnly))
            {
                log->debug(file.errorString());
                return false;
            }

            QDataStream out(&file);
            out.writeRawData(data.data(), data.length());
            file.close();

            if (!file.setPermissions(QFile::ReadOwner  |
                                     QFile::WriteOwner |
                                     QFile::ExeOwner   |
                                     QFile::ReadGroup  |
                                     QFile::ExeGroup   |
                                     QFile::ReadOther  |
                                     QFile::ExeOther))
            {
                log->error("Impossible de mettre à jour le launcher (Permissions)");
                return false;
            }


            QStringList params;

            params << "--selfupdate";
            params << QString("--path=%1").arg(QCoreApplication::applicationDirPath());

            QProcess* process = new QProcess(this);
            if (process->startDetached(file.fileName(), params, QCoreApplication::applicationDirPath()))
            {
                QCoreApplication::quit();
            }
            else
            {
                log->error(process->errorString());
                log->error("Impossible de mettre à jour le launcher (Execution)");
                return false;
            }
        }

        return true;
    }
    else
    {
        log->error("Impossible de récupérer le fichier d'information de la version du launcher");
    }

    return true;
}

void Updater::processUpdate(Http* http)
{
    if (!continueUpgrading)
    {
        return;
    }

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
            if(!continueUpgrading)
            {
                return;
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

            updateGameFiles(http, url, commonFiles, prefix, "common");
            updateGameFiles(http, url, osFiles,     "",     osString);

            updateCounter++;
        }
    }

    if (continueUpgrading)
    {
        currentClientVersion = lastVersion;

        settings->setValue("client/version", lastVersion);
        settings->sync();
    }
}

void Updater::updateGameFiles(Http* http, QString url, QJsonArray files, QString pathPrefix, QString urlPrefix)
{
    foreach(const QJsonValue& file, files)
    {
        if(!continueUpgrading)
        {
            return;
        }

        QJsonObject fileObject = file.toObject();

        QString name = fileObject.value("name").toString();
        QString md5  = fileObject.value("md5").toString();
        QString nameWithPrefix = pathPrefix + name;

        //log->debug(QString("%1 %2").arg(nameWithPrefix).arg(md5));

        if (checkIfFileRequireUpdate(nameWithPrefix, md5))
        {
            if (updateGameFile(http, url, nameWithPrefix, urlPrefix + "/" + name))
            {
                emit updateStatus(QString("Le fichier %1 a été mis à jour").arg(nameWithPrefix));
            }
            else
            {
                log->error(QString("Impossible d'écrire le fichier %1 sur le disque").arg(nameWithPrefix));
            }
        }

        emit updateProgressBarTotal(fileCounter * (progressStep * updateCounter) / filesCount);

        fileCounter++;
    }
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
    QFile file(QCoreApplication::applicationDirPath() + "/../" + path);

    if(!file.open(QIODevice::ReadOnly))
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

    if(!http->get(url + "/files/" + urlName))
    {
        log->debug(http->error());
        return false;
    }

    QByteArray data = http->data();
    QFile file(QCoreApplication::applicationDirPath() + "/../" + name);
    QFileInfo fileInfo(QCoreApplication::applicationDirPath() + "/../" + name);

    if(!fileInfo.dir().exists())
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
    if(bytesTotal <= 0)
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
