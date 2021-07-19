#include "dcmdataset.h"

DcmDataSet::DcmDataSet()
{
}

void DcmDataSet::set_patient_name(const QString& name) { patient_name_ = name; }

QString DcmDataSet::patient_name() const { return patient_name_; }

void DcmDataSet::set_patient_id(const QString& id) {patient_id_ = id; }

QString DcmDataSet::patient_id() const { return patient_id_; }

void DcmDataSet::set_study_instance_uid(const QString& uid) { study_instance_uid_ = uid; }

QString DcmDataSet::study_instance_uid() const { return study_instance_uid_; }

void DcmDataSet::set_study_description(const QString& description) { study_description_ = description; }

QString DcmDataSet::study_description() const { return study_description_; }

void DcmDataSet::set_series_instance_uid(const QString& uid) { series_instance_uid_ = uid; }

QString DcmDataSet::series_instance_uid() const { return series_instance_uid_; }

void DcmDataSet::set_series_description(const QString& description) { series_description_ = description; }

QString DcmDataSet::series_description() const { return series_description_; }

void DcmDataSet::set_series_number(const int& num) { series_number_ = num; }

int DcmDataSet::series_number() const { return series_number_; }

void DcmDataSet::set_sop_instance_uid(const QString& uid) { sop_instance_uid_ = uid; }

QString DcmDataSet::sop_instance_uid() const { return sop_instance_uid_; }

void DcmDataSet::set_sop_class_uid(const QString& uid) { sop_class_uid_ = uid; }

QString DcmDataSet::sop_class_uid() const { return sop_class_uid_; }

void DcmDataSet::set_rows(const int& rows) { rows_ = rows; }

int DcmDataSet::rows() const { return rows_; }

void DcmDataSet::set_cols(const int& cols) { cols_ = cols; }

int DcmDataSet::cols() const { return cols_; }

void DcmDataSet::set_total_instances(const int& instances) { total_instances_ = instances; }

int DcmDataSet::total_instances() const { return total_instances_; }

void DcmDataSet::set_frames_per_instance(const int& frames) { frames_per_instance_ = frames; }

int DcmDataSet::frames_per_instance() const { return frames_per_instance_; }

void DcmDataSet::set_spacing_x(const double& x) { spacing_x_ = x; }

double DcmDataSet::spacing_x() const { return spacing_x_; }

void DcmDataSet::set_spacing_y(const double& y) { spacing_y_ = y; }

double DcmDataSet::spacing_y() const { return spacing_y_; }

void DcmDataSet::set_spacing_z(const double& z) { spacing_z_ = z; }

double DcmDataSet::spacing_z() const { return spacing_z_; }

void DcmDataSet::set_slice_thickness(const double& thickness) { slice_thickness_ = thickness; }

double DcmDataSet::slice_thickness() const { return slice_thickness_; }

void DcmDataSet::set_spacing_between_slice(const double& spacing) { spacing_between_slice_ = spacing; }

double DcmDataSet::spacing_between_slice() const { return spacing_between_slice_; }

void DcmDataSet::set_pixel_representation(const int& representation) { pixel_representation_ = representation; }

int DcmDataSet::pixel_representation() const { return pixel_representation_; }

void DcmDataSet::set_rescale_intercept(const int& intercept) { rescale_intercept_ = intercept; }

int DcmDataSet::rescale_intercept() const { return rescale_intercept_; }

void DcmDataSet::set_rescale_slope(const int& slope) { rescale_slope_ = slope; }

int DcmDataSet::rescale_slope() const { return rescale_slope_; }

void DcmDataSet::set_bits_allocated(const int& bits) { bits_allocated_ = bits; }

int DcmDataSet::bits_allocated() const { return bits_allocated_; }

void DcmDataSet::set_bits_stored(const int& bits) { bits_stored_ = bits; }

int DcmDataSet::bits_stored() const { return bits_stored_; }

void DcmDataSet::set_high_bit(const int& bit) { high_bit_ = bit; }

int DcmDataSet::high_bit() const { return high_bit_; }

void DcmDataSet::set_window_width(const int& ww) { window_width_.push_back(ww); }

const std::vector<int>& DcmDataSet::window_width() const { return window_width_; }

void DcmDataSet::set_window_center(const int& wc) { window_center_.push_back(wc); }

const std::vector<int>& DcmDataSet::window_center() const { return window_center_; }

void DcmDataSet::set_pixel_data_window_width(const int& ww) { pixel_data_window_width_ = ww; }

int DcmDataSet::pixel_data_window_width() const { return pixel_data_window_width_; }

void DcmDataSet::set_pixel_data_window_center(const int& wc) { pixel_data_window_center_ = wc; }

int DcmDataSet::pixel_data_window_center() const { return pixel_data_window_center_; }

void DcmDataSet::TransformPixelData() { ConvertRawData2PixelData(); }

void DcmDataSet::set_instance_raw_data(short* raw) { instance_raw_data_list_.push_back(raw); }

short* DcmDataSet::get_instance_raw_data(const int& slice) const
{
    if (slice < 0 || slice >= total_instances_) return nullptr;
    return instance_raw_data_list_[slice];
}

short* DcmDataSet::get_frame_raw_data(const int& slice, const int& frame) const
{
    if (slice < 0 || slice >= total_instances_) return nullptr;
    if (frame < 0 || frame >= frames_per_instance_) return nullptr;
    return instance_raw_data_list_[slice] + frame * rows_ * cols_;
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
    const int       total_instance = total_instances();
    const int       img_size = rows() * cols() * frames_per_instance();

    for (int i = 0; i < total_instance; ++i)
    {
        short* raw = instance_raw_data_list_.at(i);
        uchar* img = new uchar[img_size];
        uchar* ptr = img;

        int n = 0;
        while (n < img_size)
        {
            n++;
            short HU = *raw++;
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
