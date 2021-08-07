#ifndef CVTEST_H
#define CVTEST_H

#include <opencv2/opencv.hpp>

class CVTest {
public:
    CVTest() = default;
    CVTest(int row, int col, int total_slice, std::string name);
    
    void                    display();
    void                    set_property(int row, int col, int total_slice, std::string name);
    
    void                    set_image(const cv::Mat& img, const int& slice = -1);
    void                    set_image(std::vector<cv::Mat> img_list);
    cv::Mat                 get_image(const int& slice) const;
    std::vector<cv::Mat>    get_all_image() const;
    
public:
    int                     rows_ = 0;
    int                     cols_ = 0;
    int                     slice_ = 0;
    int                     total_slice_ = 0;
    
    cv::Point               start_pt_;
    cv::Point               curr_pt_;
    std::string             win_name_ = "";
    std::vector<cv::Mat>    img_list_;
};

#endif // CVTEST_H
