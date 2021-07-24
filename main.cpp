#include "mainwindow.h"

#include <QApplication>
#include <QSurface>

int main(int argc, char *argv[])
{
    // 若不添加此行，則會有OpenGL版本的識別錯誤
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLWidget::defaultFormat());

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
