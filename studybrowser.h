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
    StudyBrowser(GlobalState* state, QWidget* parent = nullptr);
    ~StudyBrowser();

private:
    void                        SetStudyTableHeader();
    void                        SetSeriesTableHeader();
    void                        SetInstanceTableHeader();

    void                        RefreshStudyTableContents();
    void                        RefreshSeriesTableContents();
    void                        RefreshInstanceTableContents();

private slots:
    void                        ToLoadFromFolder();
    void                        ToOpenDicomSeries();
    void                        ToClearOpenedDicom();

    void                        RefreshAllTable();
    void                        SelectStudyTable(const QModelIndex& index);
    void                        SelectSeriesTable(const QModelIndex& index);
    void                        SelectInstanceTable(const QModelIndex& index);

    // while this slot receive signal, represent that "LoadFromFolder" or "Open Series" is finish.
    void                        ReceiveFromOtherThreadDcmIO(QString* path, DcmContent* list);
    void                        ReceiveFromOtherThreadDcmIO(DcmDataSet* data_set);

signals:
    void                        StartToSetupImageViewer1();
    void                        SwitchToImageViewer1();

public:
    Ui::StudyBrowser*           ui_;

private:
    GlobalState*                global_state_ = nullptr;
    bool                        is_opening = false;

};

#endif // STUDYBROWSER_H
