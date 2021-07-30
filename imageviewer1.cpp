#include "imageviewer1.h"
#include "ui_imageviewer1.h"


ImageViewer1::ImageViewer1(QWidget *parent) :
    QWidget(parent),
    ui_(new Ui::ImageViewer1)
{
    ui_->setupUi(this);
}

ImageViewer1::~ImageViewer1()
{
    delete ui_;
}

void ImageViewer1::SetupViewers()
{
    InitViewer(ViewName::TRA, ui_->vtk_viewer_0);
}

void ImageViewer1::InitViewer(ViewName view_name, QVTKOpenGLWidget* widget)
{
    Viewer* viewer = new Viewer(view_name, widget);
    double spacing[3];
    spacing[0] = GlobalState::study_browser_.dcm_data_set_.spacing_x();
    spacing[1] = GlobalState::study_browser_.dcm_data_set_.spacing_y();
    spacing[2] = GlobalState::study_browser_.dcm_data_set_.spacing_z();
    std::cout << "Send Spacing : " << spacing[0] << " , " << spacing[1] << " , " << spacing[2] << std::endl;

    viewer->set_spacing(spacing);

    const double* receive_spacing;
    receive_spacing = viewer->spacing();
    std::cout << "Receive Spacing : " << receive_spacing[0] << " , " << receive_spacing[1] << " , " << receive_spacing[2] << std::endl;

    viewer_map_[view_name] = viewer;
}
