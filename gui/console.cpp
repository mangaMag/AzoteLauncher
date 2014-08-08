#include "console.h"
#include "ui_console.h"
#include <QDebug>

Console::Console(QWidget* parent, QObject* logger) :
    QWidget(parent),
    ui(new Ui::Console)
{
    ui->setupUi(this);

    setGeometry(100, 100, width(), height());
    setFixedSize(width(), height());

    connect(logger, SIGNAL(message(QString)), this, SLOT(onMessage(QString)));
}

Console::~Console()
{
    delete ui;
}

void Console::onMessage(QString text)
{
    ui->console->append(text);
}
