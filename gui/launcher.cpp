#include "launcher.h"
#include "ui_launcher.h"
#include "../logger/logger.h"

Launcher::Launcher(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Launcher)
{
    ui->setupUi(this);

    setFixedSize(width(), height());
    ui->labelDownloadSpeed->setAttribute(Qt::WA_TranslucentBackground);

    log = &Singleton<Logger>::getInstance();
    log->showConsole();

    updater = new Updater();
    connect(updater, SIGNAL(updateProgressBarTotal(int)), ui->progressBarTotal, SLOT(setValue(int)));
    connect(updater, SIGNAL(updateProgressBarFile(int)), ui->progressBarFile, SLOT(setValue(int)));
    connect(updater, SIGNAL(updateDownloadSpeed(QString)), ui->labelDownloadSpeed, SLOT(setText(QString)));
    connect(updater, SIGNAL(enablePlayButton(bool)), ui->playButton, SLOT(setEnabled(bool)));
    updater->start();
}

Launcher::~Launcher()
{
    delete updater;
    delete ui;
}

void Launcher::closeEvent(QCloseEvent* /*event*/)
{
    updater->stopProcess();
    log->closeConsole();
}
