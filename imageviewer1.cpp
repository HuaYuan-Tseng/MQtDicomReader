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
    
    QObject::connect(ui_->imageButton_Process, SIGNAL(clicked()), this, SLOT(ToProcess()));
    QObject::connect(ui_->imageButton_LungSegment, SIGNAL(clicked()), this, SLOT(ToLungSegment()));
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
}

void ImageViewer1::SetupViewers()
{
    InitViewer(ViewName::TRA, ui_->vtk_viewer_0);
}

void ImageViewer1::InitViewer(ViewName view_name, QVTKOpenGLWidget* widget)
{
    auto it = viewer_map_.find(view_name);
    if (it != viewer_map_.end())
    {
        delete it->second;
        viewer_map_.erase(view_name);
    }
    
    Viewer* viewer = new Viewer(view_name, widget, global_state_);
    global_state_->study_browser_.dcm_data_set_.set_pixel_data_window_width(1500);
    global_state_->study_browser_.dcm_data_set_.set_pixel_data_window_center(-400);
    viewer->Init(global_state_->study_browser_.dcm_data_set_);
    viewer->image_interactor()->AddEvent(Event::MOVE_SLICE_PLUS, [&]{ MoveSlicePlus(); });
    viewer->image_interactor()->AddEvent(Event::MOVE_SLICE_MINUS, [&]{ MoveSliceMinus(); });
    viewer->image_interactor()->AddEvent(Event::DRAG_SLICE, [&] { DragSlice(); });
    viewer->image_interactor()->AddEvent(Event::ZOOM_IN, [&]{ ZoomIn(); });
    viewer->image_interactor()->AddEvent(Event::ZOOM_OUT, [&]{ ZoomOut(); });
    viewer->image_interactor()->AddEvent(Event::DRAW_ROI, [&] { DrawROI(); });
    viewer->image_interactor()->AddEvent(Event::ADD_NODULE, [&]{ AddNodule(); });
    viewer_map_[view_name] = viewer;
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
    viewer_map_[global_state_->image_viewer_1_.current_control_view_]->DrawROI();
}

void ImageViewer1::AddNodule()
{
    // Take the ROI just drawed
    ViewName control_view = global_state_->image_viewer_1_.current_control_view_;
    double* spacing = const_cast<double*>(viewer_map_[control_view]->spacing());
    int* dimension = const_cast<int*>(viewer_map_[control_view]->dimension());
    std::cout << "Spacing : " << spacing[0] << " , " << spacing[1] << " , " << spacing[2] << std::endl;
    std::cout << "Dimension : " << dimension[0] << " , " << dimension[1] << " , " << dimension[2] << std::endl;
    
    ROI* roi = viewer_map_[control_view]->roi();
    if (roi == nullptr) std::cout << "ROI is nullptr" << std::endl;
    else std::cout << "Get roi" << std::endl;
    
    // Create Nodule
    std::vector<double> vec_spacing{spacing[0], spacing[1], spacing[2]};
    std::vector<int> vec_dimension{dimension[0], dimension[1], dimension[2]};
    Nodule* nodule = new Nodule(control_view, vec_spacing, vec_dimension);
    nodule->set_roi(*roi);
    std::cout << "Init nodule." << std::endl;

    // Segment Nodule
    this->SegmentNodule(nodule);
    nodule_list_.push_back(nodule);
}

