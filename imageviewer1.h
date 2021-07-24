#ifndef IMAGEVIEWER1_H
#define IMAGEVIEWER1_H

#include <QWidget>
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)
#include <QVTKOpenGLWidget.h>

#include "globalstate.h"

namespace Ui {
class ImageViewer1;
}

class ImageViewer1 : public QWidget
{
    Q_OBJECT

public:
    explicit ImageViewer1(QWidget *parent = nullptr);
    ~ImageViewer1();

public slots:
    void                        SetupViewers();


private:
    Ui::ImageViewer1*           ui_;

};

#endif // IMAGEVIEWER1_H
