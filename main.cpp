#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("Windows-1251"));
    MainWindow w;
    w.showMaximized();
    return a.exec();
}
