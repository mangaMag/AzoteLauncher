#include "launcher.h"
#include "../gui/ui_launcher.h"
#include "../logger/logger.h"

Launcher::Launcher(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Launcher)
{
    ui->setupUi(this);

    Logger* log = &Singleton<Logger>::getInstance();
    log->showConsole();

    updater = new Updater();
    connect(updater, SIGNAL(updateProgressBar(int)), ui->progressBar, SLOT(setValue(int)));
    updater->start();
}

Launcher::~Launcher()
{
    updater->exit(0);
    delete updater;
    delete ui;
}
