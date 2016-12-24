#include "selfupdater.h"
//#include "../logger/logger.h"
#include "../utils/system.h"
#include "../gui/settings.h"

#include <QCoreApplication>
#include <QProcess>
#include <QFile>
#include <QThread>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>

#include "http/http.h"
#include "utils/system.h"
#include "logger/logger.h"

SelfUpdater::SelfUpdater(QThread *parent) :
    QThread(parent),
    continueUpgrading(true)
{
    log = &Singleton<Logger>::getInstance();
}

void SelfUpdater::getCurrentVersion()
{
    settings = new QSettings(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    currentLauncherVersion = settings->value("launcher/version", LAUNCHER_VERSION).toInt(); // LAUNCHER_VERSION;
}

void SelfUpdater::checkUpdate()
{
    Http* http = new Http();
    OperatingSystem os  = System::get();
    QString url;
    QString updateFileName = "updater";

    getCurrentVersion();

    if (os == WINDOWS)
    {
        updateFileName.append(".exe");
    }

    QFile::remove(QCoreApplication::applicationDirPath() + "/" + updateFileName);

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
        log->error("Système d'exploitation inconnu");
        return;
    }

    if(!http->get(url + "/updater.dat"))
    {
        log->debug(http->error());
        log->error("Impossible de récupérer le fichier d'information de la version du launcher");
        return;
    }

    bool ok;
    int launcherVersion = http->data().trimmed().toInt(&ok);

    if (ok)
    {
        if (launcherVersion > currentLauncherVersion)
        {
            emit newUpdaterVersion();

            sync.lock();
            pauseCond.wait(&sync);
            sync.unlock();

            if (!continueUpgrading)
            {
                return;
            }

            if(!http->get(url + "/" + updateFileName))
            {
                log->debug(http->error());
                return;
            }

            QByteArray data = http->data();
            QFile file(QCoreApplication::applicationDirPath() + "/" + updateFileName);

            if(!file.open(QIODevice::WriteOnly))
            {
                log->debug(file.errorString());
                return;
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
                return;
            }

            QStringList params;

            params << "--selfupdate";

            QProcess* process = new QProcess(this);
            if (process->startDetached(file.fileName(), params, QCoreApplication::applicationDirPath()))
            {
                QCoreApplication::quit();
            }
            else
            {
                log->error(process->errorString());
                log->error("Impossible de mettre à jour le launcher (Execution)");
                return;
            }
        }

        return;
    }
    else
    {
        log->error("Impossible de récupérer le numéro la nouvelle version du launcher");
    }

    log->info(QString("Le launcher est à jour (version: %1)").arg(currentLauncherVersion));
}

bool SelfUpdater::isUpdateAsked(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if (QString::compare("--selfupdate", argv[i]) == 0)
        {
            return true;
        }
    }

    return false;
}

void SelfUpdater::update(QString currentPath)
{
    QThread::sleep(2); // wait for launcher closed

    OperatingSystem os = System::get();
    QString launcherName = "Azote";

    if (os == WINDOWS)
    {
        launcherName.append(".exe");
    }

    QFileInfo currentBin(currentPath);

    QString newPath = currentBin.absolutePath();
    newPath.append("/" + launcherName);

    QFile::remove(newPath);
    QFile::copy(currentPath, newPath);

    QFile(newPath).setPermissions(QFile::ReadOwner |
                        QFile::WriteOwner |
                        QFile::ExeOwner |
                        QFile::ReadGroup |
                        QFile::ExeGroup |
                        QFile::ReadOther |
                        QFile::ExeOther);

    QSettings settings(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    StartMode startMode = (StartMode)settings.value("launcher/startMode", DetachedProcress).toInt();

    switch (startMode)
    {
        case Process:
        {
            QProcess* launcher = new QProcess(this);
            launcher->start(newPath);
            break;
        }
        case DetachedProcress:
            QProcess::startDetached(newPath);
            break;
        case DesktopService:
        default:
            QDesktopServices::openUrl(QUrl::fromLocalFile(newPath));
            break;
    }
}

void SelfUpdater::stopProcess()
{
    continueUpgrading = false;
}

void SelfUpdater::resume()
{
    pauseCond.wakeAll();
}

void SelfUpdater::run()
{
    checkUpdate();
}
