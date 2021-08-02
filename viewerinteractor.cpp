#include "viewerinteractor.h"

vtkStandardNewMacro(ViewerInteractor);

ViewerInteractor::ViewerInteractor()
{
}

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
}

void ViewerInteractor::OnMouseWheelBackward()
{
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
