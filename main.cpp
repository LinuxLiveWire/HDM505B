#include <iostream>
#include <QApplication>

#include "compass.h"

using namespace std;

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    AppWindow win;
    win.show();
    a.exec();
    return 0;
}
