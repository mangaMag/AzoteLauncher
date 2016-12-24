#include "gui/launcher.h"
#include "updater/selfupdater.h"
#include <QApplication>
#include "dep/SingleApplication/singleapplication.h"

int main(int argc, char *argv[])
{
    QApplication::setApplicationName("Launcher");
    QApplication::setOrganizationName("Azote");

    SelfUpdater selfupdater;

    if (selfupdater.isUpdateAsked(argc, argv))
    {
        selfupdater.update(argv[0]);
        return 0;
    }

    SingleApplication a(argc, argv);

    Launcher launcher;
    launcher.show();

    return a.exec();
}
