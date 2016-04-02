#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Settings),
    m_isConsoleActivated(true)
{
    ui->setupUi(this);

    setFixedSize(width(), height());

    connect(ui->validationButton, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onClickValidationButton(QAbstractButton*)));
}

Settings::~Settings()
{
    delete ui;
}

void Settings::onClickValidationButton(QAbstractButton* button)
{
    if ((QPushButton*)button == ui->validationButton->button(QDialogButtonBox::Ok))
    {
        m_isConsoleActivated = ui->consoleCheckBox->isChecked();
        emit consoleStateChange(m_isConsoleActivated);
    }

    hide();
}

bool Settings::isConsoleActivated()
{
    return m_isConsoleActivated;
}
