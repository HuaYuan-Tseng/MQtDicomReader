#include "roi.h"

ROI::ROI(ViewName name, std::vector<double> spacing) :
	view_name_(name),
	spacing_(spacing)
{
	roi_actor_ = vtkSmartPointer<vtkActor>::New();
}

ROI::ROI(const ROI& roi)
{
	spacing_.assign(roi.spacing().begin(), roi.spacing().end());
    
    world_top_left_.assign(roi.world_top_left().begin(), roi.world_bottom_right().end());
    world_bottom_right_.assign(roi.world_top_left().begin(), roi.world_bottom_right().end());
    
    pixel_top_left_.assign(roi.pixel_top_left().begin(), roi.pixel_top_left().end());
    pixel_bottom_right_.assign(roi.pixel_bottom_right().begin(), roi.pixel_bottom_right().end());
    
    this->ConstructROIActor();
}

ROI::~ROI()
{
	roi_actor_ = nullptr;
	if (!spacing_.empty())
    {
        spacing_.clear();
        spacing_.shrink_to_fit();
    }
	if (!world_top_left_.empty())
    {
        world_top_left_.clear();
        world_top_left_.shrink_to_fit();
    }
	if (!world_bottom_right_.empty())
    {
        world_bottom_right_.clear();
        world_bottom_right_.shrink_to_fit();
    }
	if (!pixel_top_left_.empty())
    {
        pixel_top_left_.clear();
        pixel_top_left_.shrink_to_fit();
    }
	if (!pixel_bottom_right_.empty())
    {
        pixel_bottom_right_.clear();
        pixel_bottom_right_.shrink_to_fit();
    }
}

ROI& ROI::operator = (const ROI& roi)
{
	spacing_.assign(roi.spacing().begin(), roi.spacing().end());
    
    world_top_left_.assign(roi.world_top_left().begin(), roi.world_bottom_right().end());
    world_bottom_right_.assign(roi.world_top_left().begin(), roi.world_bottom_right().end());
    
    pixel_top_left_.assign(roi.pixel_top_left().begin(), roi.pixel_top_left().end());
    pixel_bottom_right_.assign(roi.pixel_bottom_right().begin(), roi.pixel_bottom_right().end());
    
    this->ConstructROIActor();
    
	return *this;
}

void ROI::set_vtk_actor()
{
	if (world_top_left_.empty() || world_bottom_right_.empty()) return;
	this->ConstructROIActor();
}

void ROI::ConstructROIActor()
{
    roi_actor_ = vtkSmartPointer<vtkActor>::New();

	// Vertex Order
	// 0 3
	// 1 2 
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    if (view_name_ == ViewName::TRA)
    {
        double world_z_pos = world_top_left_[2];
        points->InsertNextPoint(world_top_left_[0], world_top_left_[1], world_z_pos);
        points->InsertNextPoint(world_top_left_[0], world_bottom_right_[1], world_z_pos);
        points->InsertNextPoint(world_bottom_right_[0], world_bottom_right_[1], world_z_pos);
        points->InsertNextPoint(world_bottom_right_[0], world_top_left_[1], world_z_pos);
    }
    else
    {
        double world_z_pos = world_top_left_[1];
        points->InsertNextPoint(world_top_left_[0], world_top_left_[2], world_z_pos);
        points->InsertNextPoint(world_top_left_[0], world_bottom_right_[2], world_z_pos);
        points->InsertNextPoint(world_bottom_right_[0], world_bottom_right_[2], world_z_pos);
        points->InsertNextPoint(world_bottom_right_[0], world_top_left_[2], world_z_pos);
    }
	
	vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
	for (int i = 0; i < 4; ++i)
	{
		vtkIdType line[2] = { i, (i + 1) % 4 };
		cells->InsertNextCell(2, line);
	}

	vtkSmartPointer<vtkPolyData> rectangle = vtkSmartPointer<vtkPolyData>::New();
	rectangle->SetPoints(points);
	rectangle->SetLines(cells);
	rectangle->Modified();

	double color[3] = {0.1, 0.8, 0.1};
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(rectangle);
	roi_actor_->SetMapper(mapper);
	roi_actor_->PickableOff();
	roi_actor_->GetProperty()->SetColor(color);
	roi_actor_->GetProperty()->SetLineWidth(4.0);
	roi_actor_->Modified();

}
