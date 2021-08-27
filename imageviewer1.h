#ifndef IMAGEVIEWER1_H
#define IMAGEVIEWER1_H

#include <QWidget>
#include <QTime>

#include <opencv2/opencv.hpp>

#include "globalstate.h"
#include "viewer.h"
#include "nodule.h"
#include "roi.h"

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
    void                            ClearAllViewers();

private slots:
    void                            SwitchOperateMode();
    void                            ToLungSegment();
    void                            ToProcess();
    void                            ToNoduleSegment();

private:
    void                            InitViewer(ViewName view_name, QVTKOpenGLWidget* widget);
    void                            MoveSlicePlus();
    void                            MoveSliceMinus();
    void                            DragSlice();
    void                            ZoomIn();
    void                            ZoomOut();
    void                            DrawROI();
    void                            AddNodule();
    
    void                            SetEnabledUIElements(bool enabled);
    void                            SaveCVImage(const std::string name, const cv::Mat& src) const;
    cv::Mat                         ConvertVTKImageToUCharCVMat(vtkImageData* img, int slice) const;
    cv::Mat                         ConvertVTKImageToShortCVMat(vtkImageData* img, int slice) const;
    cv::Mat                         ThresholdVTKImage(vtkImageData* img, int slice, int threshold, bool reverse) const;

private:
    Ui::ImageViewer1*               ui_;
    GlobalState*                    global_state_;
    std::map<ViewName, Viewer*>     viewer_map_;
    std::map<ViewName, ROI*>        roi_map_;
    std::vector<Nodule*>            nodule_list_;
};

template<typename T>
double Distance(T* p1, T* p2) 
{
    return std::sqrt((std::pow((p1[0] - p2[0]), 2)) + (std::pow((p1[1] - p2[1]), 2)));
}

#endif // IMAGEVIEWER1_H
