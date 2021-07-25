#ifndef DCMDATASET_H
#define DCMDATASET_H

#include <QString>
#include <vector>

class DcmDataSet {
public:
    DcmDataSet();
    void                        ClearAll();

    void                        set_patient_name(const QString& name);
    QString                     patient_name() const;

    void                        set_patient_id(const QString& id);
    QString                     patient_id() const;

    void                        set_study_instance_uid(const QString& uid);
    QString                     study_instance_uid() const;

    void                        set_study_description(const QString& description);
    QString                     study_description() const;

    void                        set_series_instance_uid(const QString& uid);
    QString                     series_instance_uid() const;

    void                        set_series_description(const QString& description);
    QString                     series_description() const;

    void                        set_series_number(const int& num);
    int                         series_number() const;

    void                        set_sop_instance_uid(const QString& uid);
    QString                     sop_instance_uid() const;

    void                        set_sop_class_uid(const QString& uid);
    QString                     sop_class_uid() const;

    void                        set_rows(const int& rows);
    int                         rows() const;

    void                        set_cols(const int& cols);
    int                         cols() const;

    void                        set_total_instances(const int& instances);
    int                         total_instances() const;

    void                        set_frames_per_instance(const int& frames);
    int                         frames_per_instance() const;

    void                        set_spacing_x(const double& x);
    double                      spacing_x() const;

    void                        set_spacing_y(const double& y);
    double                      spacing_y() const;

    void                        set_spacing_z(const double& z);
    double                      spacing_z() const;

    void                        set_slice_thickness(const double& thickness);
    double                      slice_thickness() const;

    void                        set_spacing_between_slice(const double& spacing);
    double                      spacing_between_slice() const;

    void                        set_pixel_representation(const int& representation);
    int                         pixel_representation() const;

    void                        set_rescale_intercept(const int& intercept);
    int                         rescale_intercept() const;

    void                        set_rescale_slope(const int& slope);
    int                         rescale_slope() const;

    void                        set_bits_allocated(const int& bits);
    int                         bits_allocated() const;

    void                        set_bits_stored(const int& bits);
    int                         bits_stored() const;

    void                        set_high_bit(const int& bit);
    int                         high_bit() const;

    void                        set_window_width(const int& ww);
    const std::vector<int>&     window_width() const;

    void                        set_window_center(const int& wc);
    const std::vector<int>&     window_center() const;

    void                        set_pixel_data_window_width(const int& ww);
    int                         pixel_data_window_width() const;

    void                        set_pixel_data_window_center(const int& wc);
    int                         pixel_data_window_center() const;

    void                        TransformPixelData();
    void                        set_instance_raw_data(uchar* raw);
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
    int                         rows_ = 1;
    int                         cols_ = 1;
    int                         total_instances_ = 1;
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

    std::vector<uchar*>         instance_raw_data_list_ = {};       // for input
    std::vector<uchar*>         instance_pixel_data_list_ = {};     // for output

    void                        ConvertRawData2PixelData();
};

#endif // DCMDATASET_H
