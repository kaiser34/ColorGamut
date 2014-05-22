#include <QApplication>
#include <iostream>
#include "glwindow.h"

// Author: Wallas Henrique
// Requires Qt Framework

// Requires glm - OpenGL Mathematics
// https://github.com/g-truc/glm.git

// Shaders should be
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    GLWindow glWindow;
    glWindow.show();

    return a.exec();
}
