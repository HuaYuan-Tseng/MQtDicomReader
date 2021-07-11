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
    StudyBrowser(QWidget* parent = nullptr);
    ~StudyBrowser();

private:
    void                        SetStudyTableHeader();
    void                        SetSeriesTableHeader();
    void                        SetInformationTableHeader();
    void                        RefreshStudyTableContents();
    void                        RefreshSeriesTableContents();
    void                        RefreshInformationTableContents();

private slots:
    void                        ToOpenFromFolder();
    void                        ToClearOpenedDicom();
    void                        SelectStudyTable(const QModelIndex& index);
    void                        SelectSeriesTable(const QModelIndex& index);
    void                        SelectInformationTable(const QModelIndex& index);

public:
    Ui::StudyBrowser*           ui_;

};

#endif // STUDYBROWSER_H
