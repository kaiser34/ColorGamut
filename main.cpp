#include <QApplication>
#include <iostream>
#include "glwindow.h"

// Author: Wallas Henrique
// Requires Qt Framework

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    GLWindow glWindow;
    glWindow.show();

    return a.exec();
}
