#include "controller.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/Res/image/icon.ico"));
    Controller controller;
    return a.exec();
}
