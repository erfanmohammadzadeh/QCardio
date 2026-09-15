#include "Controllers/appcontroller.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/Res/image/icon.ico"));
    AppController controller;
    return a.exec();
}
