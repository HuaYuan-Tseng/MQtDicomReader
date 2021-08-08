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

    QObject::connect(ui_->imageButton_Process, SIGNAL(clicked()), this, SLOT(ToProcess()));
}

ImageViewer1::~ImageViewer1()
{
    delete ui_;
}

void ImageViewer1::SetupViewers()
{
    InitViewer(ViewName::TRA, ui_->vtk_viewer_0);
}

void ImageViewer1::InitViewer(ViewName view_name, QVTKOpenGLWidget* widget)
{
    if (viewer_map_.find(view_name) != viewer_map_.end()) 
        viewer_map_.erase(view_name);
    
    Viewer* viewer = new Viewer(view_name, widget, global_state_);
    global_state_->study_browser_.dcm_data_set_.set_pixel_data_window_width(1500);
    global_state_->study_browser_.dcm_data_set_.set_pixel_data_window_center(-400);
    viewer->Init(global_state_->study_browser_.dcm_data_set_);
    viewer->image_interactor()->AddEvent(Event::MOVE_SLICE_PLUS, [&]{ MoveSlicePlus(); });
    viewer->image_interactor()->AddEvent(Event::MOVE_SLICE_MINUS, [&]{ MoveSliceMinus(); });
    viewer->image_interactor()->AddEvent(Event::DRAG_SLICE, [&] { DragSlice(); });
    viewer->image_interactor()->AddEvent(Event::ZOOM_IN, [&]{ ZoomIn(); });
    viewer->image_interactor()->AddEvent(Event::ZOOM_OUT, [&]{ ZoomOut(); });
    viewer_map_[view_name] = viewer;
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

void ImageViewer1::SaveOpenCVImage(const std::string name, const cv::Mat& src) const
{
	// Output to view the result
	cv::imwrite("C:/Users/Rex/Desktop/" +
		QTime::currentTime().toString("hh-mm-ss").toStdString() + "-" + name + ".jpg", src);
}

CVTest cv_src;
CVTest cv_bin;
CVTest cv_internal_marker;
CVTest cv_external_marker;
CVTest cv_watershed_marker;
CVTest cv_lung_segment;

void ImageViewer1::ToProcess()
{
    if (viewer_map_.empty()) return;
    auto& data_set = global_state_->study_browser_.dcm_data_set_;
    int total_slice = data_set.total_instances();
    int rows = data_set.rows();
    int cols = data_set.cols();

    // Src Image
    cv_src.set_property(rows, cols, total_slice, "Src");
    auto build_src = [&](int slice)
    {
        for (int i = slice; i < total_slice; i += 2)
        {
            cv::Mat src = this->ConvertVTKImageToUCharCVMat(viewer_map_[ViewName::TRA]->image_data(), i);
            cv_src.set_image(src, i);
        }
    };
    std::thread th_src_0(build_src, 0);
    std::thread th_src_1(build_src, 1);
    th_src_0.join();    th_src_1.join();
    cv_src.display();
    
    // Binary Image
    cv_bin.set_property(rows, cols, total_slice, "Binary");
    auto build_bin = [&](int slice)
    {
        for (int i = slice; i < total_slice; i += 2)
        {
            cv::Mat hu_bin_img = this->ThresholdVTKImage(viewer_map_[ViewName::TRA]->image_data(), i, -400, true);
            cv_bin.set_image(hu_bin_img, i);
        }
    };
    std::thread th_bin_0(build_bin, 0);
    std::thread th_bin_1(build_bin, 1);
    th_bin_0.join();    th_bin_1.join();
    cv_bin.display();

    // Internal Marker
    cv_internal_marker.set_property(rows, cols, total_slice, "Internal Marker");
    cv_internal_marker.set_image(cv_bin.get_all_image());
    auto build_internal_marker = [&](int slice)
    {
        for (int i = slice; i < total_slice; i += 3)
        {
            cv::Mat hu_bin_img = cv_internal_marker.get_image(i);

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
                if (stats.at<int>(i, cv::CC_STAT_LEFT) == 0 ||
                    stats.at<int>(i, cv::CC_STAT_TOP) == 0 ||
                    stats.at<int>(i, cv::CC_STAT_LEFT) + stats.at<int>(i, cv::CC_STAT_WIDTH) >= col - 1 ||
                    stats.at<int>(i, cv::CC_STAT_TOP) + stats.at<int>(i, cv::CC_STAT_HEIGHT) >= row - 1 )
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

            cv_internal_marker.set_image(hu_bin_img, i);
        }
    };
    std::thread th_internal_0(build_internal_marker, 0);
    std::thread th_internal_1(build_internal_marker, 1);
    std::thread th_internal_2(build_internal_marker, 2);
    th_internal_0.join();  th_internal_1.join(); th_internal_2.join();
    cv_internal_marker.display();
    

    // External Marker
    cv_external_marker.set_property(rows, cols, total_slice, "External Marker");
    cv_external_marker.set_image(cv_bin.get_all_image());
    auto build_external_marker = [&](int slice) 
    {
        for (int i = slice; i < total_slice; i += 3)
        {
            cv::Mat hu_bin_img = cv_external_marker.get_image(i);
            
            cv::Mat elem_a = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
            cv::Mat elem_b = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(49, 49));
            cv::Mat external_a, external_b;
            cv::morphologyEx(hu_bin_img, external_a, cv::MORPH_DILATE, elem_a, cv::Point(-1, -1), 1);
            cv::morphologyEx(hu_bin_img, external_b, cv::MORPH_DILATE, elem_b, cv::Point(-1, -1), 1);
            cv::Mat external_res;
            external_res = external_a ^ external_b;

            cv_external_marker.set_image(external_res, i);
        }
    };
    std::thread th_external_0(build_external_marker, 0);
    std::thread th_external_1(build_external_marker, 1);
    std::thread th_external_2(build_external_marker, 2);
    th_external_0.join();   th_external_1.join();   th_external_2.join();
    cv_external_marker.display();

    // Watershed Marker
    cv_watershed_marker.set_property(rows, cols, total_slice, "Watershed Marker");
    auto build_watershed_marker = [&](int slice)
    {
        for (int i = slice; i < total_slice; i += 3)
        {
            cv::Mat res = cv::Mat::zeros(data_set.rows(), data_set.cols(), CV_8UC1);
            res += cv_internal_marker.get_image(i) / 255 * 255;
            res += cv_external_marker.get_image(i) / 255 * 128;
            cv_watershed_marker.set_image(res, i);
        }
    };
    std::thread th_watershed_0(build_watershed_marker, 0);
    std::thread th_watershed_1(build_watershed_marker, 1);
    std::thread th_watershed_2(build_watershed_marker, 2);
    th_watershed_0.join();  th_watershed_1.join();  th_watershed_2.join();
    cv_watershed_marker.display();

    // Lung Segmentation
    cv_lung_segment.set_property(rows, cols, total_slice, "Lung Segment");
    auto lung_segment = [&](int slice)
    {
        for (int i = slice; i < total_slice; i += 3)
        {
            cv::Mat src = cv_src.get_image(i);

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
            grad_res = grad_res * (255 / max);

            // Watershed
            cv::Mat watershed_marker(row, col, CV_32SC1); 
            cv::Mat org_marker = cv_watershed_marker.get_image(i);
            org_marker.convertTo(watershed_marker, CV_32SC1);
            cv::cvtColor(grad_res, grad_res, CV_GRAY2RGB);
            cv::watershed(grad_res, watershed_marker);

            cv::Mat watershed;
            watershed_marker.convertTo(watershed, CV_8UC1);
            cv::threshold(watershed, watershed, 128, 255, cv::THRESH_BINARY);

            cv::Mat outline;
            cv::Mat elem_out = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
            cv::morphologyEx(watershed, outline, cv::MORPH_GRADIENT, elem_out, cv::Point(-1, -1), 1);

            unsigned char arr[49] = { 0, 0, 1, 1, 1, 0, 0,
                                        0, 1, 1, 1, 1, 1, 0,
                                        1, 1, 1, 1, 1, 1, 1,
                                        1, 1, 1, 1, 1, 1, 1,
                                        1, 1, 1, 1, 1, 1, 1,
                                        0, 1, 1, 1, 1, 1, 0,
                                        0, 0, 1, 1, 1, 0, 0 };
            cv::Mat elem_black = cv::Mat(7, 7, CV_8UC1, arr);
            cv::Mat inclusion;
            cv::morphologyEx(outline, inclusion, cv::MORPH_BLACKHAT, elem_black, cv::Point(-1, -1), 1);
            outline += inclusion;

            cv::Mat lung_filter;
            cv::bitwise_or(cv_internal_marker.get_image(i), outline, lung_filter);

            cv::Mat res;
            cv::Mat elem_lung = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
            cv::morphologyEx(lung_filter, res, cv::MORPH_CLOSE, elem_lung, cv::Point(-1, -1), 4);

            cv::Mat lung;
            src.copyTo(lung, res);

            cv_lung_segment.set_image(lung, i);
        }
    };
    std::thread th_lung_0(lung_segment, 0);
    std::thread th_lung_1(lung_segment, 1);
    std::thread th_lung_2(lung_segment, 2);
    th_lung_0.join();   th_lung_1.join();   th_lung_2.join();
    cv_lung_segment.display();

}



