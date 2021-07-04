#ifndef STUDYBROWSER_H
#define STUDYBROWSER_H

#include <QWidget>

#include "globalstate.h"

namespace Ui {
class StudyBrowser;
}

class StudyBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit StudyBrowser(GlobalState* state, QWidget* parent = nullptr);
    ~StudyBrowser();

private slots:
    void                        ToOpenFromFolder();

public:
    Ui::StudyBrowser*           ui_;

private:
    GlobalState*                global_state_ = nullptr;

};

#endif // STUDYBROWSER_H