void ImageViewer1::SegmentNodule(Nodule* nodule)
{
    Viewer* current_viewer = viewer_map_[global_state_->image_viewer_1_.current_control_view_];
    int curr_slice = nodule->label_init_slice();
 
    struct Condition {
        int         current_slice = 0;
        double      pre_seed_pixel = 0.0;
        cv::Point   current_seed = cv::Point(-1, -1);
    }   condition;

    std::queue<Condition> candidate;
    condition.current_slice = curr_slice;
    condition.current_seed = cv::Point(-1, -1);
    condition.pre_seed_pixel = -1;
    candidate.push(condition);

    std::cout << "\nStart segment." << std::endl;

    while (!candidate.empty())
    {
        condition = candidate.front();
        candidate.pop();

        if (nodule->IsSliceHaveContour(condition.current_slice)) continue;

        // Extract current slice image data
        cv::Mat src = ConvertVTKImageToUCharCVMat(current_viewer->image_data(), condition.current_slice);

        // Segmentation
        cv::Point next_seed = SliceSegment(nodule, src, condition.current_seed, condition.pre_seed_pixel, condition.current_slice);

        if (next_seed == cv::Point(-1, -1)) continue;
        candidate.push({ condition.current_slice - 1, condition.pre_seed_pixel, next_seed });
        candidate.push({ condition.current_slice + 1, condition.pre_seed_pixel, next_seed });
    }

    int start_slice = nodule->contour_start_slice();
    int end_slice = nodule->contour_end_slice();

    std::cout << "Finish segment." << std::endl;
    std::cout << "Start slice : " << start_slice << std::endl;
    std::cout << "End slice   : " << end_slice << std::endl;

    // put vtk's actor to image_viewer_
    vtkSmartPointer<vtkActor> roi;
    std::vector<vtkSmartPointer<vtkActor>> contours;
    for (int i = start_slice; i <= end_slice; ++i)
    {
        if (nodule->get_vtk_contour_actor(i, contours))
        {
            std::cout << "Slice " << i << " : have contour." << std::endl;
            for (const auto c : contours)
            {
                current_viewer->image_viewer()->GetRenderer()->AddViewProp(c);
            }
        }
        if (nodule->get_vtk_roi_actor(i, roi))
        {
            std::cout << "Slice " << i << " : have roi." << std::endl;
            current_viewer->image_viewer()->GetRenderer()->AddViewProp(roi);
            if (roi != nullptr)
                std::cout << "ROI actor z pos : " << roi->GetPosition()[2] << std::endl;
            else
                std::cout << "ROI is nullptr." << std::endl;
        }
    }
    current_viewer->RefreshViewer();
}

cv::Point ImageViewer1::SliceSegment(Nodule* nod, cv::Mat& src, cv::Point& seed, double& pre_seed_pixel, int slice)
{
    // Lung Segmentation
    cv::Mat lung = LungMarker(slice);

    // Extract ROI
    cv::Rect roi_rect = cv::Rect(cv::Point(nod->roi_tl()[0], nod->roi_tl()[1]),
                                    cv::Point(nod->roi_br()[0], nod->roi_br()[1]));
    cv::Mat roi = lung(roi_rect);

    // Detect Best Seed
    if (seed == cv::Point(-1, -1))
    {
        seed = DetectBestSeed(roi);
        if (seed == cv::Point(-1, -1))
            return seed;
        else
            seed += roi_rect.tl();
        pre_seed_pixel = src.at<uchar>(seed);
    }
    if (seed.x < 0 || seed.x >= src.cols || seed.y < 0 || seed.y >= src.rows) return cv::Point(-1, -1);
    if (std::abs(src.at<uchar>(seed) - pre_seed_pixel) > 50) return cv::Point(-1, -1);

    // Region Growing
    double otsu = 0;
    cv::Mat growing;
    cv::Mat roi_bin;
    otsu = cv::threshold(roi, roi_bin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    //cv::medianBlur(roi, roi, 3);
    //cv::adaptiveThreshold(roi, roi_bin, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 3, 0);
    SaveOpenCVImage("ROI-bin", roi_bin);
    if (src.at<uchar>(seed) < otsu || roi_bin.at<uchar>(seed - roi_rect.tl()) == 0) return cv::Point(-1, -1);
    cv::Point seed_on_roi = seed - roi_rect.tl();
    growing = RegionGrowing(roi_bin, seed_on_roi, 255, 10);
    pre_seed_pixel = src.at<uchar>(seed);

    cv::Mat res;
    cv::Mat res_elem = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::morphologyEx(growing, res, cv::MORPH_DILATE, res_elem, cv::Point(-1, -1), 1);

    // Extract contour
    std::vector<std::vector<cv::Point>> contour;
    cv::findContours(res, contour, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, cv::Point(roi_rect.x, roi_rect.y));

    // Sort contour (bit to small)
    using Contour = std::vector<cv::Point>;
    std::sort(contour.begin(), contour.end(), [](Contour& c1, Contour& c2) {
        return c1.size() > c2.size();
        });

    // Remove contour that too big or too small
    int roi_area = roi_rect.height * roi_rect.width;
    auto it = contour.begin();
    while (it != contour.end())
    {
        if (it->size() < 3 || cv::contourArea(*it) >= roi_area * 0.9)
            it = contour.erase(it);
        else
            ++it;
    }

    // Set contour
    if (contour.empty()) return cv::Point(-1, -1);
    nod->set_contour(slice, contour);

    // Calcualte centroid to become next slice's seed.
    cv::Moments mom = cv::moments(contour[0]);
    cv::Point centroid = cv::Point((mom.m10 / mom.m00), (mom.m01 / mom.m00));

    return centroid;
}

cv::Mat ImageViewer1::LungMarker(int slice)
{
    // Threshold HU Image
    Viewer* current_viewer = viewer_map_[global_state_->image_viewer_1_.current_control_view_];
    cv::Mat src = this->ConvertVTKImageToUCharCVMat(current_viewer->image_data(), slice);
    cv::Mat hu_bin = this->ThresholdVTKImage(current_viewer->image_data(), slice, -400, true);

    // Morphology
    cv::Mat lung_close, outline, blackhat;
    cv::Mat elem = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::morphologyEx(hu_bin, lung_close, cv::MORPH_CLOSE, elem, cv::Point(-1, -1), 1);
    cv::morphologyEx(lung_close, outline, cv::MORPH_GRADIENT, elem, cv::Point(-1, -1), 1);

    cv::Mat black = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(9, 9));
    cv::morphologyEx(outline, blackhat, cv::MORPH_BLACKHAT, black, cv::Point(-1, -1), 2);
    outline += blackhat;

    cv::Mat mask;
    cv::bitwise_or(lung_close, outline, mask);

    // Capture Lung Area
    cv::Mat lung;
    src.copyTo(lung, mask);
    return lung;
}

