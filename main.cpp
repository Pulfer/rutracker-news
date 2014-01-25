#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("Windows-1251"));
//    QApplication::setStyle(new QWindowsStyle);
    MainWindow w;
    w.showMaximized();
    return a.exec();
}
