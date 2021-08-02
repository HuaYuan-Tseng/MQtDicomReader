#include "viewer.h"

Viewer::Viewer(ViewName view_name, QVTKOpenGLWidget* widget, GlobalState* state) :
    view_name_(view_name),
    global_state_(state),
    widget_(widget)
{
    
}

Viewer::~Viewer()
{
    widget_ = nullptr;
    global_state_ = nullptr;

    if (image_data_ != nullptr)         image_data_->ReleaseData();
    if (image_render_ != nullptr)       image_render_->ReleaseGraphicsResources(render_window_);
    if (image_viewer_ != nullptr)       image_viewer_->RemoveAllObservers();
    if (image_interactor_ != nullptr)   image_interactor_->RemoveAllObservers();
}

void Viewer::Init(const DcmDataSet& data_set)
{
    double spacing[3];
    spacing[0] = data_set.spacing_x();
    spacing[1] = data_set.spacing_y(); 
    spacing[2] = (data_set.spacing_z() != 0.0) ? 
                data_set.spacing_z() : data_set.spacing_between_slice();
    this->set_spacing(spacing);
    
    int dimension[3];
    dimension[0] = data_set.cols();
    dimension[1] = data_set.rows();
    dimension[2] = (data_set.frames_per_instance() == 1) ? 
                data_set.total_instances() : data_set.frames_per_instance();
    this->set_diemnsion(dimension);
    
    this->set_rescale_slope(data_set.rescale_slope());
    this->set_rescale_intercept(data_set.rescale_intercept());
    this->set_window_width(data_set.pixel_data_window_width());
    this->set_window_center(data_set.pixel_data_window_center());
    
    this->InitVTKWidget(data_set);
}

void Viewer::InitVTKWidget(const DcmDataSet& data_set)
{   
    if (image_data_ != nullptr) image_data_->ReleaseData();
    image_data_ = vtkSmartPointer<vtkImageData>::New();
    image_data_ = InitVTKImageData(data_set);
    
    render_window_ = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    
    image_viewer_ = vtkSmartPointer<vtkImageViewer2>::New();
    image_viewer_->SetInputData(image_data_);
    image_viewer_->SetRenderWindow(render_window_);
    image_viewer_->Modified();
    
    image_render_ = image_viewer_->GetRenderer();
    image_render_->ResetCamera();
    image_render_->SetBackground(0.0, 0.0, 0.0);
    image_render_->GetActiveCamera()->ParallelProjectionOn();
    
    image_interactor_ = vtkSmartPointer<ViewerInteractor>::New();
    image_interactor_->SetCurrentRenderer(image_render_);
    image_interactor_->SetAutoAdjustCameraClippingRange(false);
    image_interactor_->set_image_viewer(image_viewer_);
    image_interactor_->set_global_state(global_state_);
    image_interactor_->set_view_name(view_name_);
    image_interactor_->Modified();
    
    widget_->SetRenderWindow(render_window_);
    widget_->GetInteractor()->SetInteractorStyle(image_interactor_);
    widget_->update();
    
    image_viewer_->SetColorLevel(window_center_);
    image_viewer_->SetColorWindow(window_width_);
    switch (view_name_) {
    case ViewName::TRA :
        image_viewer_->SetSliceOrientationToXY();
        image_viewer_->SetSlice(0);
        break;
    case ViewName::SAG :
        image_viewer_->SetSliceOrientationToYZ();
        image_viewer_->SetSlice(0);
        break;
    case ViewName::COR :
        image_viewer_->SetSliceOrientationToXZ();
        image_viewer_->SetSlice(0);
        break;
    }

    image_viewer_->Modified();
    this->set_clipping_range();
    this->RefreshViewer();
}

vtkSmartPointer<vtkImageData> Viewer::InitVTKImageData(const DcmDataSet& data_set)
{
    vtkSmartPointer<vtkImageData> res = vtkSmartPointer<vtkImageData>::New();
    res->Initialize();
    res->SetOrigin(0, 0, 0);
    res->SetSpacing(spacing_[0], spacing_[1], spacing_[2]);
    res->SetDimensions(dimension_[0], dimension_[1], dimension_[2]);
    res->AllocateScalars(VTK_SHORT, 1);
    
    unsigned short* raw_data = nullptr;
    unsigned short* res_data = nullptr;
    for (int i = 0; i < dimension_[2]; ++i)
    {
        raw_data = (data_set.frames_per_instance() == 1) ? 
                    reinterpret_cast<unsigned short*>(data_set.get_instance_raw_data(i)) :
                    reinterpret_cast<unsigned short*>(data_set.get_frame_raw_data(0, i));
        for (int j = dimension_[1] - 1; j >= 0; --j)
        {
            res_data = static_cast<unsigned short*>(res->GetScalarPointer(0, j, i));
            for (int k = 0; k < dimension_[0]; ++k)
            {
                *res_data = (*raw_data) * rescale_slope_ + rescale_intercept_;
                ++res_data;
                ++raw_data;
            }
        }
    }
    
    return res;
}

void Viewer::set_clipping_range()
{
    if (image_viewer_ == nullptr) return;
    double* bounds = image_viewer_->GetImageActor()->GetBounds();
    double spos = bounds[image_viewer_->GetSliceOrientation() * 2];
    double cpos = image_viewer_->GetRenderer()->GetActiveCamera()->GetPosition()[image_viewer_->GetSliceOrientation()];
    double range = fabs(spos - cpos);
    double depth_space = 0.0;
    switch (view_name_) {
    case ViewName::TRA :
        depth_space = spacing_[2];
        break;
    case ViewName::SAG :
        depth_space = spacing_[0];
        break;
    case ViewName::COR :
        depth_space = spacing_[1];
        break;
    }
    if (clipping_range_ != nullptr) delete[] clipping_range_;
    clipping_range_ = new double[2];
    clipping_range_[0] = range - depth_space / 2;
    clipping_range_[1] = range + depth_space / 2;
}

double Viewer::get_zoom_rate() const
{
    vtkSmartPointer<vtkCoordinate> coordinate = vtkSmartPointer<vtkCoordinate>::New();
    coordinate->SetValue(0, 0, 0);
    int* res = coordinate->GetComputedDisplayValue(image_render_);
    int val = res[0];
    coordinate->SetValue(dimension_[0], 0, 0);
    res = coordinate->GetComputedDisplayValue(image_render_);
    return static_cast<double>((res[0] - val) / dimension_[0]);
}

void Viewer::RefreshViewer()
{
    this->set_clipping_range();
    image_viewer_->GetRenderer()->GetActiveCamera()->SetClippingRange(clipping_range_[0], clipping_range_[1]);
    image_viewer_->Render();
}

void Viewer::MoveSlicePlus()
{
    int new_slice = image_viewer_->GetSlice() + 1;
    image_viewer_->SetSlice(new_slice);
    this->RefreshViewer();
}

void Viewer::MoveSliceMinus()
{
    int new_slice = image_viewer_->GetSlice() - 1;
    image_viewer_->SetSlice(new_slice);
    this->RefreshViewer();
}
