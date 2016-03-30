#include "launcher.h"
#include "ui_launcher.h"
#include "../logger/logger.h"
#include "../utils/system.h"

#include <QMessageBox>
#include <QFileInfo>

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

    qRegisterMetaType<LogLevel>("LogLevel");

    log = &Singleton<Logger>::getInstance();
    log->showConsole();

    updater = new Updater();
    connect(updater, SIGNAL(updateProgressBarTotal(int)), ui->progressBarTotal, SLOT(setValue(int)));
    connect(updater, SIGNAL(updateDownloadSpeed(QString)), ui->labelDownloadSpeed, SLOT(setText(QString)));
    connect(updater, SIGNAL(updateStatus(QString)), ui->labelStatus, SLOT(setText(QString)));
    connect(updater, SIGNAL(enablePlayButton(bool)), ui->playButton, SLOT(setEnabled(bool)));
    updater->start(QThread::HighestPriority);

    connect(ui->playButton, SIGNAL(clicked()), this, SLOT(onClickPlayButton()));
    connect(ui->playButton, SIGNAL(pressed()), this, SLOT(onPressedPlayButton()));
    connect(ui->playButton, SIGNAL(released()), this, SLOT(onReleasedPlayButton()));

    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(onClickCloseButton()));
    connect(ui->minimizeButton, SIGNAL(clicked()), this, SLOT(onClickMinimizeButton()));
    connect(ui->settingsButton, SIGNAL(clicked()), this, SLOT(onClickSettingsButton()));

    sound = new Sound();
    port = sound->start();

    sti = new QSystemTrayIcon(this);
    sti->setIcon(QIcon(":/ressources/Launcher.ico"));
    connect(sti, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onClickSystemTrayIcon(QSystemTrayIcon::ActivationReason)));
}

Launcher::~Launcher()
{
    updater->deleteLater();
    sound->deleteLater();
    log->deleteLater();

    delete ui;
}

void Launcher::closeEvent(QCloseEvent* /*event*/)
{
    updater->stopProcess();
    updater->wait();
    log->closeConsole();
}

void Launcher::onClickPlayButton()
{
    QProcess* dofus = new QProcess(this);
    OperatingSystem os = System::get();
    QStringList paramsDofus;

    paramsDofus << "--lang=fr";
    paramsDofus << "--update-server-port=" + QString::number(port);
    paramsDofus << "--updater_version=v2";
    paramsDofus << "--reg-client-port=" + QString::number(port + 1);

    if (os == WINDOWS)
    {
        dofus->startDetached(QCoreApplication::applicationDirPath() + "/../app/Dofus.exe", paramsDofus);
    }

    if (os == MAC)
    {
        QFileInfo dofusBin(QCoreApplication::applicationDirPath() + "/../Dofus.app/Contents/MacOs/Dofus");

        if (!dofusBin.isExecutable())
        {
            QFile::setPermissions(dofusBin.absoluteFilePath(), QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther);
        }

        dofus->startDetached(QString("open -a %1 -n --args %2").arg(QCoreApplication::applicationDirPath() + "/../Dofus.app").arg(paramsDofus.join(" ")));
    }

    if (!isRegStarted)
    {
        reg = new QProcess(this);
        QStringList paramsReg;

        paramsReg << "--reg-engine-port=" + QString::number(port + 2);

        if (os == WINDOWS)
        {
            reg->start(QCoreApplication::applicationDirPath() + "/../app/reg/Reg.exe", paramsReg);
        }

        if (os == MAC)
        {
            QFileInfo regBin(QCoreApplication::applicationDirPath() + "/../Dofus.app/Contents/Resources/Reg.app/Contents/MacOs/Reg");

            if (!regBin.isExecutable())
            {
                QFile::setPermissions(regBin.absoluteFilePath(), QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther);
            }

            reg->start(QString("open -a %1 --args %2").arg(QCoreApplication::applicationDirPath() + "/../Dofus.app/Contents/Resources/Reg.app").arg(paramsReg.join(" ")));
        }

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
    log->closeConsole();
    sti->show();
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

void Launcher::onClickSystemTrayIcon(QSystemTrayIcon::ActivationReason /*reason*/)
{
    sti->hide();
    setWindowState(Qt::WindowActive);
    log->showConsole();
}
