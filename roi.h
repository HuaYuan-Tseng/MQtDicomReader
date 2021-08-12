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
	explicit	ROI(ViewName name, double* spacing);
				~ROI();
				

	ROI&		operator = (const ROI& roi);

	//---------------------------------------------------------------------//
	void						set_vtk_actor();
	void						set_vtk_actor(vtkSmartPointer<vtkActor> actor) { roi_actor_ = actor; }
	vtkSmartPointer<vtkActor>	vtk_actor() const { return roi_actor_; }

	void						set_spacing(double* spacing) { spacing_ = spacing; }
	double*						spacing() const { return spacing_; }

	void						set_world_top_left(double* position) { world_top_left_ = position; }
	double*						world_top_left() const { return world_top_left_; }

	void						set_world_bottom_right(double* position) { world_bottom_right_ = position; }
	double*						world_bottom_right() const { return world_bottom_right_; }

	void						set_pixel_top_left(int* position) { pixel_top_left_ = position; }
	int*						pixel_top_left() const { return pixel_top_left_; }

	void						set_pixel_bottom_right(int* position) { pixel_bottom_right_ = position; }
	int*						pixel_bottom_right() const { return pixel_bottom_right_; }

private:
	void						ConstructROIActor();

	ViewName					view_name_ = ViewName::TRA;
	vtkSmartPointer<vtkActor>	roi_actor_ = nullptr;

	double*						spacing_ = nullptr;
	double*						world_top_left_ = nullptr;
	double*						world_bottom_right_ = nullptr;
	int*						pixel_top_left_ = nullptr;
	int*						pixel_bottom_right_ = nullptr;
	
};

