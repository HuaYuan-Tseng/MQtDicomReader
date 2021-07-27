#include "studybrowser.h"
#include "ui_studybrowser.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

#include <opencv2/opencv.hpp>

#include "tableoperate.h"
#include "dcmdatasetthread.h"
#include "dcmlistthread.h"
#include "dcmdataset.h"
#include "dcmlayer.h"
#include "dcmio.h"

StudyBrowser::StudyBrowser(QWidget* parent) :
    QWidget(parent),
    ui_(new Ui::StudyBrowser)
{
    ui_->setupUi(this);

    SetStudyTableHeader();
    SetSeriesTableHeader();
    SetInstanceTableHeader();

    QObject::connect(ui_->browserButton_open_folder, SIGNAL(clicked()), this, SLOT(ToLoadFromFolder()));
    QObject::connect(ui_->browserButton_open_series, SIGNAL(clicked()), this, SLOT(ToOpenDicomSeries()));
    QObject::connect(ui_->browserButton_clear_all, SIGNAL(clicked()), this, SLOT(ToClearOpenedDicom()));
    QObject::connect(ui_->table_study, SIGNAL(clicked(const QModelIndex&)), this, SLOT(SelectStudyTable(const QModelIndex&)));
    QObject::connect(ui_->table_series, SIGNAL(clicked(const QModelIndex&)), this, SLOT(SelectSeriesTable(const QModelIndex&)));
    QObject::connect(ui_->table_instance, SIGNAL(clicked(const QModelIndex&)), this, SLOT(SelectInstanceTable(const QModelIndex&)));

}

StudyBrowser::~StudyBrowser()
{
    delete ui_;
}

void StudyBrowser::ToLoadFromFolder()
{
    if (is_opening) return;

#ifdef Q_OS_MAC
    QString init_path = "/Users/huayuan/Documents/Dev/Dicom";
#endif
#ifdef Q_OS_WIN
    QString init_path = "D:/TestCases/";
#endif

    GlobalState::study_browser_.open_dir_ =
            QFileDialog::getExistingDirectory(this, "Select Dicom Folder",
                                              init_path,
                                              QFileDialog::ShowDirsOnly |
                                              QFileDialog::DontResolveSymlinks);

    if (GlobalState::study_browser_.open_dir_.isEmpty()) return;
    qDebug() << "Open Path : " << GlobalState::study_browser_.open_dir_;

    DcmIO* dcmio = new DcmIO();
    DcmListThread* thread = new DcmListThread(GlobalState::study_browser_.open_dir_,
                                              GlobalState::study_browser_.dcm_list_);

    QObject::connect(thread, SIGNAL(startToLoadFromFolder(QString*, DcmContent*)), dcmio, SLOT(LoadFromFolder(QString*, DcmContent*)));
    QObject::connect(dcmio, SIGNAL(progress(int)), ui_->progressbar, SLOT(setValue(int)));
    QObject::connect(dcmio, SIGNAL(send(QString*, DcmContent*)), this, SLOT(ReceiveFromOtherThreadDcmIO(QString*, DcmContent*)));
    QObject::connect(dcmio, SIGNAL(finish()), this, SLOT(RefreshAllTable()));
    QObject::connect(dcmio, SIGNAL(finish()), dcmio, SLOT(deleteLater()));
    QObject::connect(dcmio, SIGNAL(destroyed(QObject*)), thread, SLOT(quit()));
    QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    is_opening = true;
    dcmio->moveToThread(thread);
    thread->start();
}

void StudyBrowser::ToOpenDicomSeries()
{
    if (is_opening) return;
    if (GlobalState::study_browser_.dcm_list_.empty()) return;
    GlobalState::study_browser_.dcm_data_set_.ClearAll();

    const int patient_index = GlobalState::study_browser_.select_patient_index_;
    const int study_index = GlobalState::study_browser_.select_study_index_;
    const int series_index = GlobalState::study_browser_.select_series_index_;
    auto& select_series = GlobalState::study_browser_.dcm_list_[patient_index].study_list_[study_index].series_list_[series_index];
    std::vector<DcmInstance> instance_list;
    if (select_series.has_multi_frames_instance_)
    {
        const int instance_index = GlobalState::study_browser_.select_instance_index_;
        if (instance_index == -1) return;
        instance_list.push_back(select_series.instance_list_[instance_index]);
    }
    else
    {
        instance_list = select_series.instance_list_;
    }
    
    DcmIO* dcmio = new DcmIO();
    DcmDatasetThread* thread = new DcmDatasetThread(instance_list, GlobalState::study_browser_.dcm_data_set_);

    QObject::connect(thread, SIGNAL(startToLoadInstanceDataSet(std::vector<DcmInstance>*, DcmDataSet*)), dcmio, SLOT(LoadInstanceDataSet(std::vector<DcmInstance>*, DcmDataSet*)));
    QObject::connect(dcmio, SIGNAL(progress(int)), ui_->progressbar, SLOT(setValue(int)));
    QObject::connect(dcmio, SIGNAL(send(DcmDataSet*)), this, SLOT(ReceiveFromOtherThreadDcmIO(DcmDataSet*)));
    QObject::connect(dcmio, SIGNAL(finish()), dcmio, SLOT(deleteLater()));
    QObject::connect(dcmio, SIGNAL(destroyed(QObject*)), thread, SLOT(quit()));
    QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    GlobalState::study_browser_.open_patient_index_ = patient_index;
    GlobalState::study_browser_.open_study_index_ = study_index;
    GlobalState::study_browser_.open_series_index_ = series_index;

    is_opening = true;
    dcmio->moveToThread(thread);
    thread->start();
}

