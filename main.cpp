#include "gui/launcher.h"
#include "updater/selfupdater.h"
#include <QApplication>
#include "dep/SingleApplication/singleapplication.h"

int main(int argc, char *argv[])
{
    QApplication::setApplicationName("Launcher");
    QApplication::setOrganizationName("Azote");

    SingleApplication a(argc, argv);
    SelfUpdater selfupdater;

    if (selfupdater.isUpdateAsked(argc, argv))
    {
        selfupdater.update(argv[0]);
        a.quit();
        return 0;
    }

    Launcher launcher;
    launcher.show();

    return a.exec();
}
