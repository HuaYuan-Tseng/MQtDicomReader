#ifndef VIEWER_H
#define VIEWER_H

#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageViewer2.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include "viewerinteractor.h"


#include <QVTKOpenGLWidget.h>
#include <QString>

#include "dcmdataset.h"

enum class ViewName {
    TRA, COR, SAG
};

class Viewer {
public:
    explicit                            Viewer(ViewName view_name, QVTKOpenGLWidget* widget);
                                        ~Viewer();
    
    void                                Init(const DcmDataSet& data_set);
    void                                InitVTKWidget(const DcmDataSet& data_set);
    vtkSmartPointer<vtkImageData>       InitVTKImageData(const DcmDataSet& data_set);
    
    
    //-------------------------------------------------------------------------------------------//

    ViewName                            view_name() const { return view_name_; }
    vtkSmartPointer<vtkImageData>       image_data() const { return image_data_; }

    void                                set_spacing(const double spacing[3]) { std::memcpy(spacing_, spacing, sizeof(double) * 3); }
    const double*                       spacing() const { return spacing_; }
    
    void                                set_diemnsion(const int dimension[3]) { std::memcpy(dimension_, dimension, sizeof(int) * 3); }
    const int*                          dimension() const { return dimension_; }
    
    void                                set_rescale_slope(const int slope) { rescale_slope_ = slope; }
    int                                 rescale_slope() const { return rescale_slope_; }
    
    void                                set_rescale_intercept(const int intercept) { rescale_intercept_ = intercept; }
    int                                 rescale_intercept() const { return rescale_intercept_; }
    
    void                                set_window_width(const int width) { window_width_ = width; }
    int                                 window_width() const { return window_width_; }
    
    void                                set_window_center(const int center) { window_center_ = center; }
    int                                 window_center() const { return window_center_; }

    

private:
    vtkSmartPointer<vtkImageData>       InitVtkImageData();

private:
    ViewName                                        view_name_ = ViewName::TRA;
    QVTKOpenGLWidget*                               widget_ = nullptr;
    vtkSmartPointer<vtkImageData>                   image_data_ = nullptr;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow>   render_window_ = nullptr;
    vtkSmartPointer<vtkImageViewer2>                image_viewer_ = nullptr;
    vtkSmartPointer<vtkRenderer>                    image_render_ = nullptr;
    vtkSmartPointer<ViewerInteractor>               image_interactor_ = nullptr;

    double                              spacing_[3] = { 0 };
    int                                 dimension_[3] = { 0 };
    int                                 rescale_slope_ = 0;
    int                                 rescale_intercept_ = 0;
    int                                 window_width_ = 0;
    int                                 window_center_ = 0;

};

#endif // VIEWER_H
