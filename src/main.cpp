#include <QApplication>
#include "mainwindow.h"

#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFontDatabase::addApplicationFont(":/src/fonts/Inter_18pt-Regular.ttf");
    QFontDatabase::addApplicationFont(":/src/fonts/Inter_24pt-Regular.ttf");
    QFontDatabase::addApplicationFont(":/src/fonts/Inter_28pt-Regular.ttf");

    MainWindow w;
    w.show();
    return app.exec();
}