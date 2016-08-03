#include "launcher.h"
#include "ui_launcher.h"
#include "../logger/logger.h"
#include "../utils/system.h"

#include <QMessageBox>
#include <QFileInfo>
#include <QMenu>
#include <QAction>
#include <QDesktopServices>
#include <QProcess>

Launcher::Launcher(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Launcher)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
    setFixedSize(width(), height());

    qRegisterMetaType<LogLevel>("LogLevel");

    log = &Singleton<Logger>::getInstance();

    settings = new Settings(NULL);
    connect(settings, SIGNAL(repairStarted()), this, SLOT(onRepairStarted()));

    console = new Console(NULL, log, settings);
    console->show();

    startUpdate();

    connect(ui->playButton, SIGNAL(clicked()), this, SLOT(onClickPlayButton()));
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(onClickCloseButton()));
    connect(ui->minimizeButton, SIGNAL(clicked()), this, SLOT(onClickMinimizeButton()));
    connect(ui->settingsButton, SIGNAL(clicked()), this, SLOT(onClickSettingsButton()));

    QMenu* trayIconMenu = new QMenu();
    QAction* actionOpen = trayIconMenu->addAction("Ouvrir");
    QAction* actionQuit = trayIconMenu->addAction("Quitter");

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/ressources/azendaricone.ico"));
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onClickSystemTrayIcon(QSystemTrayIcon::ActivationReason)));
    connect(actionOpen, SIGNAL(triggered()), this, SLOT(onOpenApp()));
    connect(actionQuit, SIGNAL(triggered()), this, SLOT(onCloseApp()));

    QObject::connect(QApplication::instance(), SIGNAL(showUp()), this, SLOT(onOpenApp()));

    tab home    = { HOME,   "Home",    nullptr, ui->homeSelected };
    tab sigma   = { SERVER, "Sigma",   nullptr, ui->sigmaSelected };
    tab epsilon = { SERVER, "Epsilon", nullptr, ui->epsilonSelected };

    tabs.insert(ui->homeButton,    home);
    tabs.insert(ui->sigmaButton,   sigma);
    tabs.insert(ui->epsilonButton, epsilon);

    connect(ui->homeButton,    SIGNAL(clicked()), this, SLOT(onChangeTab()));
    connect(ui->sigmaButton,   SIGNAL(clicked()), this, SLOT(onChangeTab()));
    connect(ui->epsilonButton, SIGNAL(clicked()), this, SLOT(onChangeTab()));

    previousTab = home;
    switchSelectedTab(sigma);

    urls.insert(ui->supportButton,   QUrl("https://azote.us/support"));
    urls.insert(ui->forumButton,     QUrl("https://forum.azote.us/"));
    urls.insert(ui->shopButton,      QUrl("https://azote.us/shop"));
    urls.insert(ui->changelogButton, QUrl("https://azote.us/changelog"));
    urls.insert(ui->newBigFront,     QUrl("https://azote.us/news/1"));
    urls.insert(ui->newSmallFront1,  QUrl("https://azote.us/news/2"));
    urls.insert(ui->newSmallFront2,  QUrl("https://azote.us/news/3"));

    connect(ui->supportButton,   SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
    connect(ui->forumButton,     SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
    connect(ui->shopButton,      SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
    connect(ui->changelogButton, SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
    connect(ui->newBigFront,     SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
    connect(ui->newSmallFront1,  SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
    connect(ui->newSmallFront2,  SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
}

Launcher::~Launcher()
{
    updater->deleteLater();
    log->deleteLater();

    delete ui;
}

void Launcher::closeEvent(QCloseEvent* /*event*/)
{
    settings->close();
    console->close();
    hide();

    updater->stopProcess();
    updater->terminate();
    updater->wait();
}

void Launcher::onClickPlayButton()
{
    OperatingSystem os = System::get();

    if (os == WINDOWS)
    {
        startGame("/../app/Dofus.exe");
    }
    else if (os == MAC)
    {
        QFileInfo dofusBin(QCoreApplication::applicationDirPath() + "/../app/Dofus.app/Contents/MacOs/Flash Player");

        if (!dofusBin.isExecutable())
        {
            QFile::setPermissions(dofusBin.absoluteFilePath(), QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther);
        }

        startGame("/../app/Dofus.app");
    }
    else
    {
        log->error("System OS not found !");
    }
}

void Launcher::startGame(QString gamePath)
{
    QFileInfo dofusBin(QCoreApplication::applicationDirPath() + gamePath);
    QString path = dofusBin.absoluteFilePath();

    switch (settings->getStartMode())
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

void Launcher::switchSelectedTab(tab selectedTab)
{
    previousTab.selector->setStyleSheet("QPushButton{border:none;background:transparent;}");
    selectedTab.selector->setStyleSheet("QPushButton{border:none;background:url(:/ressources/servers/server_selected.png) no-repeat center;}");
    previousTab = selectedTab;
}

void Launcher::onClickCloseButton()
{
    console->close();
    hide();

    trayIcon->showMessage("Azote", "Le launcher a été réduit dans la barre des tâches, cliquez sur l'icon pour l'ouvrir.");
}

void Launcher::onClickMinimizeButton()
{
    setWindowState(Qt::WindowMinimized);
}

void Launcher::onClickSettingsButton()
{
    settings->show();
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

void Launcher::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        if (windowState() == Qt::WindowMinimized)
        {
            console->close();
        }
        else if (windowState() == Qt::WindowNoState || windowState() == Qt::WindowActive)
        {
            console->show();
            raise();
        }
    }
}

void Launcher::startUpdate()
{
    updater = new Updater();
    connect(updater, SIGNAL(updateProgressBarTotal(int)), ui->progressBarTotal, SLOT(setValue(int)));
    connect(updater, SIGNAL(updateDownloadSpeed(QString)), ui->labelDownloadSpeed, SLOT(setText(QString)));
    connect(updater, SIGNAL(updateStatus(QString)), ui->labelStatus, SLOT(setText(QString)));
    connect(updater, SIGNAL(updateFinished()), this, SLOT(onUpdateFinished()));
    connect(updater, SIGNAL(newUpdaterVersion()), this, SLOT(onNewUpdaterVersion()));
    updater->start(QThread::HighestPriority);
}

void Launcher::onClickSystemTrayIcon(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick || reason == QSystemTrayIcon::Trigger)
    {
        onOpenApp();
    }
}

void Launcher::onOpenApp()
{
    console->show();
    show();
    setWindowState(Qt::WindowActive);
    raise();
}

void Launcher::onCloseApp()
{
    trayIcon->hide();
    console->close();
    hide();
    close();
}

void Launcher::onNewUpdaterVersion()
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
        onCloseApp();
    }
}

void Launcher::onRepairStarted()
{
    updater->stopProcess();
    updater->terminate();
    updater->wait();
    updater->deleteLater();

    startUpdate();
}

void Launcher::onUpdateFinished()
{
    ui->playButton->setStyleSheet(ui->playButton->styleSheet().replace("install", "play"));
}

void Launcher::onChangeTab()
{
    QObject* sender = QObject::sender();
    auto tabIterator = tabs.find(sender);

    if (tabIterator != tabs.end())
    {
        tab selectedTab = tabIterator.value();
        switchSelectedTab(selectedTab);
    }
}

void Launcher::onClickLinkButton()
{
    QObject* sender = QObject::sender();
    auto url = urls.find(sender);

    if (url != urls.end())
    {
        QDesktopServices::openUrl(url.value());
    }

}
