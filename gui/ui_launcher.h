/********************************************************************************
** Form generated from reading UI file 'launcher.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LAUNCHER_H
#define UI_LAUNCHER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Launcher
{
public:
    QWidget *centralWidget;
    QPushButton *pushButton;
    QLabel *label;
    QProgressBar *progressBarTotal;
    QProgressBar *progressBarFile;

    void setupUi(QMainWindow *Launcher)
    {
        if (Launcher->objectName().isEmpty())
            Launcher->setObjectName(QStringLiteral("Launcher"));
        Launcher->resize(704, 311);
        centralWidget = new QWidget(Launcher);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(320, 250, 75, 23));
        label = new QLabel(centralWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(620, 240, 46, 13));
        progressBarTotal = new QProgressBar(centralWidget);
        progressBarTotal->setObjectName(QStringLiteral("progressBarTotal"));
        progressBarTotal->setGeometry(QRect(10, 160, 681, 23));
        progressBarTotal->setValue(0);
        progressBarFile = new QProgressBar(centralWidget);
        progressBarFile->setObjectName(QStringLiteral("progressBarFile"));
        progressBarFile->setGeometry(QRect(10, 190, 681, 23));
        progressBarFile->setValue(0);
        Launcher->setCentralWidget(centralWidget);

        retranslateUi(Launcher);

        QMetaObject::connectSlotsByName(Launcher);
    } // setupUi

    void retranslateUi(QMainWindow *Launcher)
    {
        Launcher->setWindowTitle(QApplication::translate("Launcher", "Launcher", 0));
        pushButton->setText(QApplication::translate("Launcher", "PushButton", 0));
        label->setText(QApplication::translate("Launcher", "TextLabel", 0));
    } // retranslateUi

};

namespace Ui {
    class Launcher: public Ui_Launcher {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LAUNCHER_H
