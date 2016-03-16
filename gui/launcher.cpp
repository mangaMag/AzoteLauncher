#include "launcher.h"
#include "ui_launcher.h"
#include "../logger/logger.h"
#include "../others/sound.h"

#include <QProcess>
#include <QMessageBox>

Launcher::Launcher(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Launcher),
    isRegStarted(false)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
    setFixedSize(width(), height());

    ui->labelDownloadSpeed->setAttribute(Qt::WA_TranslucentBackground);

    log = &Singleton<Logger>::getInstance();
    log->showConsole();

    updater = new Updater();
    connect(updater, SIGNAL(updateProgressBarTotal(int)), ui->progressBarTotal, SLOT(setValue(int)));
    connect(updater, SIGNAL(updateDownloadSpeed(QString)), ui->labelDownloadSpeed, SLOT(setText(QString)));
    connect(updater, SIGNAL(updateStatus(QString)), ui->labelStatus, SLOT(setText(QString)));
    connect(updater, SIGNAL(enablePlayButton(bool)), ui->playButton, SLOT(setEnabled(bool)));
    updater->start();

    connect(ui->playButton, SIGNAL(clicked()), this, SLOT(onClickPlayButton()));
    connect(ui->playButton, SIGNAL(pressed()), this, SLOT(onPressedPlayButton()));
    connect(ui->playButton, SIGNAL(released()), this, SLOT(onReleasedPlayButton()));

    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(onClickCloseButton()));
    connect(ui->minimizeButton, SIGNAL(clicked()), this, SLOT(onClickMinimizeButton()));
    connect(ui->settingsButton, SIGNAL(clicked()), this, SLOT(onClickSettingsButton()));

    Sound* sound = new Sound();
    port = sound->start();
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

void Launcher::onClickPlayButton()
{
    QProcess* dofus = new QProcess(this);
    QStringList paramsDofus;

    paramsDofus << "--lang=fr";
    paramsDofus << "--update-server-port=" + QString::number(port);
    paramsDofus << "--updater_version=v2";
    paramsDofus << "--reg-client-port=" + QString::number(port + 1);

#ifdef _WIN32
    dofus->startDetached("../app/Dofus.exe", paramsDofus);
#else
    QFile::setPermissions(QCoreApplication::applicationDirPath() + "../Dofus.app/Contents/MacOS/Dofus",
                            QFile::ReadOwner  |
                            QFile::WriteOwner |
                            QFile::ExeOwner   |
                            QFile::ReadGroup  |
                            QFile::ExeGroup   |
                            QFile::ReadOther  |
                            QFile::ExeOther);

    dofus->startDetached(QString("open %1").arg(QCoreApplication::applicationDirPath() + "../Dofus.app"), paramsDofus);
#endif

    if (!isRegStarted)
    {
        QProcess* reg = new QProcess(this);
        QStringList paramsReg;

        paramsReg << "--reg-engine-port=" + QString::number(port + 2);

#ifdef _WIN32
        reg->start("../app/reg/Reg.exe", paramsReg);
#else
        QFile::setPermissions(QCoreApplication::applicationDirPath() + "../Dofus.app/Contents/Resources/Reg.app/Contents/MacOS/Reg",
                                QFile::ReadOwner  |
                                QFile::WriteOwner |
                                QFile::ExeOwner   |
                                QFile::ReadGroup  |
                                QFile::ExeGroup   |
                                QFile::ReadOther  |
                                QFile::ExeOther);

        reg->start(QString("open %1").arg(QCoreApplication::applicationDirPath() + "../Dofus.app/Contents/Resources/Reg.app"), paramsReg);
#endif

        isRegStarted = true;
    }
}

void Launcher::onPressedPlayButton()
{
    ui->playButton->setIcon(QIcon(":/ressources/bouton_enable_pushed.png"));
}

void Launcher::onReleasedPlayButton()
{
    ui->playButton->setIcon(QIcon(":/ressources/bouton_enable.png"));
}

void Launcher::onClickCloseButton()
{
    close();
}

void Launcher::onClickMinimizeButton()
{
    setWindowState(Qt::WindowMinimized);
}

void Launcher::onClickSettingsButton()
{
    QMessageBox msgBox;
    msgBox.setText("Coming Soon");
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();
}

void Launcher::mousePressEvent(QMouseEvent* event)
{
    mpos = event->pos();
}

void Launcher::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() && Qt::LeftButton)
    {
        QPoint diff = event->pos() - mpos;
        QPoint newpos = this->pos() + diff;

        this->move(newpos);
    }
}
