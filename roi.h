#pragma once
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>

#include "globalstate.h"

class ROI {
public:
				
				ROI() = default;
				ROI(const ROI& roi);
	explicit	ROI(ViewName name, std::vector<double> spacing);
				~ROI();
				

	ROI&		operator = (const ROI& roi);

	//---------------------------------------------------------------------//
    
	void						set_vtk_actor();
	void						set_vtk_actor(vtkSmartPointer<vtkActor> actor) { roi_actor_ = actor; }
	vtkSmartPointer<vtkActor>	vtk_actor() const { return roi_actor_; }

	void						set_spacing(std::vector<double> spacing) { spacing_ = spacing; }
	const std::vector<double>&  spacing() const { return spacing_; }

	void						set_world_top_left(std::vector<double> pos) { world_top_left_ = pos; }
	const std::vector<double>&  world_top_left() const { return world_top_left_; }

	void						set_world_bottom_right(std::vector<double> pos) { world_bottom_right_ = pos; }
	const std::vector<double>&  world_bottom_right() const { return world_bottom_right_; }

	void						set_pixel_top_left(std::vector<int> pos) { pixel_top_left_ = pos; }
	const std::vector<int>&     pixel_top_left() const { return pixel_top_left_; }

	void						set_pixel_bottom_right(std::vector<int> pos) { pixel_bottom_right_ = pos; }
	const std::vector<int>&     pixel_bottom_right() const { return pixel_bottom_right_; }

private:
	void						ConstructROIActor();

	ViewName					view_name_ = ViewName::TRA;
	vtkSmartPointer<vtkActor>	roi_actor_ = nullptr;

    std::vector<double>         spacing_ = {};
    std::vector<double>         world_top_left_ = {};
    std::vector<double>         world_bottom_right_ = {};
    std::vector<int>            pixel_top_left_ = {};
    std::vector<int>            pixel_bottom_right_ = {};
	
};

