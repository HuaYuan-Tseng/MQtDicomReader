#include "imageviewer1.h"
#include "ui_imageviewer1.h"

#include <QTime>
#include <thread>

#include "cvtest.h"

ImageViewer1::ImageViewer1(GlobalState* state, QWidget *parent) :
    QWidget(parent),
    ui_(new Ui::ImageViewer1),
    global_state_(state)
{
    ui_->setupUi(this);
    ui_->imageButton_label_mode->setCheckable(true);
    ui_->imageButton_label_mode->setAutoExclusive(true);
    this->SetEnabledUIElements(false);
    
    QObject::connect(ui_->imageButton_Process, SIGNAL(clicked()), this, SLOT(ToProcess()));
    QObject::connect(ui_->imageButton_LungSegment, SIGNAL(clicked()), this, SLOT(ToLungSegment()));
    QObject::connect(ui_->imageButton_NoduleSegment, SIGNAL(clicked()), this, SLOT(ToNoduleSegment()));
    QObject::connect(ui_->imageButton_label_mode, &QPushButton::clicked, [&]() {
        global_state_->image_viewer_1_.current_operate_mode_ =
            (global_state_->image_viewer_1_.current_operate_mode_ == OperateMode::LABEL_NODULE) ?
            OperateMode::GENERAL : OperateMode::LABEL_NODULE;
        this->SwitchOperateMode();
    });
    
}

ImageViewer1::~ImageViewer1()
{
    delete ui_;
    global_state_ = nullptr;
}

void ImageViewer1::SetupViewers()
{
    this->ClearAllViewers();
    InitViewer(ViewName::TRA, ui_->vtk_viewer_0);
    
    global_state_->image_viewer_1_.Refresh();
    this->SwitchOperateMode();
    this->SetEnabledUIElements(true);
}

void ImageViewer1::InitViewer(ViewName view_name, QVTKOpenGLWidget* widget)
{
    Viewer* viewer = new Viewer(view_name, widget, global_state_);
    global_state_->study_browser_.dcm_data_set_.set_pixel_data_window_width(1500);
    global_state_->study_browser_.dcm_data_set_.set_pixel_data_window_center(-400);
    
    viewer->Init(global_state_->study_browser_.dcm_data_set_);
    viewer->image_interactor()->AddEvent(Event::REFRESH_VIEWER,     [&]{ RefreshViewer(); });
    viewer->image_interactor()->AddEvent(Event::MOVE_SLICE_PLUS,    [&]{ MoveSlicePlus(); });
    viewer->image_interactor()->AddEvent(Event::MOVE_SLICE_MINUS,   [&]{ MoveSliceMinus(); });
    viewer->image_interactor()->AddEvent(Event::DRAG_SLICE,         [&]{ DragSlice(); });
    viewer->image_interactor()->AddEvent(Event::ZOOM_IN,            [&]{ ZoomIn(); });
    viewer->image_interactor()->AddEvent(Event::ZOOM_OUT,           [&]{ ZoomOut(); });
    viewer->image_interactor()->AddEvent(Event::DRAW_ROI,           [&]{ DrawROI(); });
    viewer->image_interactor()->AddEvent(Event::ADD_NODULE,         [&]{ AddNodule(); });
    viewer_map_[view_name] = viewer;
}

void ImageViewer1::ClearAllViewers()
{
    if (!viewer_map_.empty())
    {
        for (auto& viewer : viewer_map_)
        {
            if (viewer.second != nullptr)
            {
                delete viewer.second;
                viewer.second = nullptr;
            }
        }
        viewer_map_.clear();
    }
    if (!drawing_roi_map_.empty())
    {
        for (auto& roi : drawing_roi_map_)
        {
            if (roi.second != nullptr)
            {
                delete roi.second;
                roi.second = nullptr;
            }
        }
        drawing_roi_map_.clear();
    }
    ui_->vtk_viewer_0->update();
    this->SetEnabledUIElements(false);
}

void ImageViewer1::SwitchOperateMode()
{
    // Reset
    this->ui_->imageButton_label_mode->setChecked(false);

    // Set Mode-checked
    switch (global_state_->image_viewer_1_.current_operate_mode_) {
    case OperateMode::GENERAL :
        break;
    case OperateMode::LABEL_NODULE :
        this->ui_->imageButton_label_mode->setChecked(true);
        break;
    }
}

