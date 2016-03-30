#include "logger.h"

Logger::Logger()
{
    console = new Console(0, this);
}

Logger::~Logger()
{
    delete console;
}

void Logger::showConsole()
{
    console->show();
}

void Logger::closeConsole()
{
    console->close();
}

void Logger::debug(QString text)
{
    emit message(DEBUG, text);
}

void Logger::error(QString text)
{
    emit message(ERROR, text);
}

void Logger::success(QString text)
{
    emit message(SUCCESS, text);
}

void Logger::info(QString text)
{
    emit message(INFO, text);
}

void Logger::warning(QString text)
{
    emit message(WARNING, text);
}
