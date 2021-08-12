#include "nodule.h"

Nodule::Nodule(ViewName name, std::vector<double> spacing, std::vector<int> dimension) :
	view_name_(name),
	spacing_(spacing),
	dimension_(dimension)
{
}

Nodule::~Nodule()
{
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
}

void Nodule::set_roi(const ROI& roi)
{
	label_init_slice_ = roi.pixel_top_left()[2];
    std::cout << label_init_slice_ << std::endl;
	contour_start_slice_ = contour_end_slice_ = label_init_slice_;
	roi_list_[label_init_slice_] = roi;
    std::cout << "Set roi" << std::endl;
    roi_list_[label_init_slice_].set_vtk_actor();
    std::cout << "Set roi actor" << std::endl;
}

void Nodule::set_contour(int slice, const Contour& contour)
{
	if (roi_list_.empty()) return;
	if (contour.empty() || contour.at(0).empty()) this->remove_contour(slice);

	contour_list_[slice] = contour;
	roi_list_[slice] = ConstructROIVTKActor(slice);
	vtk_contour_list_[slice] = ConstructContourVTKActor(slice, contour);
	
	this->CheckStartEndSlice();
}

bool Nodule::get_contour(int slice, Contour& contour) const
{
	if (contour_list_.find(slice) == contour_list_.end())
		return false;
	contour = contour_list_.at(slice);
	return true;
}

void Nodule::remove_contour(int slice)
{
	if (contour_list_.find(slice) == contour_list_.end()) return;
	contour_list_.erase(slice);
	vtk_contour_list_.erase(slice);
	roi_list_.erase(slice);
	this->CheckStartEndSlice();
}

bool Nodule::get_vtk_contour_actor(int slice, VTKContour& contour) const
{
	if (vtk_contour_list_.find(slice) == vtk_contour_list_.end())
		return false;
	contour = vtk_contour_list_.at(slice);
	return true;
}

bool Nodule::get_vtk_roi_actor(int slice, vtkSmartPointer<vtkActor> roi) const
{
	if (roi_list_.find(slice) == roi_list_.end())
		return false;
	roi = roi_list_.at(slice).vtk_actor();
	return true;
}

void Nodule::CheckStartEndSlice()
{
	using pair_type = decltype(contour_list_)::value_type;
	contour_start_slice_ = std::min_element(contour_list_.begin(), contour_list_.end(), 
		[](const pair_type& p1, const pair_type& p2) {
			return p1.first < p2.first;
	})->first;
	contour_end_slice_ = std::max_element(contour_list_.begin(), contour_list_.end(),
		[](const pair_type& p1, const pair_type& p2) {
			return p1.first < p2.first;
	})->first;
}

Nodule::VTKContour Nodule::ConstructContourVTKActor(int slice, const Contour& contour)
{
	std::vector<vtkSmartPointer<vtkActor>> vtk_contour;

	// contour on same slice
	for (const auto& list : contour)
	{
		vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
		double pt[3];

		// point on same contour
		for (const auto& p : list)
		{
			pt[0] = p.x * spacing_[0];
			pt[1] = (dimension_[1] - p.y - 1) * spacing_[1];
			pt[2] = (slice + 1) * spacing_[2] - spacing_[2] / 2;
			points->InsertNextPoint(pt);
		}
		pt[0] = list[0].x * spacing_[0];
		pt[1] = (dimension_[1] - list[0].y - 1) * spacing_[1];
		pt[2] = (slice + 1) * spacing_[2] - spacing_[2] / 2;
		points->InsertNextPoint(pt);

		vtkSmartPointer<vtkPolyLine> line = vtkSmartPointer<vtkPolyLine>::New();
		line->GetPointIds()->SetNumberOfIds(list.size() + 1);
		for (int i = 0; i < (list.size() + 1); ++i)
			line->GetPointIds()->SetId(i, i);

		vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
		cells->InsertNextCell(line);

		vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
		polydata->SetPoints(points);
		polydata->SetLines(cells);
		polydata->Modified();

		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputData(polydata);

		double color[3] = {0.8, 0.1, 0.1};
		vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);
		actor->PickableOff();
		actor->GetProperty()->SetColor(color);
		actor->GetProperty()->SetLineWidth(2);
		actor->Modified();

		vtk_contour.push_back(actor);
	}
	return vtk_contour;
}

ROI Nodule::ConstructROIVTKActor(int slice)
{
	ROI label_slice_roi = roi_list_[label_init_slice_];

	std::vector<double> world_tl = label_slice_roi.world_top_left();
	std::vector<double> world_br = label_slice_roi.world_bottom_right();
	std::vector<double> slice_world_tl(3);
	std::vector<double> slice_world_br(3);
	slice_world_tl[0] = world_tl[0];
	slice_world_tl[1] = world_tl[1];
	slice_world_tl[2] = (slice + 1) * spacing_[2] - spacing_[2] / 2;
	slice_world_br[0] = world_br[0];
	slice_world_br[1] = world_br[1];
	slice_world_br[2] = (slice + 1) * spacing_[2] - spacing_[2] / 2;

	std::vector<int> pixel_tl = label_slice_roi.pixel_top_left();
	std::vector<int> pixel_br = label_slice_roi.pixel_bottom_right();
	std::vector<int> slice_pixel_tl(3);
	std::vector<int> slice_pixel_br(3);
	slice_pixel_tl[0] = pixel_tl[0];
	slice_pixel_tl[1] = pixel_tl[1];
	slice_pixel_tl[2] = slice;
	slice_pixel_br[0] = pixel_br[0];
	slice_pixel_br[1] = pixel_br[1];
	slice_pixel_br[2] = slice;

	ROI slice_roi(view_name_, spacing_);
	slice_roi.set_world_top_left(slice_world_tl);
	slice_roi.set_world_bottom_right(slice_world_br);
	slice_roi.set_pixel_top_left(slice_pixel_tl);
	slice_roi.set_pixel_bottom_right(slice_pixel_br);
	slice_roi.set_vtk_actor();
	return slice_roi;
}
