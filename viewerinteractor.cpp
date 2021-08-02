#include "viewerinteractor.h"

vtkStandardNewMacro(ViewerInteractor);

ViewerInteractor::~ViewerInteractor()
{
    global_state_ = nullptr;
    image_viewer_ = nullptr;
}

void ViewerInteractor::OnMouseMove()
{
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

void ViewerInteractor::OnLeftButtonDown()
{
}

void ViewerInteractor::OnLeftButtonUp()
{
}

void ViewerInteractor::OnRightButtonDown()
{
}

void ViewerInteractor::OnRightButtonUp()
{
}

void ViewerInteractor::OnKeyDown()
{
}

void ViewerInteractor::OnKeyUp()
{
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
