#include "viewertext.h"

auto GetWLWWType = [](int level, int width, int defaultLevel, int defaultWidth) {
	if (level == -400 && width == 1500) return "[Lung]";
	if (level == 300 && width == 600) return "[Angio]";
	if (level == 60 && width == 400) return "[Abdomen]";
	if (level == 300 && width == 1500) return "[Bone]";
	if (level == 40 && width == 80) return "[Brain]";
	if (level == 40 && width == 400) return "[Chest]";
	if (level == defaultLevel && width == defaultWidth) return "[Default]";
	return "";
};

ViewerText::~ViewerText()
{
    global_state_ = nullptr;
    image_viewer_ = nullptr;
}

void ViewerText::Init(ViewName name, GlobalState* state, vtkImageViewer2* viewer)
{
    view_name_ = name;
    global_state_ = state;
    image_viewer_ = viewer;
    image_viewer_->GetRenderer()->RemoveActor2D(top_left_text_);
    image_viewer_->GetRenderer()->RemoveActor2D(bottom_left_text_);
    image_viewer_->GetRenderer()->RemoveActor2D(axis_a_text_);
    image_viewer_->GetRenderer()->RemoveActor2D(axis_p_text_);
    image_viewer_->GetRenderer()->RemoveActor2D(axis_l_text_);
    image_viewer_->GetRenderer()->RemoveActor2D(axis_r_text_);
    image_viewer_->GetRenderer()->RemoveActor2D(axis_s_text_);
    image_viewer_->GetRenderer()->RemoveActor2D(axis_i_text_);
    top_left_text_ = vtkSmartPointer<vtkActor2D>::New();
    bottom_left_text_ = vtkSmartPointer<vtkActor2D>::New();
    axis_a_text_ = vtkSmartPointer<vtkActor2D>::New();
    axis_p_text_ = vtkSmartPointer<vtkActor2D>::New();
    axis_l_text_ = vtkSmartPointer<vtkActor2D>::New();
    axis_r_text_ = vtkSmartPointer<vtkActor2D>::New();
    axis_s_text_ = vtkSmartPointer<vtkActor2D>::New();
    axis_i_text_ = vtkSmartPointer<vtkActor2D>::New();
}