void ImageViewer1::SetEnabledUIElements(bool enabled)
{
    ui_->imageButton_label_mode->setEnabled(enabled);
    ui_->imageButton_NoduleSegment->setEnabled(enabled);
    ui_->imageButton_LungSegment->setEnabled(enabled);
    ui_->imageButton_Process->setEnabled(enabled);
    ui_->vtk_viewer_0->setEnabled(enabled);
}

void ImageViewer1::RefreshViewer()
{
    viewer_map_[global_state_->image_viewer_1_.current_control_view_]->RefreshViewer();
}

void ImageViewer1::MoveSlicePlus()
{
    viewer_map_[global_state_->image_viewer_1_.current_control_view_]->MoveSlicePlus();
}

void ImageViewer1::MoveSliceMinus()
{
    viewer_map_[global_state_->image_viewer_1_.current_control_view_]->MoveSliceMinus();
}

void ImageViewer1::DragSlice()
{
    viewer_map_[global_state_->image_viewer_1_.current_control_view_]->DragSlice();
}

void ImageViewer1::ZoomIn()
{
    viewer_map_[global_state_->image_viewer_1_.current_control_view_]->Zoom(1.15);
}

void ImageViewer1::ZoomOut()
{
    viewer_map_[global_state_->image_viewer_1_.current_control_view_]->Zoom(0.85);
}

void ImageViewer1::DrawROI()
{
    ViewName name = global_state_->image_viewer_1_.current_control_view_;
    std::vector<double> spacing = viewer_map_[name]->spacing();
    
    if (drawing_roi_map_.find(name) == drawing_roi_map_.end())  
        drawing_roi_map_[name] = new ROI(name, spacing);
    ROI* roi = drawing_roi_map_[name];
    
    roi->set_world_top_left(global_state_->image_viewer_1_.control_map_[name].start_mouse_world_pos_);
    roi->set_world_bottom_right(global_state_->image_viewer_1_.control_map_[name].curr_mouse_world_pos_);
    roi->set_pixel_top_left(global_state_->image_viewer_1_.control_map_[name].start_mouse_pixel_pos_);
    roi->set_pixel_bottom_right(global_state_->image_viewer_1_.control_map_[name].curr_mouse_pixel_pos_);

    roi->set_vtk_actor();
    viewer_map_[global_state_->image_viewer_1_.current_control_view_]->DrawROI(roi);
}

void ImageViewer1::AddNodule()
{
    
}

cv::Mat ImageViewer1::ConvertVTKImageToUCharCVMat(vtkImageData* img, int slice) const
{
	double ww = global_state_->study_browser_.dcm_data_set_.pixel_data_window_width();
	double wc = global_state_->study_browser_.dcm_data_set_.pixel_data_window_center();
	double win_low = wc - 0.5 - (ww - 1) / 2;
	double win_high = wc - 0.5 + (ww - 1) / 2;
	const int row = img->GetDimensions()[1];
	const int col = img->GetDimensions()[0];
	unsigned char* gray = new unsigned char[row * col];
	unsigned char* ptr = gray;

	for (int i = 0; i < row; ++i)
	{
		unsigned short* data = static_cast<unsigned short*>(img->GetScalarPointer(0, i, slice));
		for (int j = 0; j < col; ++j)
		{
			short HU = (*data);
			if (HU <= win_low)
			{
				*(gray++) = 0;
			}
			else if (HU > win_high)
			{
				if (HU > SHRT_MAX)	*(gray++) = 0;
				else                *(gray++) = 255;
			}
			else
			{
				*(gray++) = static_cast<unsigned char>
					(255 * ((HU - (wc - 0.5)) / (ww + 1) + 0.5));
			}
			data++;
		}
	}

	cv::Mat res(row, col, CV_8UC1, ptr);
	cv::flip(res, res, 0);
	return res;
}

cv::Mat ImageViewer1::ConvertVTKImageToShortCVMat(vtkImageData* img, int slice) const
{
    const int row = img->GetDimensions()[1];
    const int col = img->GetDimensions()[0];
    short* gray = new short[row * col];
    short* ptr = gray;

    for (int i = 0; i < row; ++i)
    {
        unsigned short* data = static_cast<unsigned short*>(img->GetScalarPointer(0, i, slice));
        for (int j = 0; j < col; ++j)
        {
            *gray++ = *data++;
        }
    }

    cv::Mat res(row, col, CV_16SC1, ptr);
    cv::flip(res, res, 0);
    return res;
}

