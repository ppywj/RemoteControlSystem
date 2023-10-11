#pragma once

#include <QtWidgets/QWidget>
#include "ui_RemoteControlClient.h"

class RemoteControlClient : public QWidget
{
    Q_OBJECT

public:
    RemoteControlClient(QWidget *parent = nullptr);
    ~RemoteControlClient();

private:
    Ui::RemoteControlClientClass ui;
};
