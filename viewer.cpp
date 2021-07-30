#include "viewer.h"

Viewer::Viewer(ViewName view_name, QVTKOpenGLWidget* widget) :
    view_name_(view_name),
    widget_(widget)
{
    
}

Viewer::~Viewer()
{
    widget_ = nullptr;
}

void Viewer::Init(DcmDataSet* data_set)
{

}