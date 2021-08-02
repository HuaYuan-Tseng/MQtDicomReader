#ifndef GLOBALSTATE_H
#define GLOBALSTATE_H

#include <QString>
#include <vector>
#include <map>

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

enum class ViewName {
    TRA, COR, SAG
};

struct OperateViewer {
    ViewName                operate_view = ViewName::TRA;
    bool                    is_control_current_view = false;
    double                  curr_mouse_world_pos_[3] = { 0, 0, 0 };
    int                     curr_mouse_pixel_pos_[3] = { 0, 0, 0 };
    int                     control_current_slice = 0;
};

class GlobalState {
public:
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
    StudyBrowser            study_browser_;

    class ImageViewer1 {
    public:
        std::map<ViewName, OperateViewer> control_map_ = { {ViewName::TRA, OperateViewer{ViewName::TRA}},
                                                            {ViewName::SAG, OperateViewer{ViewName::SAG}},
                                                            {ViewName::COR, OperateViewer{ViewName::COR}} };
        
        ViewName            current_control_view_ = ViewName::TRA;
        bool                is_draging_left_ = false;
        bool                is_draging_right_ = false;
        bool                is_pressed_shift_ = false;
        bool                is_pressed_ctrl_ = false;
        bool                is_pressed_alt_ = false;
    };
    ImageViewer1            image_viewer_1_;
};

#endif // GLOBALSTATE_H
