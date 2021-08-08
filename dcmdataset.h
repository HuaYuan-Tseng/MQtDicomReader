#ifndef DCMDATASET_H
#define DCMDATASET_H

#include <QString>
#include <vector>

class DcmDataSet {
public:
    DcmDataSet();
    ~DcmDataSet();
    DcmDataSet(const DcmDataSet& data_set);
    
    DcmDataSet& operator = (const DcmDataSet& data_set);
    
    QString name_ = "";
    void setname(QString name) { name_ = name; }

    void                        ClearAll();

    void                        set_patient_name(const QString& name) { patient_name_ = name; }
    QString                     patient_name() const { return patient_name_; }

    void                        set_patient_id(const QString& id) { patient_id_ = id; }
    QString                     patient_id() const { return patient_id_; }

    void                        set_study_instance_uid(const QString& uid) { study_instance_uid_ = uid; }
    QString                     study_instance_uid() const { return study_instance_uid_; }

    void                        set_study_description(const QString& description) { study_description_ = description; }
    QString                     study_description() const { return study_description_; }

    void                        set_series_instance_uid(const QString& uid) { series_instance_uid_ = uid; }
    QString                     series_instance_uid() const { return series_instance_uid_; }

    void                        set_series_description(const QString& description) { series_description_ = description; }
    QString                     series_description() const { return series_description_; }

    void                        set_series_number(const int& num) { series_number_ = num; }
    int                         series_number() const { return series_number_; }

    void                        set_sop_instance_uid(const QString& uid) { sop_instance_uid_ = uid; }
    QString                     sop_instance_uid() const { return sop_instance_uid_; }

    void                        set_sop_class_uid(const QString& uid) { sop_class_uid_ = uid; }
    QString                     sop_class_uid() const { return sop_class_uid_; }

    void                        set_rows(const int& rows) { rows_ = rows; }
    int                         rows() const { return rows_; }

    void                        set_cols(const int& cols) { cols_ = cols; }
    int                         cols() const { return cols_; }

    void                        set_total_instances(const int& instances) { total_instances_ = instances; }
    int                         total_instances() const { return total_instances_; }

    void                        set_frames_per_instance(const int& frames) { frames_per_instance_ = frames; }
    int                         frames_per_instance() const { return frames_per_instance_; }

    void                        set_spacing_x(const double& x) { spacing_x_ = x; }
    double                      spacing_x() const { return spacing_x_; }

    void                        set_spacing_y(const double& y) { spacing_y_ = y; }
    double                      spacing_y() const { return spacing_y_; }

    void                        set_spacing_z(const double& z) { spacing_z_ = z; }
    double                      spacing_z() const { return spacing_z_; }

    void                        set_slice_thickness(const double& thickness) { slice_thickness_ = thickness; }
    double                      slice_thickness() const { return slice_thickness_; }

    void                        set_spacing_between_slice(const double& spacing) { spacing_between_slice_ = spacing; }
    double                      spacing_between_slice() const { return spacing_between_slice_; }

    void                        set_pixel_representation(const int& representation) { pixel_representation_ = representation; }
    int                         pixel_representation() const { return pixel_representation_; }

    void                        set_rescale_intercept(const int& intercept) { rescale_intercept_ = intercept; }
    int                         rescale_intercept() const { return rescale_intercept_; }

    void                        set_rescale_slope(const int& slope) { rescale_slope_ = slope; }
    int                         rescale_slope() const { return rescale_slope_; }

    void                        set_bits_allocated(const int& bits) { bits_allocated_ = bits; }
    int                         bits_allocated() const { return bits_allocated_; }

    void                        set_bits_stored(const int& bits) { bits_stored_ = bits; }
    int                         bits_stored() const { return bits_stored_; }

    void                        set_high_bit(const int& bit) { high_bit_ = bit; }
    int                         high_bit() const { return high_bit_; }

    void                        set_window_width(const int& ww) { window_width_.push_back(ww); }
    const std::vector<int>&     window_width() const { return window_width_; }

    void                        set_window_center(const int& wc) { window_center_.push_back(wc); }
    const std::vector<int>&     window_center() const { return window_center_; }

    void                        set_pixel_data_window_width(const int& ww) { pixel_data_window_width_ = ww; }
    int                         pixel_data_window_width() const { return pixel_data_window_width_; }

    void                        set_pixel_data_window_center(const int& wc) { pixel_data_window_center_ = wc; }
    int                         pixel_data_window_center() const { return pixel_data_window_center_; }

    void                        TransformPixelData();
    void                        set_instance_raw_data(uchar* const ptr);
    
    uchar*                      get_instance_raw_data(const int& slice) const;
    uchar*                      get_instance_pixel_data(const int& slice) const;
    uchar*                      get_frame_raw_data(const int& slice, const int& frame) const;
    uchar*                      get_frame_pixel_data(const int& slice, const int& frame) const;

private:
    // Basic Information
    //
    QString                     patient_name_ = "";
    QString                     patient_id_ = "";

    QString                     study_instance_uid_ = "";
    QString                     study_description_ = "";

    QString                     series_instance_uid_ = "";
    QString                     series_description_ = "";
    int                         series_number_ = -1;

    QString                     sop_instance_uid_ = "";
    QString                     sop_class_uid_ = "";

    // Image Information
    //
    int                         rows_ = 0;
    int                         cols_ = 0;
    int                         total_instances_ = 0;
    int                         frames_per_instance_ = 1;
    double                      spacing_x_ = 0.0;
    double                      spacing_y_ = 0.0;
    double                      spacing_z_ = 0.0;                   // calculate by slice location
    double                      slice_thickness_ = 0.0;
    double                      spacing_between_slice_ = 0.0;
    int                         pixel_representation_ = 0;
    int                         rescale_intercept_ = 0;
    int                         rescale_slope_ = 1;
    int                         bits_allocated_ = 0;
    int                         bits_stored_ = 0;
    int                         high_bit_ = 0;

    int                         pixel_data_window_width_ = INT_MAX;
    int                         pixel_data_window_center_ = INT_MAX;
    
    std::vector<int>            window_width_ = {};
    std::vector<int>            window_center_ = {};

    std::vector<uchar*>         instance_raw_data_list_ = {};
    std::vector<uchar*>         instance_pixel_data_list_ = {};

    void                        ConvertRawData2PixelData();
};

#endif // DCMDATASET_H
