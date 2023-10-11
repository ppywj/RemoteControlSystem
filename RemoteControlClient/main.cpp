#include "RemoteControlClient.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RemoteControlClient w;
    w.show();
    return a.exec();
}
