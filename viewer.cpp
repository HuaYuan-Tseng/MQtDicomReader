#include "viewer.h"

Viewer::Viewer(ViewName view_name, QVTKOpenGLWidget* widget, GlobalState* state) :
    view_name_(view_name),
    global_state_(state),
    widget_(widget)
{
    
}

Viewer::~Viewer()
{
    if (image_data_ != nullptr)
    {
        image_data_->ReleaseData();
        image_data_ = nullptr;
    }
    if (image_render_ != nullptr)  
    {
#ifdef Q_OS_MAC
        image_render_->ReleaseGraphicsResources(widget_->GetRenderWindow());
        image_render_ = nullptr;
#endif
#ifdef Q_OS_WIN
        image_render_->ReleaseGraphicsResources(render_window_);
        render_window_ = nullptr;
#endif
    }
    if (image_viewer_ != nullptr)       
    {
        image_viewer_->RemoveAllObservers();
        image_viewer_ = nullptr;
    }
    if (image_interactor_ != nullptr)  
    {
        image_interactor_->RemoveAllObservers();
        image_interactor_ = nullptr;
    }
    if (!spacing_.empty())
    {
        spacing_.clear();
        spacing_.shrink_to_fit();
    }
    if (!dimension_.empty())
    {
        dimension_.clear();
        dimension_.shrink_to_fit();
    }
    if (!clipping_range_.empty())
    {
        clipping_range_.clear();
        clipping_range_.shrink_to_fit();
    }
    
    widget_ = nullptr;
    global_state_ = nullptr;
    drawing_roi_ = nullptr;

}

void Viewer::Init(const DcmDataSet& data_set)
{
    double spacing[3];
    spacing[0] = data_set.spacing_x();
    spacing[1] = data_set.spacing_y();
    spacing[2] = (data_set.slice_thickness() != 0.0) ? 
                data_set.slice_thickness() : data_set.spacing_between_slice();
    //spacing[2] = (data_set.spacing_z() != 0.0) ? 
    //            data_set.spacing_z() : data_set.spacing_between_slice();
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

#ifdef Q_OS_WIN
    render_window_ = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
#endif

    image_viewer_ = vtkSmartPointer<vtkImageViewer2>::New();
    image_viewer_->SetInputData(image_data_);
#ifdef Q_OS_MAC
    image_viewer_->SetRenderWindow(widget_->GetRenderWindow());
#endif
#ifdef Q_OS_WIN
    image_viewer_->SetRenderWindow(render_window_);
#endif
    image_viewer_->Modified();
    
    image_render_ = image_viewer_->GetRenderer();
    image_render_->ResetCamera();
    image_render_->SetBackground(0.0, 0.0, 0.0);
    image_render_->GetActiveCamera()->ParallelProjectionOn();
    image_render_->Modified();
    
    image_interactor_ = vtkSmartPointer<ViewerInteractor>::New();
    image_interactor_->SetCurrentRenderer(image_render_);
    image_interactor_->SetAutoAdjustCameraClippingRange(false);
    image_interactor_->set_image_viewer(image_viewer_);
    image_interactor_->set_global_state(global_state_);
    image_interactor_->set_view_name(view_name_);
    image_interactor_->Modified();
 
#ifdef Q_OS_MAC
    widget_->GetInteractor()->SetRenderWindow(image_viewer_->GetRenderWindow());
#endif
#ifdef Q_OS_WIN
    widget_->SetRenderWindow(render_window_);
    widget_->GetInteractor()->SetRenderWindow(render_window_);
#endif
    widget_->GetInteractor()->SetInteractorStyle(image_interactor_);
    widget_->update();
    
    image_viewer_->Render();
    image_viewer_->SetColorLevel(window_center_);
    image_viewer_->SetColorWindow(window_width_);
    if (view_name_ == ViewName::TRA)
    {
        image_viewer_->SetSliceOrientationToXY();
        image_viewer_->SetSlice(0);
        this->FillView();
    }
    else if (view_name_ == ViewName::COR)
    {
        image_viewer_->SetSliceOrientationToXZ();
        image_viewer_->SetSlice(0);
        this->FillView();
    }
    else if (view_name_ == ViewName::SAG)
    {
        image_viewer_->SetSliceOrientationToYZ();
        image_viewer_->SetSlice(0);
        this->FillView();
    }
    this->RefreshViewer();
}

