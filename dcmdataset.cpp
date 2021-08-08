#include "dcmdataset.h"
#include <iostream>

DcmDataSet::DcmDataSet()
{
}

DcmDataSet::~DcmDataSet()
{
    ClearAll();
}

DcmDataSet::DcmDataSet(const DcmDataSet& data_set)
{
    if (this == &data_set) return;
    
    this->set_patient_name(data_set.patient_name());
    this->set_patient_id(data_set.patient_id());
    
    this->set_study_instance_uid(data_set.study_instance_uid());
    this->set_study_description(data_set.study_description());
    
    this->set_series_instance_uid(data_set.series_instance_uid());
    this->set_series_description(data_set.series_description());
    this->set_series_number(data_set.series_number());
    
    this->set_sop_instance_uid(data_set.sop_instance_uid());
    this->set_sop_class_uid(data_set.sop_class_uid());
    
    this->set_rows(data_set.rows());
    this->set_cols(data_set.cols());
    this->set_total_instances(data_set.total_instances());
    this->set_frames_per_instance(data_set.frames_per_instance());
    
    this->set_spacing_x(data_set.spacing_x());
    this->set_spacing_y(data_set.spacing_y());
    this->set_spacing_z(data_set.spacing_z());
    this->set_slice_thickness(data_set.slice_thickness());
    this->set_spacing_between_slice(data_set.spacing_between_slice());
    
    this->set_pixel_representation(data_set.pixel_representation());
    this->set_rescale_intercept(data_set.rescale_intercept());
    this->set_rescale_slope(data_set.rescale_slope());
    this->set_bits_allocated(data_set.bits_allocated());
    this->set_bits_stored(data_set.bits_stored());
    this->set_high_bit(data_set.high_bit());
    
    std::vector<int> ww = data_set.window_width();
    for (auto& w : ww)  this->set_window_width(w);
    std::vector<int> wc = data_set.window_center();
    for (auto& w : wc)  this->set_window_center(w);
    this->set_pixel_data_window_width(data_set.pixel_data_window_width());
    this->set_pixel_data_window_center(data_set.pixel_data_window_center());
    
    const int offset = (this->bits_allocated() > 8) ? 2 : 1;
    const int size = data_set.rows() * data_set.cols() * offset * data_set.frames_per_instance();
    for (int i = 0; i < data_set.total_instances(); ++i)
    {
        uchar* res = new uchar[size];
        std::memcpy(res, data_set.get_instance_raw_data(i), size);
        this->set_instance_raw_data(res);
    }
}

DcmDataSet& DcmDataSet::operator = (const DcmDataSet &data_set)
{
    if (this == &data_set) return *this;
    
    this->set_patient_name(data_set.patient_name());
    this->set_patient_id(data_set.patient_id());
    
    this->set_study_instance_uid(data_set.study_instance_uid());
    this->set_study_description(data_set.study_description());
    
    this->set_series_instance_uid(data_set.series_instance_uid());
    this->set_series_description(data_set.series_description());
    this->set_series_number(data_set.series_number());
    
    this->set_sop_instance_uid(data_set.sop_instance_uid());
    this->set_sop_class_uid(data_set.sop_class_uid());
    
    this->set_rows(data_set.rows());
    this->set_cols(data_set.cols());
    this->set_total_instances(data_set.total_instances());
    this->set_frames_per_instance(data_set.frames_per_instance());
    
    this->set_spacing_x(data_set.spacing_x());
    this->set_spacing_y(data_set.spacing_y());
    this->set_spacing_z(data_set.spacing_z());
    this->set_slice_thickness(data_set.slice_thickness());
    this->set_spacing_between_slice(data_set.spacing_between_slice());
    
    this->set_pixel_representation(data_set.pixel_representation());
    this->set_rescale_intercept(data_set.rescale_intercept());
    this->set_rescale_slope(data_set.rescale_slope());
    this->set_bits_allocated(data_set.bits_allocated());
    this->set_bits_stored(data_set.bits_stored());
    this->set_high_bit(data_set.high_bit());
    
    std::vector<int> ww = data_set.window_width();
    for (auto& w : ww)  this->set_window_width(w);
    std::vector<int> wc = data_set.window_center();
    for (auto& w : wc)  this->set_window_center(w);
    this->set_pixel_data_window_width(data_set.pixel_data_window_width());
    this->set_pixel_data_window_center(data_set.pixel_data_window_center());
    
    const int offset = (this->bits_allocated() > 8) ? 2 : 1;
    const int size = data_set.rows() * data_set.cols() * offset * data_set.frames_per_instance();
    for (int i = 0; i < data_set.total_instances(); ++i)
    {
        uchar* res = new uchar[size];
        std::memcpy(res, data_set.get_instance_raw_data(i), size);
        this->set_instance_raw_data(res);
    }
    return *this;
}

