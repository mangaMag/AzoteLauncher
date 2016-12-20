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

    void show(QString server);

private:
    Ui::Settings *ui;
    /*bool m_isConsoleActivated;
    StartMode m_startMode;*/
    QSettings* settings;
    QString lastServer;

private slots:
    void onClickValidationButton(QAbstractButton* button);
    void onClickRepairButton();

signals:
    //void consoleStateChange(bool isEnabled);
    void repairStarted(QString server);
};

#endif // SETTINGS_H
