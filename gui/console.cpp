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

    connect(logger, SIGNAL(message(LogLevel, QString)), this, SLOT(onMessage(LogLevel, QString)));
}

Console::~Console()
{
    delete ui;
}

void Console::onMessage(LogLevel level, QString text)
{
    QString levelText;
    QString levelColor;

    switch (level)
    {
        case DEBUG:
            levelText = "DEBUG";
            levelColor = "blue";
            break;
        case ERROR:
            levelText = "ERROR";
            levelColor = "red";
            break;
        case SUCCESS:
            levelText = "SUCCESS";
            levelColor = "green";
            break;
        case INFO:
            levelText = "INFO";
            levelColor = "cyan";
            break;
        case WARNING:
            levelText = "WARNING";
            levelColor = "orange";
            break;
        default:
            levelText = "UNKNOWN";
            levelColor = "yellow";
            break;
    }

    QString logText = QString("<b>[<font color='%1'>%2</font>] %3 </b>").arg(levelColor).arg(levelText).arg(text);

    ui->console->append(logText);
}
