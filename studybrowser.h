#ifndef STUDYBROWSER_H
#define STUDYBROWSER_H

#include <QWidget>

namespace Ui {
class StudyBrowser;
}

class StudyBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit StudyBrowser(QWidget *parent = nullptr);
    ~StudyBrowser();

public:
    Ui::StudyBrowser*           ui_;

private:

};

#endif // STUDYBROWSER_H
