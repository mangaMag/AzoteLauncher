#include "selfupdater.h"
//#include "../logger/logger.h"
#include "../utils/system.h"

#include <QCoreApplication>
#include <QProcess>
#include <QFile>
#include <QThread>
#include <QFileInfo>
#include <QDir>

#include <QDebug>

SelfUpdater::SelfUpdater(QObject *parent) : QObject(parent)
{
    //Logger* log = &Singleton<Logger>::getInstance();
    //log->showConsole();
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
    QString launcherName = "AzendarUpdater";

    if (os == WINDOWS)
    {
        launcherName.append(".exe");
    }

    QFileInfo currentBin(currentPath);

    QString newPath = currentBin.absoluteDir().path();
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

    QProcess::startDetached(newPath);
}