cv::Mat ImageViewer1::ThresholdVTKImage(vtkImageData* img, int slice, int threshold, bool reverse) const
{
    const int row = img->GetDimensions()[1];
    const int col = img->GetDimensions()[0];
    unsigned char* gray = new unsigned char[row * col];
    unsigned char* ptr = gray;
    int val1 = 0, val2 = 255;
    if (reverse) val1 = 255, val2 = 0;

    for (int i = 0; i < row; ++i)
    {
        unsigned short* data = static_cast<unsigned short*>(img->GetScalarPointer(0, i, slice));
        for (int j = 0; j < col; ++j)
        {
            short HU = *data++;
            if (HU < threshold)     *gray++ = val1;
            else                    *gray++ = val2;
        }
    }

    cv::Mat res(row, col, CV_8UC1, ptr);
    cv::flip(res, res, 0);
    return res;
}

void ImageViewer1::SaveCVImage(const std::string name, const cv::Mat& src) const
{
	// Output to view the result
	cv::imwrite("C:/Users/Rex/Desktop/" +
		QTime::currentTime().toString("hh-mm-ss").toStdString() + "-" + name + ".jpg", src);
}

CVTest cv_ls_src;
CVTest cv_ls_bin;
CVTest cv_ls_internal_marker;
CVTest cv_ls_external_marker;
CVTest cv_ls_watershed_marker;
CVTest cv_ls_lung_segment;

