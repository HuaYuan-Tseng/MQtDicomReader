#include "viewerinteractor.h"

vtkStandardNewMacro(ViewerInteractor);

ViewerInteractor::~ViewerInteractor()
{
    global_state_ = nullptr;
    image_viewer_ = nullptr;
    coordinate_ = nullptr;
}

void ViewerInteractor::OnMouseWheelForward()
{
    ConfirmCurrentControlView();
    event_map_[Event::MOVE_SLICE_PLUS]();
}

void ViewerInteractor::OnMouseWheelBackward()
{
    ConfirmCurrentControlView();
    event_map_[Event::MOVE_SLICE_MINUS]();
}

void ViewerInteractor::OnMouseMove()
{
    ConfirmCurrentControlView();
    int cur_y = this->Interactor->GetEventPosition()[1];
    int pre_y = this->Interactor->GetLastEventPosition()[1];

    double* world_pos = GetMouseWorldLocation();
    int image_pos[3];
        image_pos[0] = world_pos[0] / image_viewer_->GetInput()->GetSpacing()[0];
        image_pos[1] = image_viewer_->GetInput()->GetDimensions()[1] - 
                        (world_pos[1] / image_viewer_->GetInput()->GetSpacing()[1]);
        image_pos[2] = world_pos[2] / image_viewer_->GetInput()->GetSpacing()[2];

    global_state_->image_viewer_1_.control_map_[view_name_].curr_mouse_world_pos_[0] = world_pos[0];
    global_state_->image_viewer_1_.control_map_[view_name_].curr_mouse_world_pos_[1] = world_pos[1];
    global_state_->image_viewer_1_.control_map_[view_name_].curr_mouse_world_pos_[2] = world_pos[2];

    global_state_->image_viewer_1_.control_map_[view_name_].curr_mouse_pixel_pos_[0] = image_pos[0];
    global_state_->image_viewer_1_.control_map_[view_name_].curr_mouse_pixel_pos_[1] = image_pos[1];
    global_state_->image_viewer_1_.control_map_[view_name_].curr_mouse_pixel_pos_[2] = image_pos[2];

    if (global_state_->image_viewer_1_.is_draging_left_)
    {
        // Move Image
        if (global_state_->image_viewer_1_.is_pressed_shift_)
        {
            Superclass::OnMouseMove();
        }

        // Drag Slice
        if (global_state_->image_viewer_1_.is_pressed_ctrl_)
        {
            event_map_[Event::DRAG_SLICE]();
        }

        // Label Nodule
        if (global_state_->image_viewer_1_.current_operate_mode_ == OperateMode::LABEL_NODULE)
        {
            event_map_[Event::DRAG_ROI]();
        }
    }
    else if (global_state_->image_viewer_1_.is_draging_right_)
    {
        // Zoom Image
        if (global_state_->image_viewer_1_.is_pressed_shift_)
        {
            if (cur_y > pre_y)          event_map_[Event::ZOOM_IN]();
            else if (cur_y < pre_y)     event_map_[Event::ZOOM_OUT]();
        }
    }
}

void ViewerInteractor::OnLeftButtonDown()
{
    ConfirmCurrentControlView();
    global_state_->image_viewer_1_.is_draging_left_ = true;

    double* world_pos = GetMouseWorldLocation();
    int image_pos[3];
        image_pos[0] = world_pos[0] / image_viewer_->GetInput()->GetSpacing()[0];
        image_pos[1] = image_viewer_->GetInput()->GetDimensions()[1] -
                        (world_pos[1] / image_viewer_->GetInput()->GetSpacing()[1]);
        image_pos[2] = world_pos[2] / image_viewer_->GetInput()->GetSpacing()[2];

    global_state_->image_viewer_1_.control_map_[view_name_].start_mouse_world_pos_[0] = world_pos[0];
    global_state_->image_viewer_1_.control_map_[view_name_].start_mouse_world_pos_[1] = world_pos[1];
    global_state_->image_viewer_1_.control_map_[view_name_].start_mouse_world_pos_[2] = world_pos[2];

    global_state_->image_viewer_1_.control_map_[view_name_].start_mouse_pixel_pos_[0] = image_pos[0];
    global_state_->image_viewer_1_.control_map_[view_name_].start_mouse_pixel_pos_[1] = image_pos[1];
    global_state_->image_viewer_1_.control_map_[view_name_].start_mouse_pixel_pos_[2] = image_pos[2];

    // Move Image
    if (global_state_->image_viewer_1_.is_pressed_shift_)
    {
        Superclass::OnMiddleButtonDown();
    }
}

