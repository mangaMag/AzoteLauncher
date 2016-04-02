#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include <QAbstractButton>

namespace Ui {
class Settings;
}

class Settings : public QWidget
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = 0);
    ~Settings();

    bool isConsoleActivated();

private:
    Ui::Settings *ui;
    bool m_isConsoleActivated;

private slots:
    void onClickValidationButton(QAbstractButton* button);

signals:
    void consoleStateChange(bool isEnabled);
};

#endif // SETTINGS_H
