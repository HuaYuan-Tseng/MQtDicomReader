#include "imageviewer1.h"
#include "ui_imageviewer1.h"

#include <QTime>

#include <thread>

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

struct ShowImage {
    int                     rows = 0;
    int                     cols = 0;
    int                     slice = 0;
    int                     total_slice = 0;
    std::vector<cv::Mat>    img_list;
    std::string             win_name;
    cv::Point               start_pt;
    cv::Point               curr_pt;
};
ShowImage   src_image_;
ShowImage   internal_marker_1_;
ShowImage   internal_marker_2_;
ShowImage   external_marker_1_;
ShowImage   watershed_marker_;
ShowImage   lung_segment_;

void onMouse(int event, int x, int y, int flags, void* param)
{
    ShowImage* src = reinterpret_cast<ShowImage*>(param);
    if (x < 0 || x >= src->cols || y < 0 || y >= src->rows) return;

    if (event == cv::EVENT_MOUSEWHEEL)
    {
        // Move Slice
        if (cv::getMouseWheelDelta(flags) < 0)
        {
            src->slice = (src->slice + 1 >= src->total_slice) ? src->slice : src->slice + 1;
        }
        else
        {
            src->slice = (src->slice - 1 < 0) ? src->slice : src->slice - 1;
        }
    }
    else if (event == cv::EVENT_MOUSEMOVE)
    {
        // Drag Slice
        if ((flags & cv::EVENT_FLAG_CTRLKEY) && (flags & cv::EVENT_FLAG_LBUTTON))
        {
            src->curr_pt = cv::Point(x, y);
            int new_slice = src->slice - (src->curr_pt.y - src->start_pt.y);
            if (new_slice < 0) new_slice = 0;
            else if (new_slice >= src->total_slice) new_slice = src->total_slice - 1;
            src->slice = new_slice;
            src->start_pt = src->curr_pt;
        }
    }
    else if (event == cv::EVENT_LBUTTONDOWN)
    {
        // Drag Slice
        if (flags & cv::EVENT_FLAG_CTRLKEY)
        {
            src->start_pt = cv::Point(x, y);
        }
    }
    else if (event == cv::EVENT_LBUTTONUP)
    {
    }
    cv::putText(src->img_list.at(src->slice), std::to_string(src->slice),
        cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.35, cv::Scalar(255, 255, 255));
    cv::imshow(src->win_name, src->img_list.at(src->slice));
}

