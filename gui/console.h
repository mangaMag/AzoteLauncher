#ifndef CONSOLE_H
#define CONSOLE_H

#include <QWidget>
#include <QObject>

namespace Ui {
class Console;
}

class Console : public QWidget
{
    Q_OBJECT

public:
    Console(QWidget* parent, QObject* logger);
    ~Console();

private:
    Ui::Console *ui;

private slots:
    void onMessage(QString text);
};

#endif // CONSOLE_H
