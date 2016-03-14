#include "gui/launcher.h"
#include "updater/selfupdater.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (argc > 1)
    {
        SelfUpdater selfupdater;

        if (selfupdater.update(argc, argv))
        {
            a.quit();
            return 0;
        }
    }

    Launcher launcher;
    launcher.show();

    return a.exec();
}
