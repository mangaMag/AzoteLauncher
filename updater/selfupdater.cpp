#include "selfupdater.h"
//#include "../logger/logger.h"

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
                newPath.append("/launcher.exe");

                QFile::remove(newPath);
                QFile::copy(tempPath, newPath);

                QProcess* process = new QProcess(this);
                process->startDetached(newPath, QStringList(), path);

                return true;
            }
        }
    }

    return false;
}