void ImageViewer1::ToLungSegment()
{
    if (viewer_map_.empty()) return;
    clock_t start, end;
    start = clock();

    auto& data_set = global_state_->study_browser_.dcm_data_set_;
    int total_slice = data_set.total_instances();
    int rows = data_set.rows();
    int cols = data_set.cols();

    // Src Image
    cv_ls_src.set_property(rows, cols, total_slice, "Src");
    auto build_src = [&](int slice)
    {
        for (int i = slice; i < total_slice; i += 2)
        {
            cv::Mat src = this->ConvertVTKImageToUCharCVMat(viewer_map_[ViewName::TRA]->image_data(), i);
            cv_ls_src.set_image(src, i);
        }
    };
    std::thread th_src_0(build_src, 0);
    std::thread th_src_1(build_src, 1);
    th_src_0.join();    th_src_1.join();
    cv_ls_src.Display();
    
    // Binary Image
    cv_ls_bin.set_property(rows, cols, total_slice, "Binary");
    auto build_bin = [&](int slice)
    {
        for (int i = slice; i < total_slice; i += 2)
        {
            cv::Mat hu_bin_img = this->ThresholdVTKImage(viewer_map_[ViewName::TRA]->image_data(), i, -400, true);
            cv_ls_bin.set_image(hu_bin_img, i);
        }
    };
    std::thread th_bin_0(build_bin, 0);
    std::thread th_bin_1(build_bin, 1);
    th_bin_0.join();    th_bin_1.join();
    cv_ls_bin.Display();

    // Internal Marker
    cv_ls_internal_marker.set_property(rows, cols, total_slice, "Internal Marker");
    cv_ls_internal_marker.set_image(cv_ls_bin.get_all_image());
    auto build_internal_marker = [&](int slice)
    {
        for (int i = slice; i < total_slice; i += 3)
        {
            cv::Mat hu_bin_img = cv_ls_internal_marker.get_image(i);

            cv::Mat label_img;
            cv::Mat stats;
            cv::Mat centroids;
            int label_cnt = 0;
            label_cnt = cv::connectedComponentsWithStats(hu_bin_img, label_img, stats, centroids);

            int row = hu_bin_img.rows;
            int col = hu_bin_img.cols;
            std::vector<uchar> color(label_cnt);
            for (int i = 0; i < label_cnt; ++i)
            {
                if (stats.at<int>(i, cv::CC_STAT_LEFT) <= 5 ||
                    stats.at<int>(i, cv::CC_STAT_TOP) <= 5 ||
                    stats.at<int>(i, cv::CC_STAT_LEFT) + stats.at<int>(i, cv::CC_STAT_WIDTH) >= col - 5 ||
                    stats.at<int>(i, cv::CC_STAT_TOP) + stats.at<int>(i, cv::CC_STAT_HEIGHT) >= row - 5 )
                    color[i] = 0;
                else
                    color[i] = 255;
            }

            for (int i = 0; i < row; ++i)
            {
                uchar* hu_ptr = hu_bin_img.ptr<uchar>(i);
                for (int j = 0; j < col; ++j)
                {
                    int label = label_img.at<int>(i, j);
                    hu_ptr[j] = color[label];
                }
            }

            cv::Mat res_elem = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
            cv::morphologyEx(hu_bin_img, hu_bin_img, cv::MORPH_OPEN, res_elem, cv::Point(-1, -1), 2);

            cv_ls_internal_marker.set_image(hu_bin_img, i);
        }
    };
    std::thread th_internal_0(build_internal_marker, 0);
    std::thread th_internal_1(build_internal_marker, 1);
    std::thread th_internal_2(build_internal_marker, 2);
    th_internal_0.join();  th_internal_1.join(); th_internal_2.join();
    cv_ls_internal_marker.Display();
    

    // External Marker
    cv_ls_external_marker.set_property(rows, cols, total_slice, "External Marker");
    cv_ls_external_marker.set_image(cv_ls_bin.get_all_image());
    auto build_external_marker = [&](int slice) 
    {
        for (int i = slice; i < total_slice; i += 3)
        {
            cv::Mat hu_bin_img = cv_ls_external_marker.get_image(i);
            
            cv::Mat elem_a = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
            cv::Mat elem_b = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(49, 49));
            cv::Mat external_a, external_b;
            cv::morphologyEx(hu_bin_img, external_a, cv::MORPH_DILATE, elem_a, cv::Point(-1, -1), 1);
            cv::morphologyEx(hu_bin_img, external_b, cv::MORPH_DILATE, elem_b, cv::Point(-1, -1), 1);
            cv::Mat external_res;
            external_res = external_a ^ external_b;

            cv_ls_external_marker.set_image(external_res, i);
        }
    };
    std::thread th_external_0(build_external_marker, 0);
    std::thread th_external_1(build_external_marker, 1);
    std::thread th_external_2(build_external_marker, 2);
    th_external_0.join();   th_external_1.join();   th_external_2.join();
    cv_ls_external_marker.Display();

    // Watershed Marker
    cv_ls_watershed_marker.set_property(rows, cols, total_slice, "Watershed Marker");
    auto build_watershed_marker = [&](int slice)
    {
        for (int i = slice; i < total_slice; i += 3)
        {
            cv::Mat res = cv::Mat::zeros(data_set.rows(), data_set.cols(), CV_8UC1);
            res += cv_ls_internal_marker.get_image(i) / 255 * 255;
            res += cv_ls_external_marker.get_image(i) / 255 * 128;
            cv_ls_watershed_marker.set_image(res, i);
        }
    };
    std::thread th_watershed_0(build_watershed_marker, 0);
    std::thread th_watershed_1(build_watershed_marker, 1);
    std::thread th_watershed_2(build_watershed_marker, 2);
    th_watershed_0.join();  th_watershed_1.join();  th_watershed_2.join();
    cv_ls_watershed_marker.Display();

    // Lung Segmentation
    cv_ls_lung_segment.set_property(rows, cols, total_slice, "Lung Segment");
    auto lung_segment = [&](int slice)
    {
        for (int i = slice; i < total_slice; i += 3)
        {
            cv::Mat src = cv_ls_src.get_image(i);

            // Sobel
            cv::Mat grad_x, grad_y;
            cv::Sobel(src, grad_x, CV_8UC1, 1, 0, 3);
            cv::Sobel(src, grad_y, CV_8UC1, 0, 1, 3);
            cv::convertScaleAbs(grad_x, grad_x);
            cv::convertScaleAbs(grad_y, grad_y);

            int max = -1;
            int row = data_set.rows();
            int col = data_set.cols();
            cv::Mat grad_res(row, col, CV_8UC1);
            for (int i = 0; i < row; ++i)
            {
                for (int j = 0; j < col; ++j)
                {
                    int gx = grad_x.at<uchar>(i, j);
                    int gy = grad_y.at<uchar>(i, j);
                    int grad = cv::saturate_cast<uchar>(gx + gy);
                    grad_res.at<uchar>(i, j) = grad;
                    if (grad > max) max = grad;
                }
            }
            if (max == 0) max = 255;
            grad_res = grad_res * (255 / max);

            // Watershed
            cv::Mat watershed_marker(row, col, CV_32SC1); 
            cv::Mat org_marker = cv_ls_watershed_marker.get_image(i);
            org_marker.convertTo(watershed_marker, CV_32SC1);
            cv::cvtColor(grad_res, grad_res, CV_GRAY2RGB);
            cv::watershed(grad_res, watershed_marker);

            cv::Mat watershed;
            watershed_marker.convertTo(watershed, CV_8UC1);
            cv::threshold(watershed, watershed, 128, 255, cv::THRESH_BINARY);

            cv::Mat outline;
            cv::Mat elem_out = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
            cv::morphologyEx(watershed, outline, cv::MORPH_GRADIENT, elem_out, cv::Point(-1, -1), 1);

            /*unsigned char arr[49] = { 0, 0, 1, 1, 1, 0, 0,
                                        0, 1, 1, 1, 1, 1, 0,
                                        1, 1, 1, 1, 1, 1, 1,
                                        1, 1, 1, 1, 1, 1, 1,
                                        1, 1, 1, 1, 1, 1, 1,
                                        0, 1, 1, 1, 1, 1, 0,
                                        0, 0, 1, 1, 1, 0, 0 };
            cv::Mat elem_black = cv::Mat(7, 7, CV_8UC1, arr);*/
            cv::Mat elem_black = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(9, 9));
            cv::Mat inclusion;
            cv::morphologyEx(outline, inclusion, cv::MORPH_BLACKHAT, elem_black, cv::Point(-1, -1), 2);
            outline += inclusion;

            cv::Mat lung_filter;
            cv::bitwise_or(cv_ls_internal_marker.get_image(i), outline, lung_filter);

            cv::Mat res;
            cv::Mat elem_lung = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
            cv::morphologyEx(lung_filter, res, cv::MORPH_CLOSE, elem_lung, cv::Point(-1, -1), 4);

            cv::Mat lung;
            src.copyTo(lung, res);

            cv_ls_lung_segment.set_image(lung, i);
        }
    };
    std::thread th_lung_0(lung_segment, 0);
    std::thread th_lung_1(lung_segment, 1);
    std::thread th_lung_2(lung_segment, 2);
    th_lung_0.join();   th_lung_1.join();   th_lung_2.join();
    cv_ls_lung_segment.Display();

    end = clock();
    double spend_time = (double)(end - start) / CLK_TCK;
    double one_slice_time = spend_time / total_slice;
    std::cout << "\nTotal spend time : " << spend_time << std::endl;
    std::cout << "One slice time   : " << one_slice_time << std::endl;
    
    while (1)
    {
        char key = cv::waitKey(0);
        if (key == 27)
        {
            cv::destroyAllWindows();
            cv_ls_src.ClearAll();
            cv_ls_bin.ClearAll();
            cv_ls_internal_marker.ClearAll();
            cv_ls_external_marker.ClearAll();
            cv_ls_watershed_marker.ClearAll();
            cv_ls_lung_segment.ClearAll();
            break;
        }
    }

}

