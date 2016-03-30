#ifndef SYSTEM_H
#define SYSTEM_H

enum OperatingSystem
{
    WINDOWS,
    MAC,
    LINUX,
    UNKNOWN,
    NOTSET
};

class System
{
public:
    static OperatingSystem get();

private:
    static OperatingSystem operatingSystem;
};

#endif // SYSTEM_H