void StudyBrowser::ReceiveFromOtherThreadDcmIO(QString* path, DcmContent* list)
{
    GlobalState::study_browser_.open_dir_ = *path;
    GlobalState::study_browser_.dcm_list_ = *list;
    is_opening = false;
}
void StudyBrowser::ReceiveFromOtherThreadDcmIO(DcmDataSet* data_set)
{
    GlobalState::study_browser_.dcm_data_set_ = *data_set;
    is_opening = false;

    emit StartToSetupImageViewer1();
    emit SwitchToImageViewer1();
}

void StudyBrowser::RefreshAllTable()
{
    RefreshStudyTableContents();
    RefreshSeriesTableContents();
    RefreshInstanceTableContents();
}

void StudyBrowser::RefreshStudyTableContents()
{
    QStandardItemModel* model = reinterpret_cast<QStandardItemModel*>(ui_->table_study->model());
    if (model->rowCount() > 0) model->removeRows(0, model->rowCount());
    if (GlobalState::study_browser_.dcm_list_.empty()) return;

    std::vector<std::vector<QString>> study_table;
    for (const auto& patient : GlobalState::study_browser_.dcm_list_)
    {
        std::vector<QString> res;
        for (const auto& study : patient.study_list_)
        {
            res.push_back(patient.patient_id_);
            res.push_back(patient.patient_name_);
            res.push_back(study.study_description_);
        }
        study_table.push_back(res);
    }
    TableOperate::RefreshTableContents(model, study_table);
    GlobalState::study_browser_.study_table_.select_index = 0;
    GlobalState::study_browser_.select_patient_index_ = 0;
    GlobalState::study_browser_.select_study_index_ = 0;
    ui_->table_study->selectRow(0);
}

void StudyBrowser::RefreshSeriesTableContents()
{
    QStandardItemModel* model = reinterpret_cast<QStandardItemModel*>(ui_->table_series->model());
    if (model->rowCount() > 0) model->removeRows(0, model->rowCount());
    if (GlobalState::study_browser_.dcm_list_.empty()) return;

    int patient_index = GlobalState::study_browser_.select_patient_index_;
    int study_index = GlobalState::study_browser_.select_study_index_;
    std::vector<std::vector<QString>> series_table;
    for (const auto& s :
         GlobalState::study_browser_.dcm_list_[patient_index].study_list_[study_index].series_list_)
    {
        std::vector<QString> res;
        res.push_back(QString::number(s.series_number_));
        res.push_back(s.series_description_);
        series_table.push_back(res);
    }
    TableOperate::RefreshTableContents(model, series_table);
    GlobalState::study_browser_.series_table_.select_index = 0;
    GlobalState::study_browser_.select_series_index_ = 0;
    ui_->table_series->selectRow(0);
}

void StudyBrowser::RefreshInstanceTableContents()
{
    QStandardItemModel* model = reinterpret_cast<QStandardItemModel*>(ui_->table_instance->model());
    if (model->rowCount() > 0) model->removeRows(0, model->rowCount());
    if (GlobalState::study_browser_.dcm_list_.empty()) return;

    int patient_index = GlobalState::study_browser_.select_patient_index_;
    int study_index = GlobalState::study_browser_.select_study_index_;
    int series_index = GlobalState::study_browser_.select_series_index_;
    auto& select_series = GlobalState::study_browser_.dcm_list_[patient_index].study_list_[study_index].series_list_[series_index];
    
    std::vector<std::vector<QString>> info_table;
    for (const auto& instance : select_series.instance_list_)
    {
        if (instance.number_of_frames_ <= 1) continue;
        std::vector<QString> info;
        info.push_back(instance.view_name_);
        info.push_back(instance.sop_instance_uid_);
        info_table.push_back(info);
    }
    if (info_table.empty()) return;
    
    TableOperate::RefreshTableContents(model, info_table);
    GlobalState::study_browser_.instance_table_.select_index = 0;
    GlobalState::study_browser_.select_instance_index_ = 0;
    ui_->table_instance->selectRow(0);
}

