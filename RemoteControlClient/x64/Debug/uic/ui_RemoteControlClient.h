/********************************************************************************
** Form generated from reading UI file 'RemoteControlClient.ui'
**
** Created by: Qt User Interface Compiler version 5.12.12
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REMOTECONTROLCLIENT_H
#define UI_REMOTECONTROLCLIENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RemoteControlClientClass
{
public:

    void setupUi(QWidget *RemoteControlClientClass)
    {
        if (RemoteControlClientClass->objectName().isEmpty())
            RemoteControlClientClass->setObjectName(QString::fromUtf8("RemoteControlClientClass"));
        RemoteControlClientClass->resize(600, 400);

        retranslateUi(RemoteControlClientClass);

        QMetaObject::connectSlotsByName(RemoteControlClientClass);
    } // setupUi

    void retranslateUi(QWidget *RemoteControlClientClass)
    {
        RemoteControlClientClass->setWindowTitle(QApplication::translate("RemoteControlClientClass", "RemoteControlClient", nullptr));
    } // retranslateUi

};

namespace Ui {
    class RemoteControlClientClass: public Ui_RemoteControlClientClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REMOTECONTROLCLIENT_H
