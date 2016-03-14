#ifndef SELFUPDATER_H
#define SELFUPDATER_H

#include <QObject>

class SelfUpdater : public QObject
{
    Q_OBJECT
public:
    explicit SelfUpdater(QObject *parent = 0);
    bool update(int argc, char *argv[]);

signals:

public slots:
};

#endif // SELFUPDATER_H
