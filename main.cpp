#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //Remove the border of the QLable in the Statusbar
    a.setStyleSheet("QStatusBar::item { border: 0px solid black }; ");

    MainWindow w;
    w.show();

    return a.exec();
}