cv::Point ImageViewer1::DetectBestSeed(cv::Mat& roi)
{
    // Threshold
    cv::Mat roi_bin;
    cv::threshold(roi, roi_bin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // Find Components
    cv::Mat label, stats, centroids;
    int label_cnt = 0;
    label_cnt = cv::connectedComponentsWithStats(roi_bin, label, stats, centroids);
    if (label_cnt == 1) return cv::Point(-1, -1);

    // Find the centroids that more close to roi's center.
    int roi_cent_x = roi_bin.cols / 2;
    int roi_cent_y = roi_bin.rows / 2;
    double limit = std::sqrt(std::pow(roi_cent_x, 2) + std::pow(roi_cent_y, 2)) / 2;
    std::vector<std::pair<int, double>> center_dist;
    for (int i = 1; i < label_cnt; ++i)
    {
        int x_range = centroids.at<double>(i, 0) - roi_cent_x;
        int y_range = centroids.at<double>(i, 1) - roi_cent_y;
        double range = std::sqrt(std::pow(x_range, 2) + std::pow(y_range, 2));
        if (range > limit) continue;
        center_dist.push_back(std::make_pair(i, range));
    }
    std::sort(center_dist.begin(), center_dist.end(), [](std::pair<int, double>& a, std::pair<int, double>& b) {
        return a.second < b.second;
        });
    if (center_dist.empty()) return cv::Point(-1, -1);

    // Select Seed
    cv::Point select_seed;
    if (center_dist.size() == 1)
    {
        select_seed.x = centroids.at<double>(center_dist.at(0).first, 0);
        select_seed.y = centroids.at<double>(center_dist.at(0).first, 1);
    }
    else
    {
        cv::Point roi_center(roi_cent_x, roi_cent_y);
        int center_label = label.at<int>(roi_center.x, roi_center.y);
        if (center_label != 0)
        {
            select_seed.x = centroids.at<double>(center_label, 0);
            select_seed.y = centroids.at<double>(center_label, 1);
        }
        else
        {
            select_seed.x = centroids.at<double>(center_dist.at(0).first, 0);
            select_seed.y = centroids.at<double>(center_dist.at(0).first, 1);
        }
    }

    return select_seed;
}

cv::Mat ImageViewer1::RegionGrowing(cv::Mat& src, cv::Point& seed, const int& new_val, const int& th)
{
    cv::Mat res = cv::Mat::zeros(src.size(), CV_8UC1);
    cv::Point growing_pt;
    int growable = 0;
    int src_value = 0;
    int cur_value = 0;

    int DIR[8][2] = { {-1,-1}, {0,-1}, {1,-1}, {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0} };
    std::queue<cv::Point> candidate;
    candidate.push(seed);
    res.at<uchar>(seed) = new_val;

    while (!candidate.empty())
    {
        seed = candidate.front();
        candidate.pop();

        src_value = src.at<uchar>(seed);
        for (int i = 0; i < 8; ++i)
        {
            growing_pt.x = seed.x + DIR[i][0];
            growing_pt.y = seed.y + DIR[i][1];

            if (growing_pt.x <= 0 || growing_pt.y <= 0 ||
                growing_pt.x > (src.cols - 1) || growing_pt.y > (src.rows - 1))
                continue;
            growable = res.at<uchar>(growing_pt);
            if (growable == 0)
            {
                cur_value = src.at<uchar>(growing_pt);
                if (std::abs(src_value - cur_value) <= th)
                {
                    res.at<uchar>(growing_pt) = new_val;
                    candidate.push(growing_pt);
                }
            }
        }
    }
    return res.clone();
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


