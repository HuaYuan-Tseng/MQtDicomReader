#ifndef STUDYBROWSER_H
#define STUDYBROWSER_H

#include <QWidget>

#include "globalstate.h"
#include "dcmlayer.h"

namespace Ui {
class StudyBrowser;
}

class StudyBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit StudyBrowser(GlobalState* state, QWidget* parent = nullptr);
    ~StudyBrowser();

private:
    void                        SetStudyTableHeader();
    void                        SetSeriesTableHeader();
    void                        SetInformationTableHeader();


private slots:
    void                        ToOpenFromFolder();

public:
    Ui::StudyBrowser*           ui_;

private:
    GlobalState*                global_state_ = nullptr;
    DcmList*                    dcm_list_ = nullptr;
};

#endif // STUDYBROWSER_H
