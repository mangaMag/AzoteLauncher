#ifndef UPDATER_H
#define UPDATER_H

#include <QThread>

class Updater : public QThread
{
    Q_OBJECT

private:
    bool flagRun;

    void run();

public:
    explicit Updater(QThread* parent = 0);
    void stopProcess();

signals:
    void updateProgressBar(const int value);

};

#endif // UPDATER_H