void ViewerInteractor::OnLeftButtonUp()
{
    ConfirmCurrentControlView();
    global_state_->image_viewer_1_.is_draging_left_ = false;

    double* world_pos = GetMouseWorldLocation();
    int image_pos[3];
        image_pos[0] = world_pos[0] / image_viewer_->GetInput()->GetSpacing()[0];
        image_pos[1] = image_viewer_->GetInput()->GetDimensions()[1] -
                        (world_pos[1] / image_viewer_->GetInput()->GetSpacing()[1]);
        image_pos[2] = world_pos[2] / image_viewer_->GetInput()->GetSpacing()[2];

    global_state_->image_viewer_1_.control_map_[view_name_].end_mouse_world_pos_[0] = world_pos[0];
    global_state_->image_viewer_1_.control_map_[view_name_].end_mouse_world_pos_[1] = world_pos[1];
    global_state_->image_viewer_1_.control_map_[view_name_].end_mouse_world_pos_[2] = world_pos[2];

    global_state_->image_viewer_1_.control_map_[view_name_].end_mouse_pixel_pos_[0] = image_pos[0];
    global_state_->image_viewer_1_.control_map_[view_name_].end_mouse_pixel_pos_[1] = image_pos[1];
    global_state_->image_viewer_1_.control_map_[view_name_].end_mouse_pixel_pos_[2] = image_pos[2];

    // Move Image
    if (global_state_->image_viewer_1_.is_pressed_shift_)
    {
        Superclass::OnMiddleButtonUp();
    }
}

void ViewerInteractor::OnRightButtonDown()
{
    ConfirmCurrentControlView();
    global_state_->image_viewer_1_.is_draging_right_ = true;

    double* world_pos = GetMouseWorldLocation();
    int image_pos[3];
        image_pos[0] = world_pos[0] / image_viewer_->GetInput()->GetSpacing()[0];
        image_pos[1] = image_viewer_->GetInput()->GetDimensions()[1] -
                        (world_pos[1] / image_viewer_->GetInput()->GetSpacing()[1]);
        image_pos[2] = world_pos[2] / image_viewer_->GetInput()->GetSpacing()[2];

    global_state_->image_viewer_1_.control_map_[view_name_].start_mouse_world_pos_[0] = world_pos[0];
    global_state_->image_viewer_1_.control_map_[view_name_].start_mouse_world_pos_[1] = world_pos[1];
    global_state_->image_viewer_1_.control_map_[view_name_].start_mouse_world_pos_[2] = world_pos[2];

    global_state_->image_viewer_1_.control_map_[view_name_].start_mouse_pixel_pos_[0] = image_pos[0];
    global_state_->image_viewer_1_.control_map_[view_name_].start_mouse_pixel_pos_[1] = image_pos[1];
    global_state_->image_viewer_1_.control_map_[view_name_].start_mouse_pixel_pos_[2] = image_pos[2];
}

void ViewerInteractor::OnRightButtonUp()
{
    ConfirmCurrentControlView();
    global_state_->image_viewer_1_.is_draging_right_ = false;

    double* world_pos = GetMouseWorldLocation();
    int image_pos[3];
        image_pos[0] = world_pos[0] / image_viewer_->GetInput()->GetSpacing()[0];
        image_pos[1] = image_viewer_->GetInput()->GetDimensions()[1] -
                        (world_pos[1] / image_viewer_->GetInput()->GetSpacing()[1]);
        image_pos[2] = world_pos[2] / image_viewer_->GetInput()->GetSpacing()[2];

    global_state_->image_viewer_1_.control_map_[view_name_].end_mouse_world_pos_[0] = world_pos[0];
    global_state_->image_viewer_1_.control_map_[view_name_].end_mouse_world_pos_[1] = world_pos[1];
    global_state_->image_viewer_1_.control_map_[view_name_].end_mouse_world_pos_[2] = world_pos[2];

    global_state_->image_viewer_1_.control_map_[view_name_].end_mouse_pixel_pos_[0] = image_pos[0];
    global_state_->image_viewer_1_.control_map_[view_name_].end_mouse_pixel_pos_[1] = image_pos[1];
    global_state_->image_viewer_1_.control_map_[view_name_].end_mouse_pixel_pos_[2] = image_pos[2];
}

void ViewerInteractor::OnKeyDown()
{
    vtkRenderWindowInteractor* rwi = this->Interactor;
    std::string key = rwi->GetKeySym();
    
    if (key == "Shift_L")
    {
        global_state_->image_viewer_1_.is_pressed_shift_ = true;
    }
    else if (key == "Control_L")
    {
        global_state_->image_viewer_1_.is_pressed_ctrl_ = true;
    }
    else if (key == "Alt_L")
    {
        global_state_->image_viewer_1_.is_pressed_alt_ = true;
    }
}

void ViewerInteractor::OnKeyUp()
{
    vtkRenderWindowInteractor* rwi = this->Interactor;
    std::string key = rwi->GetKeySym();

    if (key == "Shift_L")
    {
        global_state_->image_viewer_1_.is_pressed_shift_ = false;
    }
    else if (key == "Control_L")
    {
        global_state_->image_viewer_1_.is_pressed_ctrl_ = false;
    }
    else if (key == "Alt_L")
    {
        global_state_->image_viewer_1_.is_pressed_alt_ = false;
    }
}

double* ViewerInteractor::GetMouseWorldLocation()
{
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];
    coordinate_ = vtkSmartPointer<vtkCoordinate>::New();
    coordinate_->SetCoordinateSystemToDisplay();
    coordinate_->SetValue(x, y, 0);
    return coordinate_->GetComputedWorldValue(this->GetCurrentRenderer()->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
}

void ViewerInteractor::ConfirmCurrentControlView()
{
    global_state_->image_viewer_1_.current_control_view_ = view_name_;
    for(auto& view : global_state_->image_viewer_1_.control_map_)
    {
        if (view.first == view_name_)   
            view.second.is_control_current_view = true;
        else                            
            view.second.is_control_current_view = false;   
    }
}