vtkSmartPointer<vtkImageData> Viewer::InitVTKImageData(const DcmDataSet& data_set)
{   
    vtkSmartPointer<vtkImageData> res = vtkSmartPointer<vtkImageData>::New();
    res->Initialize();
    res->SetOrigin(0, 0, 0);
    res->SetSpacing(spacing_[0], spacing_[1], spacing_[2]);
    res->SetDimensions(dimension_[0], dimension_[1], dimension_[2]);
    if (data_set.bits_allocated() <= 8)
    {
        res->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
        
        unsigned char* raw_data = nullptr;
        unsigned char* res_data = nullptr;
        for (int i = 0; i < dimension_[2]; ++i)
        {
            raw_data = (data_set.frames_per_instance() == 1) ? 
                        reinterpret_cast<unsigned char*>(data_set.get_instance_raw_data(i)) :
                        reinterpret_cast<unsigned char*>(data_set.get_frame_raw_data(0, i));
            for (int j = 0; j < dimension_[1]; ++j)
            {
                res_data = static_cast<unsigned char*>(res->GetScalarPointer(0, j, i));
                for (int k = 0; k < dimension_[0]; ++k)
                {
                    *res_data++ = (*raw_data++) * rescale_slope_ + rescale_intercept_;
                }
            }
        }
    }
    else
    {
        res->AllocateScalars(VTK_SHORT, 1);
        
        unsigned short* raw_data = nullptr;
        unsigned short* res_data = nullptr;
        for (int i = 0; i < dimension_[2]; ++i)
        {
            raw_data = (data_set.frames_per_instance() == 1) ? 
                        reinterpret_cast<unsigned short*>(data_set.get_instance_raw_data(i)) :
                        reinterpret_cast<unsigned short*>(data_set.get_frame_raw_data(0, i));
            for (int j = 0; j < dimension_[1]; ++j)
            {
                res_data = static_cast<unsigned short*>(res->GetScalarPointer(0, j, i));
                for (int k = 0; k < dimension_[0]; ++k)
                {
                    *res_data++ = (*raw_data++) * rescale_slope_ + rescale_intercept_;
                }
            }
        }
    }
    
    vtkSmartPointer<vtkImageFlip> flip = vtkSmartPointer<vtkImageFlip>::New();
    flip->SetInputData(res);
    if (view_name_ == ViewName::TRA) flip->SetFilteredAxes(1);
    else if (view_name_ == ViewName::COR) flip->SetFilteredAxes(2);
    else if (view_name_ == ViewName::SAG) flip->SetFilteredAxes(2);
    flip->Update();
    res = flip->GetOutput();

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
    if (view_name_ == ViewName::TRA)        depth_space = spacing_[2];
    else if (view_name_ == ViewName::COR)   depth_space = spacing_[1];
    else if (view_name_ == ViewName::SAG)   depth_space = spacing_[0];
    if (clipping_range_.empty()) clipping_range_.resize(2);
    clipping_range_[0] = range - depth_space / 2;
    clipping_range_[1] = range + depth_space / 2;
    //std::cout << "Clipping range : " << clipping_range_[0] << " , " << clipping_range_[1] << std::endl;
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
    global_state_->image_viewer_1_.control_map_[view_name_].control_current_slice = new_slice;
    this->RefreshViewer();
}

void Viewer::MoveSliceMinus()
{
    int new_slice = image_viewer_->GetSlice() - 1;
    image_viewer_->SetSlice(new_slice);
    global_state_->image_viewer_1_.control_map_[view_name_].control_current_slice = new_slice;
    this->RefreshViewer();
}

