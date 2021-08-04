#include "mainwindow.h"

#include <QApplication>
#include <QSurface>

int main(int argc, char *argv[])
{
    // Identify OpenGL Version (Neeed !!)
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLWidget::defaultFormat());

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
