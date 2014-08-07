#include "logger.h"
#include <iostream>

Logger::Logger()
{
    console = new Console(0, this);
}

Logger::~Logger()
{
    closeConsole();
}

void Logger::showConsole()
{
    console->show();
}

void Logger::closeConsole()
{
    console->hide();
}

void Logger::debug(QString text)
{
    //std::cout << "[DEBUG] " << text.toStdString() << std::endl;
    emit message(QString("<b>[<font color='blue'>DEBUG</font>] %1</b>").arg(text));
}

void Logger::error(QString text)
{
    //std::cout << "[ERROR] " << text.toStdString() << std::endl;
    emit message(QString("<b>[<font color='red'>ERROR</font>] %1</b>").arg(text));
}
