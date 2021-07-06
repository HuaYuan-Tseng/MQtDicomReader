#ifndef GLOBALSTATE_H
#define GLOBALSTATE_H

#include <QString>
#include <vector>

enum MainStackedWidget
{
    STUDY_BROWSER
};

struct Table
{
    std::vector<QString>    header = {};
    int                     select_index = -1;
};

struct GlobalState
{
    struct StudyBrowser
    {
        QString             open_dir = "";
        Table               study_table = {{"Patient ID", "Patient Name", "Access No.", "Description"}, -1};
        Table               series_table = {{"Series No.", "Series Description"}, -1};
        Table               information_table = {{"Description", "Value"}, -1};
    }   study_browser_;
};

#endif // GLOBALSTATE_H
