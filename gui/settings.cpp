#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Settings),
    m_isConsoleActivated(true),
    m_startMode(DesktopService)
{
    ui->setupUi(this);

    setFixedSize(width(), height());

    settings = new QSettings(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);

    m_startMode = (StartMode)settings->value("launcher/startMode", m_startMode).toInt();
    m_isConsoleActivated = settings->value("launcher/console", m_isConsoleActivated).toBool();

    ui->startModeComboBox->setCurrentIndex(m_startMode);
    ui->consoleCheckBox->setChecked(m_isConsoleActivated);

    connect(ui->validationButton, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onClickValidationButton(QAbstractButton*)));
    connect(ui->repairButton, SIGNAL(clicked()), this, SLOT(onClickRepairButton()));
}

Settings::~Settings()
{
    delete ui;
}

void Settings::onClickValidationButton(QAbstractButton* button)
{
    if ((QPushButton*)button == ui->validationButton->button(QDialogButtonBox::Ok))
    {
        switch(ui->startModeComboBox->currentIndex())
        {
            case 1:
                m_startMode = Process;
                break;
            case 2:
                m_startMode = DetachedProcress;
                break;
            case 0:
            default:
                m_startMode = DesktopService;
                break;
        }

        m_isConsoleActivated = ui->consoleCheckBox->isChecked();

        settings->setValue("launcher/startMode", m_startMode);
        settings->setValue("launcher/console", m_isConsoleActivated);
        settings->sync();

        emit consoleStateChange(m_isConsoleActivated);
    }

    hide();
}

void Settings::onClickRepairButton()
{
    settings->setValue("client/version", 0);
    settings->sync();

    emit repairStarted();
}

bool Settings::isConsoleActivated()
{
    return m_isConsoleActivated;
}

StartMode Settings::getStartMode()
{
    return m_startMode;
}
