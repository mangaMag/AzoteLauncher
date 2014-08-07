#include "logger.h"
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

Logger::Logger()
{
}

void Logger::showConsole()
{
#ifdef _WIN32
    AllocConsole();
    FILE *pFileCon = NULL;
    pFileCon = freopen("CONOUT$", "w", stdout);

    COORD coordInfo;
    coordInfo.X = 130;
    coordInfo.Y = 9000;

    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coordInfo);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE),ENABLE_QUICK_EDIT_MODE| ENABLE_EXTENDED_FLAGS);
#endif
}

void Logger::debug(QString text)
{
    std::cout << "[DEBUG] " << text.toStdString() << std::endl;
}

void Logger::error(QString text)
{
    std::cout << "[ERROR] " << text.toStdString() << std::endl;
}
