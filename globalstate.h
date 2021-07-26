#ifndef GLOBALSTATE_H
#define GLOBALSTATE_H

#include <QString>
#include <vector>

#include "dcmlayer.h"
#include "dcmdataset.h"

enum MainStackedWidget
{
    STUDY_BROWSER,
    IMAGE_VIEWER_1
};

struct Table
{
    std::vector<QString>    header = {};
    int                     select_index = -1;
};

namespace GlobalState
{
    class StudyBrowser {
    public:
        QString             open_dir_ = "";
        DcmContent          dcm_list_ = {};
        DcmDataSet          dcm_data_set_;

        Table               study_table_ = {{"Patient ID", "Patient Name", "Study Description"}, 0};
        Table               series_table_ = {{"Series No.", "Series Description"}, 0};
        Table               instance_table_ = {{"View Name", "SOP Instance UID"}, -1};

        int                 select_patient_index_ = 0;
        int                 select_study_index_ = 0;
        int                 select_series_index_ = 0;
        int                 select_instance_index_ = -1;

        int                 open_patient_index_ = 0;
        int                 open_study_index_ = 0;
        int                 open_series_index_ = 0;
        int                 open_instance_index_ = -1;
    };
    static StudyBrowser     study_browser_;

    class ImageViewer1 {
    public:
    };
    static ImageViewer1     image_viewer_1_;
};

#endif // GLOBALSTATE_H
