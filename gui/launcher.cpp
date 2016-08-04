#include "launcher.h"
#include "ui_launcher.h"
#include "../logger/logger.h"
#include "../utils/system.h"
#include "server.h"

#include <QMessageBox>
#include <QMenu>
#include <QAction>

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

    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(onClickCloseButton()));
    connect(ui->minimizeButton, SIGNAL(clicked()), this, SLOT(onClickMinimizeButton()));

    QMenu* trayIconMenu = new QMenu();
    QAction* actionOpen = trayIconMenu->addAction("Ouvrir");
    QAction* actionQuit = trayIconMenu->addAction("Quitter");

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/ressources/icon.ico"));
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onClickSystemTrayIcon(QSystemTrayIcon::ActivationReason)));
    connect(actionOpen, SIGNAL(triggered()), this, SLOT(onOpenApp()));
    connect(actionQuit, SIGNAL(triggered()), this, SLOT(onCloseApp()));

    QObject::connect(QApplication::instance(), SIGNAL(showUp()), this, SLOT(onOpenApp()));

    tab home    = { HOME,    "Home",    nullptr, ui->homeSelected };
    tab sigma   = { SERVER,  "Sigma",   nullptr, ui->sigmaSelected };
    tab epsilon = { SERVER,  "Epsilon", nullptr, ui->epsilonSelected };
    tab console = { CONSOLE, "Console", nullptr, ui->consoleSelected };

    tabs.insert(ui->homeButton,    home);
    tabs.insert(ui->sigmaButton,   sigma);
    tabs.insert(ui->epsilonButton, epsilon);
    tabs.insert(ui->consoleButton, console);

    connect(ui->homeButton,    SIGNAL(clicked()), this, SLOT(onChangeTab()));
    connect(ui->sigmaButton,   SIGNAL(clicked()), this, SLOT(onChangeTab()));
    connect(ui->epsilonButton, SIGNAL(clicked()), this, SLOT(onChangeTab()));
    connect(ui->consoleButton, SIGNAL(clicked()), this, SLOT(onChangeTab()));

    previousTab = home;
    switchSelectedTab(sigma);

    currentWindows = new Server(this, ui->mainWidget);

    urls.insert(ui->supportButton,   QUrl("https://azote.us/support"));
    urls.insert(ui->forumButton,     QUrl("https://forum.azote.us/"));
    urls.insert(ui->shopButton,      QUrl("https://azote.us/shop"));
    urls.insert(ui->changelogButton, QUrl("https://azote.us/changelog"));

    connect(ui->supportButton,   SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
    connect(ui->forumButton,     SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
    connect(ui->shopButton,      SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
    connect(ui->changelogButton, SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
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

void Launcher::onRepairStarted()
{
    updater->stopProcess();
    updater->terminate();
    updater->wait();
    updater->deleteLater();

    //startUpdate();
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
