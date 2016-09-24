#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include <QAbstractButton>
#include <QSettings>

namespace Ui {
class Settings;
}

enum StartMode
{
    DesktopService,
    Process,
    DetachedProcress
};

class Settings : public QWidget
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = 0);
    ~Settings();

    /*bool isConsoleActivated();
    StartMode getStartMode();*/

private:
    Ui::Settings *ui;
    /*bool m_isConsoleActivated;
    StartMode m_startMode;*/
    QSettings* settings;

private slots:
    void onClickValidationButton(QAbstractButton* button);
    void onClickRepairButton();

signals:
    //void consoleStateChange(bool isEnabled);
    void repairStarted();
};

#endif // SETTINGS_H