void StudyBrowser::SelectStudyTable(const QModelIndex& index)
{
    if (!index.isValid()) return;
    int study_index = index.row() + 1;
    GlobalState::study_browser_.study_table_.select_index = index.row();
    auto it = GlobalState::study_browser_.dcm_list_.begin();
    for (int patient = 0; it != GlobalState::study_browser_.dcm_list_.end(); ++it, ++patient)
    {
        int study_count = static_cast<int>(it->study_list_.size());
        if (study_index > study_count)
        {
            study_index -= study_count;
        }
        else
        {
            GlobalState::study_browser_.select_patient_index_ = patient;
            GlobalState::study_browser_.select_study_index_ = study_index - 1;
            break;
        }
    }
    RefreshSeriesTableContents();
    RefreshInstanceTableContents();
}

void StudyBrowser::SelectSeriesTable(const QModelIndex& index)
{
    if (!index.isValid()) return;
    GlobalState::study_browser_.series_table_.select_index = index.row();
    GlobalState::study_browser_.select_series_index_ = index.row();
    RefreshInstanceTableContents();
}

void StudyBrowser::SelectInstanceTable(const QModelIndex& index)
{
    if (!index.isValid()) return;
    if (index.row() != GlobalState::study_browser_.instance_table_.select_index)
    {
        GlobalState::study_browser_.instance_table_.select_index = index.row();
        GlobalState::study_browser_.select_instance_index_ = index.row();
    }
    else
    {
        ui_->table_instance->clearSelection();
        GlobalState::study_browser_.instance_table_.select_index = -1;
        GlobalState::study_browser_.select_instance_index_ = -1;
    }
}

void StudyBrowser::ToClearOpenedDicom()
{
    if (is_opening) return;
    GlobalState::study_browser_.open_dir_.clear();
    GlobalState::study_browser_.dcm_list_.clear();
    GlobalState::study_browser_.dcm_data_set_.ClearAll();
    GlobalState::study_browser_.study_table_.select_index = 0;
    GlobalState::study_browser_.series_table_.select_index = 0;
    GlobalState::study_browser_.instance_table_.select_index = -1;
    GlobalState::study_browser_.select_patient_index_ = 0;
    GlobalState::study_browser_.select_series_index_ = 0;
    GlobalState::study_browser_.select_study_index_ = 0;
    GlobalState::study_browser_.select_instance_index_ = -1;
    GlobalState::study_browser_.open_patient_index_ = 0;
    GlobalState::study_browser_.open_study_index_ = 0;
    GlobalState::study_browser_.open_series_index_ = 0;
    GlobalState::study_browser_.open_instance_index_ = -1;
    RefreshStudyTableContents();
    RefreshSeriesTableContents();
    RefreshInstanceTableContents();
    ui_->progressbar->setValue(0);
}

void StudyBrowser::SetStudyTableHeader()
{
    int size = static_cast<int>(GlobalState::study_browser_.study_table_.header.size());
    QStandardItemModel* model = new QStandardItemModel(0, size, this);
    TableOperate::SetTableHeader(model, GlobalState::study_browser_.study_table_.header);
    ui_->table_study->setModel(model);
    ui_->table_study->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_->table_study->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_->table_study->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    ui_->table_study->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
}

void StudyBrowser::SetSeriesTableHeader()
{
    int size = static_cast<int>(GlobalState::study_browser_.series_table_.header.size());
    QStandardItemModel* model = new QStandardItemModel(0, size, this);
    TableOperate::SetTableHeader(model, GlobalState::study_browser_.series_table_.header);
    ui_->table_series->setModel(model);
    ui_->table_series->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_->table_series->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_->table_series->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    ui_->table_series->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
}

void StudyBrowser::SetInstanceTableHeader()
{
    int size = static_cast<int>(GlobalState::study_browser_.instance_table_.header.size());
    QStandardItemModel* model = new QStandardItemModel(0, size, this);
    TableOperate::SetTableHeader(model, GlobalState::study_browser_.instance_table_.header);
    ui_->table_instance->setModel(model);
    ui_->table_instance->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_->table_instance->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_->table_instance->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    ui_->table_instance->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
}

