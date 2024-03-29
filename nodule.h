#ifndef NODULE_H
#define NODULE_H

#include <vtkPolyLine.h>
#include <opencv2/opencv.hpp>

#include "roi.h"

class Nodule {
public:
	using Contour = std::vector<std::vector<cv::Point>>;
	using VTKContour = std::vector<vtkSmartPointer<vtkActor>>;

							Nodule(ViewName name, std::vector<double> spacing, std::vector<int> dimension);
							~Nodule();

	void					set_roi(const ROI& roi);

	void					set_contour(int slice, const Contour& contour);
	bool                    get_contour(int slice, Contour& contour) const;		// C++17 : [[nodiscard]]
	void					remove_contour(int slice);

	bool                    get_vtk_contour_actor(int slice, VTKContour& contour) const;

	bool                    get_vtk_roi_actor(int slice, vtkSmartPointer<vtkActor>& roi) const;

	//------------------------------------------------------------------------//

	int						label_init_slice() const { return label_init_slice_; }

	int						contour_start_slice() const { return contour_start_slice_; }

	int						contour_end_slice() const { return contour_end_slice_; }

	const std::vector<int>& roi_tl() const { return roi_list_.at(label_init_slice()).pixel_top_left(); }

    const std::vector<int>& roi_br() const { return roi_list_.at(label_init_slice()).pixel_bottom_right(); }

	bool					IsSliceHaveContour(int slice) { return contour_list_.find(slice) != contour_list_.end(); }

private:
	VTKContour				ConstructContourVTKActor(int slice, const Contour& contour);
	ROI                     ConstructROIVTKActor(int slice);
	void					CheckStartEndSlice();

private:
	ViewName									view_name_ = ViewName::TRA;
    std::vector<double>                         spacing_ = {};
    std::vector<int>                            dimension_ = {};

	int											label_init_slice_ = 0;
	int											contour_start_slice_ = 0;
	int											contour_end_slice_ = 0;

	std::map<int, ROI>							roi_list_;
	std::map<int, Contour>						contour_list_;
	std::map<int, VTKContour>					vtk_contour_list_;

};

#endif