void ImageViewer1::ToProcess()
{
    if (viewer_map_.empty()) return;
    auto& data_set = global_state_->study_browser_.dcm_data_set_;
    int total_slice = data_set.total_instances();

    // Src Image
    int slice = 0;
    for (int i = 0; i < total_slice; ++i)
    {
        cv::Mat src = this->ConvertVTKImageToUCharCVMat(viewer_map_[ViewName::TRA]->image_data(), slice++);
        src_image_.img_list.push_back(src);
    }
    src_image_.rows = data_set.rows();
    src_image_.cols = data_set.cols();
    src_image_.total_slice = total_slice;
    src_image_.win_name = "Src";
    cv::namedWindow(src_image_.win_name);
    cv::imshow(src_image_.win_name, src_image_.img_list.at(0));
    cv::setMouseCallback(src_image_.win_name, onMouse, &src_image_);

    // Internal Marker
    slice = 0;
    internal_marker_2_ = src_image_;
    auto internal_marker = [&](int slice)
    {
        for (; slice < total_slice; slice += 3)
        {
            cv::Mat hu_bin_img = this->ThresholdVTKImage(viewer_map_[ViewName::TRA]->image_data(), slice, -400, true);

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

            internal_marker_2_.img_list.at(slice) = hu_bin_img;
        }
    };
    
    std::thread t4(internal_marker, 0);
    std::thread t5(internal_marker, 1);
    std::thread t6(internal_marker, 2);
    t4.join();
    t5.join();
    t6.join();
    internal_marker_2_.win_name = "Internal Marker - 2";
    cv::namedWindow(internal_marker_2_.win_name);
    cv::imshow(internal_marker_2_.win_name, internal_marker_2_.img_list.at(0));
    cv::setMouseCallback(internal_marker_2_.win_name, onMouse, &internal_marker_2_);

    // External Marker
    slice = 0;
    external_marker_1_ = src_image_;
    auto external_marker = [&](int slice) 
    {
        for (; slice < total_slice; slice += 3)
        {
            cv::Mat hu_bin_img = this->ThresholdVTKImage(viewer_map_[ViewName::TRA]->image_data(), slice, -400, true);
            
            //cv::Mat elem = internal_marker_2_.img_list.at(slice);
            cv::Mat elem_a = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
            cv::Mat elem_b = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(49, 49));
            cv::Mat external_a, external_b;
            cv::morphologyEx(hu_bin_img, external_a, cv::MORPH_DILATE, elem_a, cv::Point(-1, -1), 1);
            cv::morphologyEx(hu_bin_img, external_b, cv::MORPH_DILATE, elem_b, cv::Point(-1, -1), 1);
            cv::Mat external_res;
            external_res = external_a ^ external_b;
            //external_res = external_b - external_a;

            external_marker_1_.img_list.at(slice) = external_res;
        }
    };
    std::thread t1(external_marker, 0);
    std::thread t2(external_marker, 1);
    std::thread t3(external_marker, 2);
    t1.join();
    t2.join();
    t3.join();
    external_marker_1_.win_name = "External Marker - 1";
    cv::namedWindow(external_marker_1_.win_name);
    cv::imshow(external_marker_1_.win_name, external_marker_1_.img_list.at(0));
    cv::setMouseCallback(external_marker_1_.win_name, onMouse, &external_marker_1_);

    // Watershed Marker
    watershed_marker_ = src_image_;
    auto watershed_marker = [&](int slice)
    {
        for (; slice < total_slice; slice += 3)
        {
            cv::Mat res = cv::Mat::zeros(data_set.rows(), data_set.cols(), CV_8UC1);
            res += internal_marker_2_.img_list.at(slice) / 255 * 255;
            res += external_marker_1_.img_list.at(slice) / 255 * 128;
            watershed_marker_.img_list.at(slice) = res;
        }
    };
    std::thread t7(watershed_marker, 0);
    std::thread t8(watershed_marker, 1);
    std::thread t9(watershed_marker, 2);
    t7.join();
    t8.join();
    t9.join();
    watershed_marker_.win_name = "Watershed Marker";
    cv::namedWindow(watershed_marker_.win_name);
    cv::imshow(watershed_marker_.win_name, watershed_marker_.img_list.at(0));
    cv::setMouseCallback(watershed_marker_.win_name, onMouse, &watershed_marker_);

    // Lung Segmentation
    lung_segment_ = src_image_;
    auto lung_segment = [&](int slice)
    {
        for (; slice < total_slice; slice += 3)
        {
            cv::Mat src = src_image_.img_list.at(slice);

            // Sobel
            cv::Mat grad_x, grad_y;
            cv::Sobel(src, grad_x, CV_8UC1, 1, 0, 3);
            cv::Sobel(src, grad_y, CV_8UC1, 0, 1, 3);
            cv::convertScaleAbs(grad_x, grad_x);
            cv::convertScaleAbs(grad_y, grad_y);

            int max = 0;
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
            cv::Mat org_marker = watershed_marker_.img_list.at(slice);
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
            cv::bitwise_or(internal_marker_2_.img_list.at(slice), outline, lung_filter);

            cv::Mat res;
            cv::Mat elem_lung = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
            cv::morphologyEx(lung_filter, res, cv::MORPH_CLOSE, elem_lung, cv::Point(-1, -1), 4);

            cv::Mat lung;
            src.copyTo(lung, res);

            lung_segment_.img_list.at(slice) = lung;
        }
    };
    std::thread t10(lung_segment, 0);
    std::thread t11(lung_segment, 1);
    std::thread t12(lung_segment, 2);
    t10.join();
    t11.join();
    t12.join();
    lung_segment_.win_name = "Lung Segment";
    cv::namedWindow(lung_segment_.win_name);
    cv::imshow(lung_segment_.win_name, lung_segment_.img_list.at(0));
    cv::setMouseCallback(lung_segment_.win_name, onMouse, &lung_segment_);

}



