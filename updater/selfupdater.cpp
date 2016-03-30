#include "selfupdater.h"
//#include "../logger/logger.h"
#include "../utils/system.h"

#include <QCoreApplication>
#include <QProcess>
#include <QFile>
#include <QThread>

SelfUpdater::SelfUpdater(QObject *parent) : QObject(parent)
{
}

bool SelfUpdater::update(int argc, char *argv[])
{
    //Logger* log = &Singleton<Logger>::getInstance();
    //log->showConsole();
    OperatingSystem os = System::get();

    QString launcherName = "ArkalysUpdater";

    if (os == WINDOWS)
    {
        launcherName.append(".exe");
    }

    for (int i = 1; i < argc; i++)
    {
        if (QString::compare("--selfupdate", argv[i]) == 0)
        {
            QString path(argv[i + 1]);
            QString tempPath(argv[0]);

            if (path.contains("--path="))
            {
                QThread::sleep(2); // wait for launcher closed

                path.remove("--path=");

                QString newPath = path;
                newPath.append("/" + launcherName);

                QFile::remove(newPath);
                QFile::copy(tempPath, newPath);

                QFile(newPath).setPermissions(QFile::ReadOwner |
                                    QFile::WriteOwner |
                                    QFile::ExeOwner |
                                    QFile::ReadGroup |
                                    QFile::ExeGroup |
                                    QFile::ReadOther |
                                    QFile::ExeOther);

                QProcess* process = new QProcess(this);
                process->startDetached(newPath, QStringList(), path);

                return true;
            }
        }
    }

    return false;
}
