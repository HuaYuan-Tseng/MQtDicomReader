#include "imageviewer1.h"
#include "ui_imageviewer1.h"

#include <QTime>

ImageViewer1::ImageViewer1(GlobalState* state, QWidget *parent) :
    QWidget(parent),
    ui_(new Ui::ImageViewer1),
    global_state_(state)
{
    ui_->setupUi(this);

    QObject::connect(ui_->imageButton_Process, SIGNAL(clicked()), this, SLOT(ToProcess()));
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
    Viewer* viewer = new Viewer(view_name, widget, global_state_);
    global_state_->study_browser_.dcm_data_set_.set_pixel_data_window_width(1500);
    global_state_->study_browser_.dcm_data_set_.set_pixel_data_window_center(-400);
    viewer->Init(global_state_->study_browser_.dcm_data_set_);
    viewer->image_interactor()->AddEvent(Event::MOVE_SLICE_PLUS, [&]{ MoveSlicePlus(); });
    viewer->image_interactor()->AddEvent(Event::MOVE_SLICE_MINUS, [&]{ MoveSliceMinus(); });
    viewer->image_interactor()->AddEvent(Event::DRAG_SLICE, [&] { DragSlice(); });
    viewer->image_interactor()->AddEvent(Event::ZOOM_IN, [&]{ ZoomIn(); });
    viewer->image_interactor()->AddEvent(Event::ZOOM_OUT, [&]{ ZoomOut(); });
    viewer_map_[view_name] = viewer;
}

void ImageViewer1::MoveSlicePlus()
{
    viewer_map_[global_state_->image_viewer_1_.current_control_view_]->MoveSlicePlus();
}

void ImageViewer1::MoveSliceMinus()
{
    viewer_map_[global_state_->image_viewer_1_.current_control_view_]->MoveSliceMinus();
}

void ImageViewer1::DragSlice()
{
    viewer_map_[global_state_->image_viewer_1_.current_control_view_]->DragSlice();
}

void ImageViewer1::ZoomIn()
{
    viewer_map_[global_state_->image_viewer_1_.current_control_view_]->Zoom(1.15);
}

void ImageViewer1::ZoomOut()
{
    viewer_map_[global_state_->image_viewer_1_.current_control_view_]->Zoom(0.85);
}

void ImageViewer1::SaveOpenCVImage(const std::string name, const cv::Mat& src) const
{
    // Output to view the result
    cv::imwrite("C:/Users/Rex/Desktop/" +
        QTime::currentTime().toString("hh-mm-ss").toStdString() + "-" + name + ".jpg", src);
}

void ImageViewer1::ToProcess()
{
    if (viewer_map_.empty()) return;
    auto& data = global_state_->study_browser_.dcm_data_set_;
    cv::Mat src(data.rows(), data.cols(), CV_8UC1, data.get_instance_pixel_data(0));
    cv::imshow("src", src);

}



