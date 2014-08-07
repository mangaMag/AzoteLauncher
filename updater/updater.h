#ifndef UPDATER_H
#define UPDATER_H

#include <QThread>

class Updater : public QThread
{
    Q_OBJECT

private:
    void run();

public:
    explicit Updater(QThread* parent = 0);

signals:
    void updateProgressBar(const int value);

public slots:

};

#endif // UPDATER_H
