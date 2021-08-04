#include "imageviewer1.h"
#include "ui_imageviewer1.h"

#include <QTime>

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
				if (HU > 60000)	*(gray++) = 0;
				else			*(gray++) = 255;
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

cv::Mat ImageViewer1::ThresholdVTKImage(vtkImageData* img, int slice, int val) const
{
    const int row = img->GetDimensions()[1];
    const int col = img->GetDimensions()[0];
    unsigned char* gray = new unsigned char[row * col];
    unsigned char* ptr = gray;

    for (int i = 0; i < row; ++i)
    {
        unsigned short* data = static_cast<unsigned short*>(img->GetScalarPointer(0, i, slice));
        for (int j = 0; j < col; ++j)
        {
            short HU = *data++;
            if (HU < val)   *gray++ = 0;
            else            *gray++ = 255;
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
    int rows = 0;
    int cols = 0;
    int slice = 0;
    int total_slice = 0;
    std::vector<cv::Mat> img_list;
    std::string win_name;
};
ShowImage   src_image_;
ShowImage   internal_marker_1_;
ShowImage   internal_marker_2_;

void onMouse(int event, int x, int y, int flags, void* param)
{
    ShowImage* src = reinterpret_cast<ShowImage*>(param);
    if (x < 0 || x >= src->cols || y < 0 || y >= src->rows) return;

    if (event == cv::EVENT_MOUSEWHEEL)
    {
        if (cv::getMouseWheelDelta(flags) > 0)
        {
            src->slice = (src->slice + 1 >= src->total_slice) ? src->slice : src->slice + 1;
        }
        else
        {
            src->slice = (src->slice - 1 < 0) ? src->slice : src->slice - 1;
        }
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
    for (int i = 0; i < total_slice; ++i)
    {
        cv::Mat src = this->ConvertVTKImageToUCharCVMat(viewer_map_[ViewName::TRA]->image_data(), i);
        src_image_.img_list.push_back(src);
    }
    src_image_.rows = data_set.rows();
    src_image_.cols = data_set.cols();
    src_image_.total_slice = total_slice;
    src_image_.win_name = "Src";
    cv::namedWindow(src_image_.win_name);
    cv::imshow(src_image_.win_name, src_image_.img_list.at(0));
    cv::setMouseCallback(src_image_.win_name, onMouse, &src_image_);

    // Internal Marker - Threshold (-400)
    int slice = 0;
    internal_marker_1_ = src_image_;
    for (auto& marker : internal_marker_1_.img_list)
    {
        cv::Mat hu_bin_img = this->ThresholdVTKImage(viewer_map_[ViewName::TRA]->image_data(), slice++, -400);
        marker = hu_bin_img;
    }
    internal_marker_1_.win_name = "Internal Marker - 1";
    cv::namedWindow(internal_marker_1_.win_name);
    cv::imshow(internal_marker_1_.win_name, internal_marker_1_.img_list.at(0));
    cv::setMouseCallback(internal_marker_1_.win_name, onMouse, &internal_marker_1_);

    // Internal Marker - Threshold (Remove areas that attached image border)
    slice = 0;
    internal_marker_2_ = src_image_;
    for (auto& marker : internal_marker_2_.img_list)
    {
        cv::Mat hu_bin_img = this->ThresholdVTKImage(viewer_map_[ViewName::TRA]->image_data(), slice++, -400);
        
        cv::Mat label_img;
        cv::Mat stats;
        cv::Mat centroids;
        cv::connectedComponentsWithStats(hu_bin_img, label_img, stats, centroids);

        int row = hu_bin_img.rows;
        int col = hu_bin_img.cols;
        for (int i = 0; i < row; ++i)
        {
            for (int j = 0; j < col; ++j)
            {
                int label = label_img.at<int>(i, j);
                if (label == 0) continue;
                if (stats.at<int>(label, cv::CC_STAT_LEFT) == 0 ||
                    stats.at<int>(label, cv::CC_STAT_TOP) == 0)
                    hu_bin_img.at<uchar>(i, j) = 0;
            }
        }

       marker = hu_bin_img;
    }
    internal_marker_2_.win_name = "Internal Marker - 2";
    cv::namedWindow(internal_marker_2_.win_name);
    cv::imshow(internal_marker_2_.win_name, internal_marker_2_.img_list.at(0));
    cv::setMouseCallback(internal_marker_2_.win_name, onMouse, &internal_marker_2_);

}