void Viewer::DragSlice()
{
    if (view_name_ == ViewName::TRA)
    {
        image_viewer_->SetSlice(image_viewer_->GetSlice() -
            (global_state_->image_viewer_1_.control_map_[view_name_].curr_mouse_world_pos_[1] -
                global_state_->image_viewer_1_.control_map_[view_name_].start_mouse_world_pos_[1]));

        global_state_->image_viewer_1_.control_map_[view_name_].start_mouse_world_pos_[1] =
           global_state_->image_viewer_1_.control_map_[view_name_].curr_mouse_world_pos_[1];

        global_state_->image_viewer_1_.control_map_[view_name_].control_current_slice = image_viewer_->GetSlice();
    }
    else if (view_name_ == ViewName::COR)
    {
        image_viewer_->SetSlice(image_viewer_->GetSlice() -
            (global_state_->image_viewer_1_.control_map_[view_name_].curr_mouse_world_pos_[2] -
                global_state_->image_viewer_1_.control_map_[view_name_].start_mouse_world_pos_[2]));

        global_state_->image_viewer_1_.control_map_[view_name_].start_mouse_world_pos_[2] =
            global_state_->image_viewer_1_.control_map_[view_name_].curr_mouse_world_pos_[2];

        global_state_->image_viewer_1_.control_map_[view_name_].control_current_slice = image_viewer_->GetSlice();
    }
    else if (view_name_ == ViewName::SAG)
    {
        image_viewer_->SetSlice(image_viewer_->GetSlice() -
            (global_state_->image_viewer_1_.control_map_[view_name_].curr_mouse_world_pos_[0] -
                global_state_->image_viewer_1_.control_map_[view_name_].start_mouse_world_pos_[0]));

        global_state_->image_viewer_1_.control_map_[view_name_].start_mouse_world_pos_[0] =
            global_state_->image_viewer_1_.control_map_[view_name_].curr_mouse_world_pos_[0];

        global_state_->image_viewer_1_.control_map_[view_name_].control_current_slice = image_viewer_->GetSlice();
    }
    this->RefreshViewer();
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

void Viewer::FillView()
{
    double* bounds = image_viewer_->GetRenderer()->ComputeVisiblePropBounds();
    double dim[3] = { (bounds[1] - bounds[0]) / 2, 
                        (bounds[3] - bounds[2]) / 2,
                        (bounds[5] - bounds[4]) / 2};
    int* win_size = image_viewer_->GetSize();
    double r = static_cast<double>(win_size[0]) / win_size[1];

    double x = 0.0, y = 0.0;
    if (view_name_ == ViewName::TRA)
    {
        x = dim[0];
        y = dim[1];
    }
    else if (view_name_ == ViewName::COR)
    {
        x = dim[0];
        y = dim[2];
    }
    else if (view_name_ == ViewName::SAG)
    {
        x = dim[1];
        y = dim[2];
    }

    if (r >= x / y)
        image_viewer_->GetRenderer()->GetActiveCamera()->SetParallelScale(y + 1);
    else
        image_viewer_->GetRenderer()->GetActiveCamera()->SetParallelScale(x / r + 1);

    this->RefreshViewer();
}

void Viewer::Zoom(const double rate)
{
    image_viewer_->GetRenderer()->GetActiveCamera()->Zoom(rate);
    zoom_rate_ = this->get_zoom_rate();
    this->RefreshViewer();
}

void Viewer::DrawROI(ROI* roi)
{
    if (drawing_roi_ != nullptr) image_viewer_->GetRenderer()->RemoveActor(drawing_roi_);
    drawing_roi_ = roi->vtk_actor();
    image_viewer_->GetRenderer()->AddActor(drawing_roi_);
    this->RefreshViewer();
    //std::cout << "Roi pos : " << drawing_roi_->GetCenter()[0] << " , " << drawing_roi_->GetCenter()[1] << " , " << drawing_roi_->GetCenter()[2] << std::endl;
}
