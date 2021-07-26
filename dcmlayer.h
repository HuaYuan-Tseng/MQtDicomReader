#ifndef DCMLAYER_H
#define DCMLAYER_H

#include <QString>
#include <vector>

class DcmInstance {
public:
    QString                     file_path_ = "";
    QString                     view_name_ = "";
    QString                     sop_instance_uid_ = "";
    int                         number_of_frames_ = 0;
    int                         instance_number_ = 0;
};

class DcmSeries {
public:
    QString                     series_instance_uid_ = "";
    QString                     series_description_ = "";
    int                         series_number_ = 0;
    
    bool                        has_multi_frames_instance_ = false;
    std::vector<DcmInstance>    instance_list_ = {};
    std::vector<DcmInstance>    sr_instance_list_ = {};
};

class DcmStudy {
public:
    QString                     study_date_ = "";
    QString                     study_description_ = "";
    QString                     study_instance_uid_ = "";

    std::vector<DcmSeries>      series_list_ = {};
};

class DcmPatient {
public:
    QString                     patient_name_ = "";
    QString                     patient_id_ = "";
    QString                     patient_bd_ = "";

    std::vector<DcmStudy>       study_list_ = {};
};

using DcmContent = std::vector<DcmPatient>;

#endif // DCMLAYER_H