void ViewerText::Update()
{
    DcmDataSet& data_set = global_state_->study_browser_.dcm_data_set_;
    std::vector<int> pos = global_state_->image_viewer_1_.control_map_[view_name_].curr_mouse_pixel_pos_;
    int x = pos[0], y = pos[1], z = pos[2];
    int total_z = 0;
    if (data_set.frames_per_instance() == 1) total_z = data_set.total_instances();
    else                                    total_z = data_set.frames_per_instance();
    
    std::string val = "";
    if (x < 0) x = 0;   else if (x >= data_set.cols()) x = data_set.cols() - 1;
    if (y < 0) y = 0;   else if (y >= data_set.rows()) y = data_set.rows() - 1;
    if (z < 0) z = 0;   else if (z >= total_z) z = total_z - 1;
    
    int vtk_x = x, vtk_y = y, vtk_z = z;
    if (view_name_ == ViewName::TRA)    vtk_y = data_set.rows() - y - 1;
    else                                vtk_z = total_z - z - 1;
    
    if (image_viewer_->GetInput()->GetScalarType() == VTK_UNSIGNED_CHAR)
    {
        uchar* value = reinterpret_cast<unsigned char*>(image_viewer_->GetInput()->GetScalarPointer(vtk_x, vtk_y, vtk_z));
        val = std::to_string(*value);
    }
    else if (image_viewer_->GetInput()->GetScalarType() == VTK_SHORT)
    {
        short* value = reinterpret_cast<short*>(image_viewer_->GetInput()->GetScalarPointer(vtk_x, vtk_y, vtk_z));
        val = std::to_string(*value);
    }
    
    std::string ww_wl = "";
    std::vector<int> ww = data_set.window_width();
    std::vector<int> wl = data_set.window_center();
    int ww_wl_size = ww.size();
    for (int i = 0; i < ww_wl_size; ++i)
    {
        std::string res = GetWLWWType(image_viewer_->GetColorLevel(), image_viewer_->GetColorWindow(), wl[i], ww[i]);
        ww_wl = (res == "") ? ww_wl : res;
    }
    
    std::string top_left_str;
    std::string bottom_left_str;
    if (view_name_ == ViewName::TRA)
    {
        top_left_str = "[ TRA ] \n";
        top_left_str += "Im : " + std::to_string(image_viewer_->GetSlice()) + " / " + std::to_string(image_viewer_->GetSliceMax());
        
        bottom_left_str = "X : " + std::to_string(x) + " Y : " + std::to_string(y) + "  Val : " + val + "\n";
        bottom_left_str += "WW : " + std::to_string((int)image_viewer_->GetColorWindow()) + 
                            " WL : " + std::to_string((int)image_viewer_->GetColorLevel()) + " " + ww_wl;
        
        this->UpdateText("A", VTK_TEXT_TOP, VTK_TEXT_LEFT, 0.5, 0.97, axis_a_text_);
        this->UpdateText("R", VTK_TEXT_BOTTOM, VTK_TEXT_LEFT, 0.01, 0.5, axis_r_text_);
        this->UpdateText("P", VTK_TEXT_BOTTOM, VTK_TEXT_LEFT, 0.5, 0.02, axis_p_text_);
        this->UpdateText("L", VTK_TEXT_BOTTOM, VTK_TEXT_RIGHT, 0.98, 0.5, axis_l_text_);
    }
    else if (view_name_ == ViewName::COR)
    {
        top_left_str = "[ COR ] \n";
        top_left_str += "Im : " + std::to_string(image_viewer_->GetSlice()) + " / " + std::to_string(image_viewer_->GetSliceMax());
        
        bottom_left_str = "X : " + std::to_string(x) + " Y : " + std::to_string(z) + "  Val : " + val + "\n";
        bottom_left_str += "WW : " + std::to_string((int)image_viewer_->GetColorWindow()) + 
                            " WL : " + std::to_string((int)image_viewer_->GetColorLevel()) + " " + ww_wl;
        
        this->UpdateText("S", VTK_TEXT_TOP, VTK_TEXT_LEFT, 0.5, 0.97, axis_s_text_);
        this->UpdateText("R", VTK_TEXT_BOTTOM, VTK_TEXT_LEFT, 0.01, 0.5, axis_r_text_);
        this->UpdateText("I", VTK_TEXT_BOTTOM, VTK_TEXT_LEFT, 0.5, 0.02, axis_i_text_);
        this->UpdateText("L", VTK_TEXT_BOTTOM, VTK_TEXT_RIGHT, 0.98, 0.5, axis_l_text_);
    }
    else if (view_name_ == ViewName::SAG)
    {
        top_left_str = "[ SAG ] \n";
        top_left_str += "Im : " + std::to_string(image_viewer_->GetSlice()) + " / " + std::to_string(image_viewer_->GetSliceMax());
        
        bottom_left_str = "X : " + std::to_string(y) + " Y : " + std::to_string(y) + "  Val : " + val + "\n";
        bottom_left_str += "WW : " + std::to_string((int)image_viewer_->GetColorWindow()) + 
                            " WL : " + std::to_string((int)image_viewer_->GetColorLevel()) + " " + ww_wl;
        
        this->UpdateText("S", VTK_TEXT_TOP, VTK_TEXT_LEFT, 0.5, 0.97, axis_s_text_);
        this->UpdateText("A", VTK_TEXT_BOTTOM, VTK_TEXT_LEFT, 0.01, 0.5, axis_a_text_);
        this->UpdateText("I", VTK_TEXT_BOTTOM, VTK_TEXT_LEFT, 0.5, 0.02, axis_i_text_);
        this->UpdateText("P", VTK_TEXT_BOTTOM, VTK_TEXT_RIGHT, 0.98, 0.5, axis_p_text_);
    }
    
    this->UpdateText(top_left_str, VTK_TEXT_TOP, VTK_TEXT_LEFT, 0.13, 0.95, top_left_text_);
    this->UpdateText(bottom_left_str, VTK_TEXT_BOTTOM, VTK_TEXT_LEFT, 0.02, 0.02, bottom_left_text_);
}

double yellow[] = { 1.0, 1.0, 0.59 };
void ViewerText::UpdateText(std::string str, int justification, int vertical_justification,
                 double init_x, double init_y, vtkSmartPointer<vtkActor2D>& vtk_str)
{
    vtkSmartPointer<vtkTextMapper> textMapper = vtkSmartPointer<vtkTextMapper>::New();
    textMapper->SetInput(str.c_str());
    textMapper->GetTextProperty()->SetFontFamilyToCourier();
    textMapper->GetTextProperty()->SetFontSize(14);
    textMapper->GetTextProperty()->SetColor(yellow);
    textMapper->GetTextProperty()->SetVerticalJustification(vertical_justification);
    textMapper->GetTextProperty()->SetJustification(justification);
    vtk_str->SetMapper(textMapper);
    vtk_str->GetPositionCoordinate()->SetCoordinateSystemToNormalizedDisplay();
    vtk_str->GetPositionCoordinate()->SetValue(init_x, init_y);
    this->image_viewer_->GetRenderer()->AddActor2D(vtk_str);
}
