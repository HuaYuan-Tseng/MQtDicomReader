#include "roi.h"

ROI::ROI(double* spacing, double* world_top_left, double* world_bottom_right) :
	spacing_(spacing),
	world_top_left_(world_top_left),
	world_bottom_right_(world_bottom_right)
{
	roi_actor_ = vtkSmartPointer<vtkActor>::New();
}

ROI::ROI(const ROI& roi)
{
	this->set_vtk_actor(roi.get_vtk_actor());

	double* src_spacing = roi.get_spacing();
	spacing_ = new double[3];
	std::copy(src_spacing, src_spacing + 3, spacing_);

	double* src_top_left = roi.get_world_top_left();
	world_top_left_ = new double[3];
	std::copy(src_top_left, src_top_left + 3, world_top_left_);

	double* src_bottom_right = roi.get_world_bottom_right();
	world_bottom_right_ = new double[3];
	std::copy(src_bottom_right, src_bottom_right + 3, world_bottom_right_);
}

ROI::~ROI()
{
	if (roi_actor_ != nullptr)				roi_actor_ = nullptr;
	if (spacing_ != nullptr)				delete[] spacing_;
	if (world_top_left_ != nullptr)			delete[] world_top_left_;
	if (world_bottom_right_ != nullptr)		delete[] world_bottom_right_;
	
}

ROI& ROI::operator = (const ROI& roi)
{
	this->set_vtk_actor(roi.get_vtk_actor());

	double* src_spacing = roi.get_spacing();
	if (spacing_ != nullptr)
	{
		delete[] spacing_;
		spacing_ = nullptr;
	}
	spacing_ = new double[3];
	std::copy(src_spacing, src_spacing + 3, spacing_);

	double* src_top_left = roi.get_world_top_left();
	if (world_top_left_ != nullptr)
	{
		delete[] world_top_left_;
		world_top_left_ = nullptr;
	}
	world_top_left_ = new double[3];
	std::copy(src_top_left, src_top_left + 3, world_top_left_);

	double* src_bottom_right = roi.get_world_bottom_right();
	if (world_bottom_right_ != nullptr)
	{
		delete[] world_bottom_right_;
		world_bottom_right_ = nullptr;
	}
	world_bottom_right_ = new double[3];
	std::copy(src_bottom_right, src_bottom_right + 3, world_bottom_right_);

	return *this;
}

void ROI::set_vtk_actor()
{
	this->ConstructROIActor();
}

void ROI::ConstructROIActor()
{
	if (roi_actor_ == nullptr) roi_actor_ = vtkSmartPointer<vtkActor>::New();

	// Vertex Order
	// 0 3
	// 1 2 
	double draw_z_pos = world_top_left_[2];
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	points->InsertNextPoint(world_top_left_[0], world_top_left_[1], draw_z_pos);
	points->InsertNextPoint(world_top_left_[0], world_bottom_right_[1], draw_z_pos);
	points->InsertNextPoint(world_bottom_right_[0], world_bottom_right_[1], draw_z_pos);
	points->InsertNextPoint(world_bottom_right_[0], world_top_left_[1], draw_z_pos);

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
