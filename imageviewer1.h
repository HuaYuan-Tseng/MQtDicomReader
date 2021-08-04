#ifndef IMAGEVIEWER1_H
#define IMAGEVIEWER1_H

#include <QWidget>

#include <opencv2/opencv.hpp>

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
    void                            ToProcess();

private:
    void                            InitViewer(ViewName view_name, QVTKOpenGLWidget* widget);
    void                            MoveSlicePlus();
    void                            MoveSliceMinus();
    void                            DragSlice();
    void                            ZoomIn();
    void                            ZoomOut();

    void                            SaveOpenCVImage(const std::string name, const cv::Mat& src) const;
    cv::Mat                         ConvertVTKImageToCVMat(vtkImageData* img, int slice) const;

private:
    Ui::ImageViewer1*               ui_;
    GlobalState*                    global_state_;

    std::map<ViewName, Viewer*>     viewer_map_;
};

#endif // IMAGEVIEWER1_H
