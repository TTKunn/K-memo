#include "kmemo.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    kmemo w;
    w.show();
    return a.exec();
}
