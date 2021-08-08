#include "cvtest.h"

void onMouse(int event, int x, int y, int flags, void* param)
{
    CVTest* src = reinterpret_cast<CVTest*>(param);
    if (x < 0 || x >= src->cols_ || y < 0 || y >= src->rows_) return;

    if (event == cv::EVENT_MOUSEWHEEL)
    {
        // Move Slice
        if (cv::getMouseWheelDelta(flags) < 0)
        {
            src->slice_ = (src->slice_ + 1 >= src->total_slice_) ? src->slice_ : src->slice_ + 1;
        }
        else
        {
            src->slice_ = (src->slice_ - 1 < 0) ? src->slice_ : src->slice_ - 1;
        }
    }
    else if (event == cv::EVENT_MOUSEMOVE)
    {
        // Drag Slice
        if ((flags & cv::EVENT_FLAG_SHIFTKEY) && (flags & cv::EVENT_FLAG_LBUTTON))
        {
            src->curr_pt_ = cv::Point(x, y);
            int new_slice = src->slice_ - (src->curr_pt_.y - src->start_pt_.y);
            if (new_slice < 0) new_slice = 0;
            else if (new_slice >= src->total_slice_) new_slice = src->total_slice_ - 1;
            src->slice_ = new_slice;
            src->start_pt_ = src->curr_pt_;
        }
        // Draw ROI
        else if (src->is_drawing_ && (flags & cv::EVENT_FLAG_CTRLKEY) && (flags & cv::EVENT_FLAG_LBUTTON))
        {
            std::cout << "Drawing" << std::endl;
            src->curr_pt_ = cv::Point(x, y);
            src->roi_ = cv::Rect(src->start_pt_, src->curr_pt_);
            cv::Mat org = src->temp_display_.clone();
            cv::rectangle(org, src->roi_.tl(), src->roi_.br(), cv::Scalar(0, 255, 0), 1);
            src->img_list_.at(src->slice_) = org;
        }
    }
    else if (event == cv::EVENT_LBUTTONDOWN)
    {
        // Drag Slice
        if (flags & cv::EVENT_FLAG_SHIFTKEY)
        {
            src->start_pt_ = cv::Point(x, y);
        }
        // Draw ROI
        else if (flags & cv::EVENT_FLAG_CTRLKEY)
        {
            src->start_pt_ = cv::Point(x, y);
            src->is_drawing_ = true;
            src->temp_display_ = src->img_list_.at(src->slice_);
        }
    }
    else if (event == cv::EVENT_LBUTTONUP)
    {
        // Draw ROI
        if (flags & cv::EVENT_FLAG_ALTKEY)
        {
            src->is_drawing_ = false;
        }
    }
    cv::Scalar color;
    uchar pixel = src->get_image(0).at<uchar>(0, 0);
    if (pixel <= 128) color = cv::Scalar(255, 255, 255);
    else color = cv::Scalar(0, 0, 0);
    
    cv::putText(src->img_list_.at(src->slice_), std::to_string(src->slice_),
        cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.35, color);
    cv::imshow(src->win_name_, src->img_list_.at(src->slice_));
}

CVTest::CVTest(int row, int col, int total_slice, std::string name) :
    rows_(row),
    cols_(col),
    total_slice_(total_slice),
    win_name_(name)
{
}

void CVTest::set_property(int row, int col, int total_slice, std::string name)
{
    this->rows_ = row;
    this->cols_ = col;
    this->total_slice_ = total_slice;
    this->win_name_ = name;
    this->img_list_.resize(total_slice_);
}

void CVTest::display()
{
    if (this->img_list_.empty()) return;
    cv::namedWindow(this->win_name_);
    cv::imshow(this->win_name_, this->img_list_.at(0));
    cv::setMouseCallback(this->win_name_, onMouse, this);
}

void CVTest::set_image(const cv::Mat& img, const int& slice)
{
    if (slice == -1)    img_list_.push_back(img);
    else                img_list_[slice] = img;
}

void CVTest::set_image(std::vector<cv::Mat> img_list)
{
    this->img_list_ = img_list;
}

cv::Mat CVTest::get_image(const int& slice) const
{
    return img_list_.at(slice).clone();
}

std::vector<cv::Mat> CVTest::get_all_image() const
{
    return img_list_;
}
