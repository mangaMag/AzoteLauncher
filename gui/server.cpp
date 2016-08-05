#include "server.h"
#include "ui_server.h"
#include "launcher.h"
#include "updater/updater.h"
#include "logger/logger.h"

#include <QMessageBox>
#include <QFileInfo>
#include <QProcess>
#include <QDesktopServices>
#include <QVector>

Server::Server(QWidget* parent, Launcher* _launcher, QString _name) :
    QWidget(parent),
    launcher(_launcher),
    ui(new Ui::Server),
    updater(NULL),
    state(PAUSE),
    name(_name)
{
    ui->setupUi(this);

    log = &Singleton<Logger>::getInstance();

    //startUpdate();

    QVector<QWidget*> images;
    images.push_back(ui->serverDescription);
    images.push_back(ui->backgroundArtwork);

    foreach(QWidget* image, images)
    {
        QString style = image->styleSheet().replace("_SERVER_", name.toLower());
        image->setStyleSheet(style);
    }

    connect(ui->playButton, SIGNAL(clicked()), this, SLOT(onClickPlayButton()));
    connect(ui->settingsButton, SIGNAL(clicked()), this, SLOT(onClickSettingsButton()));
    connect(ui->resumePauseButton, SIGNAL(clicked()), this, SLOT(onClickResumePauseButton()));

    urls.insert(ui->newBigFront,     QUrl("https://azote.us/news/1"));
    urls.insert(ui->newSmallFront1,  QUrl("https://azote.us/news/2"));
    urls.insert(ui->newSmallFront2,  QUrl("https://azote.us/news/3"));

    connect(ui->newBigFront,     SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
    connect(ui->newSmallFront1,  SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
    connect(ui->newSmallFront2,  SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
}

Server::~Server()
{
    if (updater != NULL)
    {
        updater->stopProcess();
        updater->terminate();
        updater->wait();
        updater->deleteLater();
    }

    delete ui;
}

void Server::startGame(QString gamePath)
{
    QFileInfo dofusBin(QCoreApplication::applicationDirPath() + gamePath);
    QString path = dofusBin.absoluteFilePath();

    switch (launcher->settings->getStartMode())
    {
        case Process:
        {
            QProcess* dofus = new QProcess(this);
            dofus->start(path);
            break;
        }
        case DetachedProcress:
            QProcess::startDetached(path);
            break;
        case DesktopService:
        default:
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
            break;
    }
}

void Server::startUpdate()
{
    ui->resumePauseButton->setStyleSheet(ui->resumePauseButton->styleSheet().replace("resume", "pause"));
    stylePlay = ui->playButton->styleSheet().replace("install", "play");
    ui->playButton->setStyleSheet("QPushButton{border:none;background:url(:/ressources/download_play/play_disabled.png) no-repeat;}");
    state = RESUME;

    if (updater != NULL)
    {
        updater->resume();
    }
    else
    {
        updater = new Updater(name);
        connect(updater, SIGNAL(updateProgressBarTotal(int)), ui->progressBarTotal, SLOT(setValue(int)));
        connect(updater, SIGNAL(updateDownloadSpeed(QString)), ui->labelDownloadSpeed, SLOT(setText(QString)));
        connect(updater, SIGNAL(updateStatus(QString)), ui->labelStatus, SLOT(setText(QString)));
        connect(updater, SIGNAL(updateFinished()), this, SLOT(onUpdateFinished()));
        connect(updater, SIGNAL(newUpdaterVersion()), this, SLOT(onNewUpdaterVersion()));
        updater->start(QThread::HighestPriority);
    }
}

void Server::pauseUpdater()
{
    if (updater == NULL)
    {
        return;
    }

    state = PAUSE;
    ui->resumePauseButton->setStyleSheet(ui->resumePauseButton->styleSheet().replace("pause", "resume"));
    updater->pause();
}

void Server::onUpdateFinished()
{
    ui->playButton->setStyleSheet(stylePlay);
    ui->resumePauseButton->hide();
    ui->playButton->setEnabled(true);
    state = FINISHED;
}

void Server::onNewUpdaterVersion()
{
    QMessageBox::StandardButton reply = QMessageBox::information(NULL, "Azote", "Une nouvelle version du launcher est disponible. Cliquez sur Ok pour continuer.", QMessageBox::Ok | QMessageBox::Cancel);

    if (reply == QMessageBox::Ok)
    {
        updater->resume();
    }
    else
    {
        updater->stopProcess();
        updater->resume();
        launcher->onCloseApp();
    }
}

void Server::onClickPlayButton()
{
    if (state == PAUSE)
    {
        startUpdate();
        ui->playButton->setEnabled(false);
    }

    if (state != FINISHED)
    {
        return;
    }

    OperatingSystem os = System::get();

    if (os == WINDOWS)
    {
        startGame("/../" + name + "_app/Dofus.exe");
    }
    else if (os == MAC)
    {
        QFileInfo dofusBin(QCoreApplication::applicationDirPath() + "/../" + name + "_app/Dofus.app/Contents/MacOs/Flash Player");

        if (!dofusBin.isExecutable())
        {
            QFile::setPermissions(dofusBin.absoluteFilePath(), QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther);
        }

        startGame("/../" + name + "_app/Dofus.app");
    }
    else
    {
        log->error("System OS not found !");
    }
}

void Server::onClickSettingsButton()
{
    launcher->settings->show();
}

void Server::onClickLinkButton()
{
    QObject* sender = QObject::sender();
    auto url = urls.find(sender);

    if (url != urls.end())
    {
        QDesktopServices::openUrl(url.value());
    }
}

void Server::onClickResumePauseButton()
{
    switch (state)
    {
        case PAUSE:
            startUpdate();
            break;
        case RESUME:
            pauseUpdater();
            break;
        case FINISHED:
        default:
            ui->resumePauseButton->hide();
            break;

    }
}
