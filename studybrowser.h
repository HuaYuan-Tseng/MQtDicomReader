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
    void                        ToLoadFromFolder();
    void                        ToOpenDicomSeries();
    void                        ToClearOpenedDicom();

    void                        RefreshAllTable();
    void                        SelectStudyTable(const QModelIndex& index);
    void                        SelectSeriesTable(const QModelIndex& index);
    void                        SelectInformationTable(const QModelIndex& index);

    // while this slot receive signal, represent that "LoadFromFolder" is finish.
    void                        ReceiveFromOtherThreadDcmIO(QString& path, DcmContent& list);

public:
    Ui::StudyBrowser*           ui_;

private:
    bool                        is_opening = false;

};

#endif // STUDYBROWSER_H
