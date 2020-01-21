#include "maingraph.h"

#include <QApplication>
#include <iostream>
#include <QTextStream>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainGraph w(&a);
    w.show();
    return a.exec();
}
