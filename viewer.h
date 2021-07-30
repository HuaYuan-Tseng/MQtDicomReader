#ifndef VIEWER_H
#define VIEWER_H

#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)
#include <vtkSmartPointer.h>
#include <vtkImageData.h>

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
    
    void                                Init(DcmDataSet* data_set);

    ViewName                            view_name() const { return view_name_; }
    vtkSmartPointer<vtkImageData>       image_data() const { return image_data_; }

    void                                set_spacing(const double spacing[3]) { std::memcpy(spacing_, spacing, sizeof(double) * 3); }
    const double*                       spacing() const { return spacing_; }

    

private:
    vtkSmartPointer<vtkImageData>       InitVtkImageData();

private:
    ViewName                            view_name_ = ViewName::TRA;
    QVTKOpenGLWidget*                   widget_ = nullptr;
    vtkSmartPointer<vtkImageData>       image_data_ = nullptr;

    double                              spacing_[3] = { 0 };
    int                                 dimension_[3] = { 0 };
    int                                 rescale_slope_ = 0;
    int                                 rescale_intercept_ = 0;
    int                                 display_window_width_ = 0;
    int                                 display_window_center_ = 0;

};

#endif // VIEWER_H
