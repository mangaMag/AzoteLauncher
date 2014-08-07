#include "logger.h"
#include <Windows.h>
#include <iostream>

Logger::Logger()
{
}

void Logger::showConsole()
{
    AllocConsole();
    FILE *pFileCon = NULL;
    pFileCon = freopen("CONOUT$", "w", stdout);

    COORD coordInfo;
    coordInfo.X = 130;
    coordInfo.Y = 9000;

    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coordInfo);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE),ENABLE_QUICK_EDIT_MODE| ENABLE_EXTENDED_FLAGS);
}

void Logger::debug(QString text)
{
    std::cout << "[DEBUG] " << text.toStdString() << std::endl;
}

void Logger::error(QString text)
{
    std::cout << "[ERROR] " << text.toStdString() << std::endl;
}
