#ifndef VIEWERTEXT_H
#define VIEWERTEXT_H

#include <vtkSmartPointer.h>
#include <vtkImageViewer2.h>
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include <vtkActor2D.h>

#include "globalstate.h"

class ViewerText {
public:
    ViewerText() = default;
    ~ViewerText();
        
    void                Init(ViewName name, GlobalState* state, vtkImageViewer2* viewer);
    void                Update();
        
    //------------------------------------------------------------------------//
        
    void                set_view_name(ViewName name) { view_name_ = name; }
    ViewName            view_name() const { return view_name_; }
    
    void                set_global_state(GlobalState* state) { global_state_ = state; }
    
    void                set_image_viewer(vtkImageViewer2* viewer) { image_viewer_ = viewer; }
    vtkImageViewer2*    image_viewer() const { return image_viewer_; }
        
private:
    void                UpdateText(std::string str, int justification, int vertical_justification,
                                   double init_x, double init_y, vtkSmartPointer<vtkActor2D>& vtk_str);
    
    ViewName                    view_name_ = ViewName::TRA;
    GlobalState*                global_state_ = nullptr;
    vtkImageViewer2*            image_viewer_ = nullptr;
    
    vtkSmartPointer<vtkActor2D>   top_left_text_ = nullptr;
    vtkSmartPointer<vtkActor2D>   bottom_left_text_ = nullptr;
    vtkSmartPointer<vtkActor2D>   axis_a_text_ = nullptr;
    vtkSmartPointer<vtkActor2D>   axis_p_text_ = nullptr;
    vtkSmartPointer<vtkActor2D>   axis_l_text_ = nullptr;
    vtkSmartPointer<vtkActor2D>   axis_r_text_ = nullptr;
    vtkSmartPointer<vtkActor2D>   axis_s_text_ = nullptr;
    vtkSmartPointer<vtkActor2D>   axis_i_text_ = nullptr;
};

#endif // VIEWERTEXT_H
