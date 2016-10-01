#include "launcher.h"
#include "ui_launcher.h"
#include "logger/logger.h"
#include "utils/system.h"
#include "updater/selfupdater.h"
#include "server.h"
#include "console.h"
#include "home.h"

#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QDesktopServices>

Launcher::Launcher(QWidget* parent) :
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

    ui->epsilon->hide();
    ui->epsilonButton->hide();
    ui->epsilonSelected->hide();

    Tab* home    = new Tab(HOME,    "Home",    ui->homeSelected);
    Tab* sigma   = new Tab(SERVER,  "Sigma",   ui->sigmaSelected);
    //Tab* epsilon = new Tab(SERVER,  "Epsilon", ui->epsilonSelected);
    Tab* console = new Tab(CONSOLE, "Console", ui->consoleSelected);

    tabs.insert(ui->homeButton,    home);
    tabs.insert(ui->sigmaButton,   sigma);
    //tabs.insert(ui->epsilonButton, epsilon);
    tabs.insert(ui->consoleButton, console);

    foreach (Tab* tab, tabs)
    {
        if (tab->window == NULL)
        {
            switch (tab->type)
            {
                case HOME:
                    tab->window = new Home(ui->mainWidget);
                    break;
                case SERVER:
                    tab->window = new Server(ui->mainWidget, this, tab->name);
                    break;
                case CONSOLE:
                    tab->window = new Console(ui->mainWidget, log, settings);
                    break;
                case SETTINGS:
                    tab->window = NULL;
                    break;
            }
        }

        if (tab->window != NULL) tab->window->hide();
    }

    connect(ui->homeButton,    SIGNAL(clicked()), this, SLOT(onChangeTab()));
    connect(ui->sigmaButton,   SIGNAL(clicked()), this, SLOT(onChangeTab()));
    connect(ui->epsilonButton, SIGNAL(clicked()), this, SLOT(onChangeTab()));
    connect(ui->consoleButton, SIGNAL(clicked()), this, SLOT(onChangeTab()));

    previousTab = home;
    switchSelectedTab(sigma);

    urls.insert(ui->supportButton,   QUrl("http://azote.us/support"));
    urls.insert(ui->forumButton,     QUrl("http://forum.azote.us/"));
    urls.insert(ui->shopButton,      QUrl("http://azote.us/boutique/paiement/choix-pays"));
    urls.insert(ui->changelogButton, QUrl("http://azote.us"));

    connect(ui->supportButton,   SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
    connect(ui->forumButton,     SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
    connect(ui->shopButton,      SIGNAL(clicked()), this, SLOT(onClickLinkButton()));
    connect(ui->changelogButton, SIGNAL(clicked()), this, SLOT(onClickLinkButton()));

    selfUpdater = new SelfUpdater();
    connect(selfUpdater, SIGNAL(newUpdaterVersion()), this, SLOT(onNewUpdaterVersion()));
    selfUpdater->start(QThread::HighestPriority);
}

Launcher::~Launcher()
{
    log->deleteLater();
    selfUpdater->deleteLater();

    delete ui;
}

void Launcher::closeEvent(QCloseEvent* /*event*/)
{
    settings->close();
    hide();

    // TODO: close all windows
}


void Launcher::switchSelectedTab(Tab* selectedTab)
{
    if (previousTab->window != NULL)
    {
        previousTab->window->hide();
    }

    previousTab->selector->setStyleSheet("QPushButton{border:none;background:transparent;}");
    selectedTab->selector->setStyleSheet("QPushButton{border:none;background:url(:/ressources/servers/server_selected.png) no-repeat center;}");
    previousTab = selectedTab;

    if (selectedTab->window != NULL)
    {
        selectedTab->window->show();
    }
}

void Launcher::onClickCloseButton()
{
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
            //
        }
        else if (windowState() == Qt::WindowNoState || windowState() == Qt::WindowActive)
        {
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
    show();
    setWindowState(Qt::WindowActive);
    raise();
}

void Launcher::onCloseApp()
{
    trayIcon->hide();
    hide();
    close();
}

void Launcher::onChangeTab()
{
    QObject* sender = QObject::sender();
    auto tabIterator = tabs.find(sender);

    if (tabIterator != tabs.end())
    {
        Tab* selectedTab = tabIterator.value();
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

void Launcher::onNewUpdaterVersion()
{
    QMessageBox::StandardButton reply = QMessageBox::information(NULL, "Azote", "Une nouvelle version du launcher est disponible. Cliquez sur Ok pour continuer.", QMessageBox::Ok | QMessageBox::Cancel);

    if (reply == QMessageBox::Ok)
    {
        selfUpdater->resume();
    }
    else
    {
        selfUpdater->stopProcess();
        selfUpdater->resume();
        onCloseApp();
    }
}