void DcmDataSet::TransformPixelData() { ConvertRawData2PixelData(); }

void DcmDataSet::set_instance_raw_data(uchar* const ptr)
{ 
    instance_raw_data_list_.push_back(ptr);
}

uchar* DcmDataSet::get_instance_raw_data(const int& slice) const
{
    if (slice < 0 || slice >= total_instances_) return nullptr;
    return instance_raw_data_list_[slice];
}

uchar* DcmDataSet::get_frame_raw_data(const int& slice, const int& frame) const
{
    if (slice < 0 || slice >= total_instances_) return nullptr;
    if (frame < 0 || frame >= frames_per_instance_) return nullptr;
    const int offset = (bits_allocated() > 8) ? 2 : 1;
    const int size = rows() * cols() * offset;
    return instance_raw_data_list_[slice] + frame * size;
}

uchar* DcmDataSet::get_instance_pixel_data(const int& slice) const
{
    if (instance_pixel_data_list_.empty())
        const_cast<DcmDataSet*>(this)->ConvertRawData2PixelData();
    if (slice < 0 || slice >= total_instances_) return nullptr;
    return instance_pixel_data_list_[slice];
}

uchar* DcmDataSet::get_frame_pixel_data(const int& slice, const int& frame) const
{
    if (instance_pixel_data_list_.empty())
        const_cast<DcmDataSet*>(this)->ConvertRawData2PixelData();
    if (slice < 0 || slice >= total_instances_) return nullptr;
    if (frame < 0 || frame >= frames_per_instance_) return nullptr;
    const int size = rows() * cols();
    return instance_pixel_data_list_[slice] + frame * size;
}

void DcmDataSet::ConvertRawData2PixelData()
{
    if (pixel_data_window_width_ == INT_MAX || pixel_data_window_center_ == INT_MAX)
    {
        pixel_data_window_width_ = window_width()[0];
        pixel_data_window_center_ = window_center()[0];
    }

    if (!instance_pixel_data_list_.empty())
    {
        instance_pixel_data_list_.clear();
        instance_pixel_data_list_.shrink_to_fit();
    }

    const double    slope = rescale_slope();
    const double    intercept = rescale_intercept();
    const double    ww = pixel_data_window_width();
    const double    wc = pixel_data_window_center();
    const double    win_low = wc - 0.5 - (ww - 1) / 2;
    const double    win_high = wc - 0.5 + (ww - 1) / 2;
    const int       bits_allocate = bits_allocated();
    const int       total_instance = total_instances();
    const int       img_size = rows() * cols() * frames_per_instance();

    for (int i = 0; i < total_instance; ++i)
    {
        uchar* raw = instance_raw_data_list_.at(i);
        uchar* img = new uchar[img_size];
        uchar* ptr = img;

        int n = 0;
        while (n < img_size)
        {
            n++;
            short HU = 0;
            if (bits_allocate <= 8) HU = *raw++;
            else
            {
                HU = (*(raw + 1) << 8) + *raw;
                raw += 2;
            }
            HU = HU * slope + intercept;

            if (HU <= win_low)
            {
                *(ptr++) = 0;
            }
            else if (HU > win_high)
            {
                if (HU > 60000.0)   *(ptr++) = 0;
                else                *(ptr++) = 255;
            }
            else
            {
                *(ptr++) = static_cast<uchar>
                    (255 * ((HU - (wc - 0.5)) / (ww + 1) + 0.5));
            }
        }
        instance_pixel_data_list_.push_back(img);
    }

}

void DcmDataSet::ClearAll()
{
    rows_ = 0;
    cols_ = 0;
    total_instances_ = 0;
    frames_per_instance_ = 1;
    spacing_x_ = 0.0;
    spacing_y_ = 0.0;
    spacing_z_ = 0.0;
    slice_thickness_ = 0.0;
    spacing_between_slice_ = 0.0;
    pixel_representation_ = 0;
    rescale_intercept_ = 0;
    rescale_slope_ = 1;
    bits_allocated_ = 0;
    bits_stored_ = 0;
    high_bit_ = 0;

    pixel_data_window_width_ = INT_MAX;
    pixel_data_window_center_ = INT_MAX;
    window_width_.clear();
    window_width_.shrink_to_fit();
    window_center_.clear();
    window_center_.shrink_to_fit();
    
    for (auto& ptr : instance_raw_data_list_)
    {
        if (ptr != nullptr) delete[] ptr;
    }
    instance_raw_data_list_.clear();
    instance_raw_data_list_.shrink_to_fit();
    
    for (auto& ptr : instance_pixel_data_list_)
    {
        if (ptr != nullptr) delete[] ptr;
    }
    instance_pixel_data_list_.clear();
    instance_pixel_data_list_.shrink_to_fit();
}
