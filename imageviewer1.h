#ifndef IMAGEVIEWER1_H
#define IMAGEVIEWER1_H

#include <QWidget>

#include "globalstate.h"
#include "viewer.h"

namespace Ui {
class ImageViewer1;
}

class ImageViewer1 : public QWidget
{
    Q_OBJECT

public:
    explicit                        ImageViewer1(GlobalState* state, QWidget *parent = nullptr);
                                    ~ImageViewer1();

public slots:
    void                            SetupViewers();

private:
    void                            InitViewer(ViewName view_name, QVTKOpenGLWidget* widget);
    void                            MoveSlicePlus();
    void                            MoveSliceMinus();

private:
    Ui::ImageViewer1*               ui_;
    GlobalState*                    global_state_;

    std::map<ViewName, Viewer*>     viewer_map_;
};

#endif // IMAGEVIEWER1_H
