#include "launcher.h"
#include "../gui/ui_launcher.h"
#include "../logger/logger.h"

Launcher::Launcher(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Launcher)
{
    ui->setupUi(this);

    setFixedSize(width(), height());

    log = &Singleton<Logger>::getInstance();
    log->showConsole();

    updater = new Updater();
    connect(updater, SIGNAL(updateProgressBar(int)), ui->progressBar, SLOT(setValue(int)));
    updater->start();
}

Launcher::~Launcher()
{
    delete updater;
    delete ui;
}

void Launcher::closeEvent(QCloseEvent* event)
{
    log->closeConsole();
    updater->exit(0);
    close();
    //QCoreApplication::exit();
}