CVTest cv_proc_src;
CVTest cv_proc_hu_bin;
CVTest cv_proc_remove_border;
CVTest cv_proc_lung_mask;
CVTest cv_proc_lung_area;

void ImageViewer1::ToProcess()
{
    // Lung Area Segment (another version).
    //
    clock_t start, end;
    start = clock();
    
    auto& data_set = global_state_->study_browser_.dcm_data_set_;
    data_set.TransformPixelData();
    int total_slice = data_set.total_instances();
    int rows = data_set.rows();
    int cols = data_set.cols();
    
    // 1. Take source image.
    cv_proc_src.set_property(rows, cols, total_slice, "Src");
    for (int i = 0; i < total_slice; ++i)
    {
        cv::Mat src(rows, cols, CV_8UC1, data_set.get_instance_pixel_data(i));
        cv_proc_src.set_image(src, i);
    }
    cv_proc_src.Display();
    
    // 2. Threshold Image (-400)
    cv_proc_hu_bin.set_property(rows, cols, total_slice, "HU bin");
    for (int i = 0; i < total_slice; ++i)
    {
        cv::Mat hu_bin = ThresholdVTKImage(viewer_map_[ViewName::TRA]->image_data(), i, -400, true);
        cv_proc_hu_bin.set_image(hu_bin, i);
    }
    cv_proc_hu_bin.Display();
    
    // 3 - 1. Remove the area that attached to image border.
    // 3 - 2. Keep the labels with 3 largest areas.
    cv_proc_remove_border.set_property(rows, cols, total_slice, "Remove Border");
    for (int i = 0; i < total_slice; ++i)
    {
        cv::Mat hu_bin = cv_proc_hu_bin.get_image(i);
        
        // Label image
        int label_cnt = 0;
        cv::Mat label, stats, centroids;
        label_cnt = cv::connectedComponentsWithStats(hu_bin, label, stats, centroids);

        // Sorting label's area (except background)
        // Usually, the background is "the area around the lung (spine. ribs. etc.)".
        // Because openCV will determine the "black area of bin image" is "background".
        int row = hu_bin.rows;
        int col = hu_bin.cols;
        int label_1 = label.at<int>(1, 1);
        int label_2 = label.at<int>(row - 1, col - 1);
        std::vector<int> area(label_cnt);
        for (int i = 1; i < label_cnt; ++i)
        {
            if (i == label_1 || i == label_2)
                area[i] = 0;
            else
                area[i] = stats.at<int>(i, cv::CC_STAT_AREA);
        }
        std::sort(area.begin(), area.end(), std::greater<int>());

        std::vector<uchar> color(label_cnt);
        color[0] = 0;
        for (int i = 1; i < label_cnt; ++i)
        {
            if (stats.at<int>(i, cv::CC_STAT_LEFT) <= 0 ||
                stats.at<int>(i, cv::CC_STAT_TOP) <= 0 ||
                stats.at<int>(i, cv::CC_STAT_LEFT) + stats.at<int>(i, cv::CC_STAT_WIDTH) >= col - 1 ||
                stats.at<int>(i, cv::CC_STAT_TOP) + stats.at<int>(i, cv::CC_STAT_HEIGHT) >= row - 1)
                color.at(i) = 0;
            else if (i == label_1 || i == label_2)
                color.at(i) = 0;
            else if (label_cnt > 3 && stats.at<int>(i, cv::CC_STAT_AREA) < area.at(2))
                color.at(i) = 0;
            else
                color.at(i) = 255;
        }
        
        cv::Mat res(row, col, CV_8UC1);
        for (int i = 0; i < row; ++i)
        {
            uchar* res_ptr = res.ptr<uchar>(i);
            for (int j = 0; j < col; ++j)
            {
                int n = label.at<int>(i, j);
                res_ptr[j] = color[n];
            }
        }
        
        cv_proc_remove_border.set_image(res, i);
    }
    cv_proc_remove_border.Display();
    
    cv_proc_lung_mask.set_property(rows, cols, total_slice, "Lung Mask");
    for (int i = 0; i < total_slice; ++i)
    {
        cv::Mat bin = cv_proc_remove_border.get_image(i);

        cv::Mat close_bin, close_elem;
        close_elem = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(21, 21));
        cv::morphologyEx(bin, close_bin, cv::MORPH_CLOSE, close_elem, cv::Point(-1, -1), 1);

        cv::Mat black_bin, black_elem;
        black_elem = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(21, 21));
        cv::morphologyEx(close_bin, black_bin, cv::MORPH_BLACKHAT, black_elem, cv::Point(-1, -1), 1);

        cv::Mat mask;
        cv::bitwise_or(close_bin, black_bin, mask);

        cv::Mat res(bin.rows, bin.cols, CV_8UC1, cv::Scalar(0));
        std::vector<std::vector<cv::Point>> contour;
        cv::findContours(mask, contour, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
        cv::drawContours(res, contour, -1, cv::Scalar(255), -1, 8);
        
        cv_proc_lung_mask.set_image(res, i);
    }
    cv_proc_lung_mask.Display();
    
    cv_proc_lung_area.set_property(rows, cols, total_slice, "Lung Area");
    for (int i = 0; i < total_slice; ++i)
    {
        cv::Mat src = cv_proc_src.get_image(i);
        cv::Mat lung_mask = cv_proc_lung_mask.get_image(i);
        cv::Mat res;
        src.copyTo(res, lung_mask);
        
        cv::cvtColor(res, res, cv::COLOR_GRAY2RGB);
        cv_proc_lung_area.set_image(res, i);
    }
    cv_proc_lung_area.Display();
    
    end = clock();
    double spend_time = (double)(end - start) / CLK_TCK;
    double one_slice_time = spend_time / total_slice;
    std::cout << "\nTotal spend time : " << spend_time << std::endl;
    std::cout << "One slice time   : " << one_slice_time << std::endl;
    
    while (1)
    {
        char key = cv::waitKey(0);
        if (key == 27) 
        {
            cv::destroyAllWindows();
            cv_proc_src.ClearAll();
            cv_proc_hu_bin.ClearAll();
            cv_proc_remove_border.ClearAll();
            cv_proc_lung_mask.ClearAll();
            cv_proc_lung_area.ClearAll();
            break;
        }
    }
}

