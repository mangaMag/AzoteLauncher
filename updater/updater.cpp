#include "updater.h"
#include <QJsonDocument>
#include <QJsonArray>
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

    #ifdef _WIN32
        updateFileName = "update.exe";
    #else
        updateFileName = "update";
    #endif

    currentLauncherVersion = 3;
}

Updater::~Updater()
{
}

void Updater::run()
{
    QFile::remove(updateFileName);

    getCurrentVersion();

    Http* http = new Http();

    if (selfUpdate(http))
    {
        processUpdate(http);
    }

    log->info("Le client est à jour (" + QString::number(currentLauncherVersion) + ")");
    emit updateDownloadSpeed("0 o/s");
    emit updateStatus("Le client est à jour");
    emit enablePlayButton(true);
    emit updateProgressBarTotal(100);

    delete http;
    delete settings;
}

void Updater::stopProcess()
{
    continueUpgrading = false;
}

void Updater::getCurrentVersion()
{
    settings = new QSettings("./config.ini", QSettings::IniFormat);

    currentClientVersion   = settings->value("client/version", 0).toInt();
}

bool Updater::selfUpdate(Http* http)
{
    if(!http->get(URL "/updater.dat"))
    {
        log->debug(http->error());
    }

    bool ok;
    int launcherVersion = http->data().toInt(&ok);

    if (ok)
    {
        if (launcherVersion > currentLauncherVersion)
        {
            if(!http->get(URL "/" + updateFileName))
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
        int progressStep = 100 / numberOfUpdates;
        int updateCounter = 1;

        QFileInfo parentDir(QCoreApplication::applicationDirPath() + "/../");

        if (!parentDir.isWritable())
        {
            stopProcess();
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

            QJsonArray files = updateFile.value("files").toArray();
            int filesCount = files.count();
            log->info(QString("La mise à jour %1 comporte %2 fichier(s)").arg(tempVersion).arg(filesCount));

            int fileCounter = 1;

            foreach(const QJsonValue& file, files)
            {
                if(!continueUpgrading)
                {
                    return;
                }

                QJsonObject fileObject = file.toObject();

                QString name = fileObject.value("name").toString();
                QString md5 = fileObject.value("md5").toString();

                //log->debug(QString("%1 %2").arg(name).arg(md5));

                if (checkIfFileRequireUpdate(name, md5))
                {
                    if (updateGameFile(http, name, url))
                    {
                        emit updateStatus(QString("Le fichier %1 a été mis à jour").arg(name));
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
    settings->sync();
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
    QFile file(QCoreApplication::applicationDirPath() + "/../" + path);
    QFileInfo fileInfo(QCoreApplication::applicationDirPath() + "/../" + path);

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

    updatedFiles.append(path);

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
