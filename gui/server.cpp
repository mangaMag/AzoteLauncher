#include "server.h"
#include "ui_server.h"
#include "launcher.h"
#include "updater/updater.h"
#include "logger/logger.h"
#include "others/sound.h"

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
    state(NO_STARTED),
    name(_name),
    port(1337),
    isRegStarted(false)
{
    ui->setupUi(this);

    log = &Singleton<Logger>::getInstance();
    os = System::get();

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

    urls.insert(ui->newBigFront,     QUrl("http://azote.us/news/1"));
    urls.insert(ui->newSmallFront1,  QUrl("http://azote.us/news/2"));
    urls.insert(ui->newSmallFront2,  QUrl("http://azote.us/news/3"));

    connect(ui->newBigFront,     SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
    connect(ui->newSmallFront1,  SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
    connect(ui->newSmallFront2,  SIGNAL(clicked()), this, SLOT(onClickLinkButton()));

    stylePlay = ui->playButton->styleSheet().replace("install", "play");

    checkUpdate();
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

void Server::startProcess(QString processName, QStringList args, bool forceDetached = false)
{
    QFileInfo processBin(QCoreApplication::applicationDirPath() + "/" + processName);
    QString path = processBin.absoluteFilePath();

    if (forceDetached)
    {
        if (os == WINDOWS)
        {
            QProcess::startDetached(path, args);
        }
        else if (os == MAC)
        {
            QProcess::startDetached(QString("open -a %1 -n --args %2").arg(path).arg(args.join(" ")));
        }
    }
    else
    {
        QProcess* process = new QProcess(this);
        process->start(path, args);
    }
}

void Server::startGame()
{
    QStringList args;

    //args << "--lang=fr";
    //args << "--update-server-port=" + QString::number(port);
    //args << "--updater_version=v2";
    args << "--reg-client-port=" + QString::number(port + 1);

    if (os == WINDOWS) startProcess(name + "_app/Dofus.exe", args, true);
    if (os == MAC)     startProcess(name + ".app", args, true);
}

void Server::startSound()
{
    if (!isRegStarted)
    {
        sound = new Sound();
        port = sound->start();

        QStringList args;

        args << "--reg-engine-port=" + QString::number(port + 2);

        if (os == WINDOWS) startProcess(name + "_app/reg/Reg.exe", args);
        if (os == MAC)     startProcess(name + ".app/Contents/Resources/Reg.app", args);
    }
}

void Server::startUpdate()
{
    ui->resumePauseButton->setStyleSheet(ui->resumePauseButton->styleSheet().replace("resume", "pause"));
    ui->playButton->setStyleSheet("QPushButton{border:none;background:url(:/ressources/download_play/play_disabled.png) no-repeat;}");

    if (updater != NULL)
    {
        if (state == NO_STARTED)
        {
            updater->start(QThread::HighestPriority);
        }
        else if (state == PAUSE)
        {
            updater->resume();
        }
    }
    else
    {
        createUpdater();
        updater->start(QThread::HighestPriority);
    }

    state = RESUME;
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

void Server::createUpdater()
{
    if (updater == NULL)
    {
        updater = new Updater(name);
        connect(updater, SIGNAL(updateProgressBarTotal(int)), ui->progressBarTotal, SLOT(setValue(int)));
        connect(updater, SIGNAL(updateDownloadSpeed(QString)), ui->labelDownloadSpeed, SLOT(setText(QString)));
        connect(updater, SIGNAL(updateStatus(QString)), ui->labelStatus, SLOT(setText(QString)));
        connect(updater, SIGNAL(updateFinished()), this, SLOT(onUpdateFinished()));
    }
}

void Server::checkUpdate()
{
    createUpdater();
    if (!updater->isNeedUpdate())
    {
        onUpdateFinished();
    }
}

void Server::onUpdateFinished()
{
    ui->playButton->setStyleSheet(stylePlay);
    ui->resumePauseButton->hide();
    ui->playButton->setEnabled(true);
    state = FINISHED;
}

void Server::onClickPlayButton()
{
    if (state == PAUSE || state == NO_STARTED)
    {
        startUpdate();
        ui->playButton->setEnabled(false);
    }

    if (state != FINISHED)
    {
        return;
    }

    if (os == MAC)
    {
        QFileInfo dofusBin(QCoreApplication::applicationDirPath() + "/../" + name + ".app/Contents/MacOS/Dofus");
        QFile::setPermissions(dofusBin.absoluteFilePath(), QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther);

        QFileInfo regBin(QCoreApplication::applicationDirPath() + "/../" + name + ".app/Contents/Resources/Reg.app/Contents/MacOS/Reg");
        QFile::setPermissions(regBin.absoluteFilePath(), QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther);
    }

    startSound();
    startGame();
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