CVTest  cv_nod_src;
CVTest  cv_nod_roi;

void ImageViewer1::ToNoduleSegment()
{
    clock_t start, end;
    start = clock();

    auto& data_set = global_state_->study_browser_.dcm_data_set_;
    data_set.TransformPixelData();
    int rows = data_set.rows();
    int cols = data_set.cols();

    double          diameter = 5.04;
    cv::Point3i     center(176, 286, 36);
    int x_radius = std::ceil(diameter / data_set.spacing_x() / 2) * 2;
    int y_radius = std::ceil(diameter / data_set.spacing_y() / 2) * 2;
    int z_radius = std::ceil(diameter / data_set.spacing_z() / 2);
    std::vector<int> x_range{ center.x - x_radius, center.x + x_radius };
    std::vector<int> y_range{ center.y - y_radius, center.y + y_radius };
    std::vector<int> z_range{ center.z - z_radius, center.z + z_radius };
    cv::Rect roi_rect(cv::Point2i(x_range.at(0), y_range.at(0)), 
                        cv::Point2i(x_range.at(1), y_range.at(1)));

    // 1. Take source image.
    std::map<int, cv::Mat> src_map;
    for (int i = z_range.at(0) - 1; i <= z_range.at(1) + 1; ++i)
    {
        src_map[i] = cv::Mat(rows, cols, CV_8UC1, data_set.get_instance_pixel_data(i));
        //SaveCVImage("Src - " + std::to_string(i), src_map[i]);
    }

    // 2. Take ROI.
    std::map<int, cv::Mat> roi_map;
    for (int i = z_range.at(0) - 1; i <= z_range.at(1) + 1; ++i)
    {
        roi_map[i] = src_map[i](roi_rect);
        //SaveCVImage("ROI - " + std::to_string(i), roi_map[i]);
    }

    // 3. Background estimation (optic flow image).
    std::map<int, cv::Mat> roi_bkg_map;
    int mid_slice = (z_range.at(0) + z_range.at(1)) / 2;
    for (int i = z_range.at(0); i <= z_range.at(1); ++i)
    {
        cv::Mat roi_ref;
        if (i >= mid_slice) roi_ref = roi_map[z_range.at(0) - 1];
        else                roi_ref = roi_map[z_range.at(1) + 1];
        cv::Mat roi = roi_map[i];

        cv::Mat flow;
        cv::calcOpticalFlowFarneback(roi_ref, roi, flow, 0.5, 1, 3, 3, 5, 1.1, 0);
        cv::Mat roi_bkg(roi.rows, roi.cols, CV_8UC1, cv::Scalar::all(0));
        for (int row = 0; row < roi.rows; ++row)
        {
            uchar* bkg_ptr = roi_bkg.ptr <uchar>(row);
            cv::Point2f* flow_ptr = flow.ptr<cv::Point2f>(row);
            for (int col = 0; col < roi.cols; ++col)
            {
                cv::Point2f displace = flow_ptr[col];
                cv::Point dest = cv::Point(col + cvRound(displace.x), row + cvRound(displace.y));
                if (dest.x < 0 || dest.x >= roi.cols || dest.y < 0 || dest.y >= roi.rows)
                    bkg_ptr[col] = roi_ref.at<uchar>(row, col);
                else
                    bkg_ptr[col] = roi_ref.at<uchar>(dest.y, dest.x);
            }
        }

        roi_bkg_map[i] = roi_bkg;
        //SaveCVImage("Bkg - " + std::to_string(i), roi_bkg_map[i]);
    }

    // 4. Background subtraction
    std::map<int, cv::Mat> roi_sub_map;
    for (int i = z_range.at(0); i <= z_range.at(1); ++i)
    {
        cv::Mat roi = roi_map[i];
        cv::Mat roi_bkg = roi_bkg_map[i];
        cv::Mat roi_sub(roi.rows, roi.cols, CV_8UC1, cv::Scalar::all(0));

        for (int row = 0; row < roi.rows; ++row)
        {
            uchar* roi_ptr = roi.ptr<uchar>(row);
            uchar* bkg_ptr = roi_bkg.ptr<uchar>(row);
            uchar* sub_ptr = roi_sub.ptr<uchar>(row);
            for (int col = 0; col < roi.cols; ++col)
            {
                int sub = roi_ptr[col] - bkg_ptr[col];
                sub = (sub < 0) ? 0 : sub;
                sub_ptr[col] = sub;
            }
        }
        roi_sub_map[i] = roi_sub;
        //SaveCVImage("Sub - " + std::to_string(i), roi_sub_map[i]);
    }

    // 5. Refinement (normalize. mean filter)
    std::map<int, cv::Mat> roi_ref_map;
    for (int i = z_range.at(0); i <= z_range.at(1); ++i)
    {
        cv::Mat roi_sub = roi_sub_map[i];
        cv::Mat roi_norm;
        cv::normalize(roi_sub, roi_norm, 1.0, 0.0, cv::NORM_INF, CV_32FC1);
        cv::Mat roi_ref;
        cv::blur(roi_norm, roi_ref, cv::Size(3, 3));
        roi_ref_map[i] = roi_ref;
    }

    // 6. Nodule Mask (OTSU. Dilate. Close)
    std::map<int, cv::Mat> nodule_mask_map;
    for (int i = z_range.at(0); i <= z_range.at(1); ++i)
    {
        cv::Mat roi_ref = roi_ref_map[i];
        cv::Mat roi_norm;
        cv::normalize(roi_ref, roi_norm, 0, 255, cv::NORM_MINMAX, CV_8UC1);
        cv::Mat roi_otsu;
        cv::threshold(roi_norm, roi_otsu, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

        cv::Mat dilate_elem = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
        cv::Mat roi_dilate;
        cv::morphologyEx(roi_otsu, roi_dilate, cv::MORPH_DILATE, dilate_elem, cv::Point(-1, -1), 1);

        cv::Mat close_elem = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
        cv::Mat roi_close;
        cv::morphologyEx(roi_dilate, roi_close, cv::MORPH_CLOSE, close_elem, cv::Point(-1, -1), 1);

        nodule_mask_map[i] = roi_close;
        //SaveCVImage("Nodule - " + std::to_string(i), nodule_mask_map[i]);
    }

    // Display
    std::map<int, int> area_map;
    for (int i = z_range.at(0); i <= z_range.at(1); ++i)
    {
        cv::Mat src = src_map[i];
        cv::Mat nodule_mask = nodule_mask_map[i];

        // Calculate area
        int area = 0;
        for (int row = 0; row < nodule_mask.rows; ++row)
        {
            uchar* mask_ptr = nodule_mask.ptr<uchar>(row);
            for (int col = 0; col < nodule_mask.cols; ++col)
            {
                if (mask_ptr[col] == 255) area += 1;
            }
        }
        std::cout << "Slice " << i << " : " << area << std::endl;
        area_map[i] = area;

        // Draw Contours
        cv::Mat res;
        src.copyTo(res);
        cv::cvtColor(res, res, cv::COLOR_GRAY2RGB);
        std::vector<std::vector<cv::Point>> contour_list;
        cv::findContours(nodule_mask, contour_list, cv::RETR_CCOMP, cv::CHAIN_APPROX_NONE);
        cv::drawContours(res, contour_list, -1, cv::Scalar(0, 0, 255), 1, 8, cv::noArray(), INT_MAX, roi_rect.tl());

        SaveCVImage("Res - " + std::to_string(i), res);
    }

}
