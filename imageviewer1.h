#ifndef IMAGEVIEWER1_H
#define IMAGEVIEWER1_H

#include <QWidget>
#include <QTime>

#include <opencv2/opencv.hpp>

#include "globalstate.h"
#include "viewer.h"
#include "nodule.h"

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

private slots:
    void                            SwitchOperateMode();
    void                            ToLungSegment();
    void                            ToProcess();

private:
    void                            InitViewer(ViewName view_name, QVTKOpenGLWidget* widget);
    void                            MoveSlicePlus();
    void                            MoveSliceMinus();
    void                            DragSlice();
    void                            ZoomIn();
    void                            ZoomOut();
    void                            DrawROI();
    void                            AddNodule();

    void                            SaveOpenCVImage(const std::string name, const cv::Mat& src) const;
    cv::Mat                         ConvertVTKImageToUCharCVMat(vtkImageData* img, int slice) const;
    cv::Mat                         ConvertVTKImageToShortCVMat(vtkImageData* img, int slice) const;
    cv::Mat                         ThresholdVTKImage(vtkImageData* img, int slice, int threshold, bool reverse) const;

private:
    Ui::ImageViewer1*               ui_;
    GlobalState*                    global_state_;

    std::map<ViewName, Viewer*>     viewer_map_;
    std::vector<Nodule*>            nodule_list_;
};

#endif // IMAGEVIEWER1_H
