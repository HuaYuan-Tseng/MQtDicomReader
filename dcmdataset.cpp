#include "dcmdataset.h"

DcmDataSet::DcmDataSet()
{
}

DcmDataSet::DcmDataSet(DcmDataSet* data_set)
{
    this->patient_name_ = data_set->patient_name();
    this->patient_id_ = data_set->patient_id();
    this->study_instance_uid() = data_set->study_instance_uid();
}

DcmDataSet::~DcmDataSet()
{
    ClearAll();
}

void DcmDataSet::TransformPixelData() { ConvertRawData2PixelData(); }

void DcmDataSet::set_instance_raw_data(uchar* raw) { instance_raw_data_list_.push_back(raw); }

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
    return instance_raw_data_list_[slice] + frame * rows_ * cols_ * offset;
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
    return instance_pixel_data_list_[slice] + frame * rows_ * cols_;
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
        for (auto ptr : instance_pixel_data_list_)
            delete[] ptr;
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
                *(img++) = 0;
            }
            else if (HU > win_high)
            {
                if (HU > 60000.0)   *(img++) = 0;
                else                *(img++) = 255;
            }
            else
            {
                *(img++) = static_cast<uchar>
                    (255 * ((HU - (wc - 0.5)) / (ww + 1) + 0.5));
            }
        }
        instance_pixel_data_list_.push_back(ptr);
    }

}

void DcmDataSet::ClearAll()
{
    rows_ = 1;
    cols_ = 1;
    total_instances_ = 1;
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

    for (auto& ptr : instance_raw_data_list_) delete[] ptr;
    instance_raw_data_list_.clear();
    instance_raw_data_list_.shrink_to_fit();

    for (auto& ptr : instance_pixel_data_list_) delete[] ptr;
    instance_pixel_data_list_.clear();
    instance_pixel_data_list_.shrink_to_fit();
}
