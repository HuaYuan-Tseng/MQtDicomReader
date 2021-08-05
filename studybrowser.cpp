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

StudyBrowser::StudyBrowser(GlobalState* state, QWidget* parent) :
    QWidget(parent),
    ui_(new Ui::StudyBrowser),
    global_state_(state)
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

    this->ToLoadFromFolder();
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
    QString init_path = "D:/TestCases/LungCT";
#endif
    
    /*global_state_->study_browser_.open_dir_ =
            QFileDialog::getExistingDirectory(this, "Select Dicom Folder",
                                              init_path,
                                              QFileDialog::ShowDirsOnly |
                                              QFileDialog::DontResolveSymlinks);*/
    
    global_state_->study_browser_.open_dir_ = "D:/TestCases/LungCT/3759213/";

    if (global_state_->study_browser_.open_dir_.isEmpty()) return;

    DcmIO* dcmio = new DcmIO();
    DcmListThread* thread = new DcmListThread(global_state_->study_browser_.open_dir_,
                                              global_state_->study_browser_.dcm_list_);

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
    if (global_state_->study_browser_.dcm_list_.empty()) return;
    global_state_->study_browser_.dcm_data_set_.ClearAll();

    const int patient_index = global_state_->study_browser_.select_patient_index_;
    const int study_index = global_state_->study_browser_.select_study_index_;
    const int series_index = global_state_->study_browser_.select_series_index_;
    auto& select_series = global_state_->study_browser_.dcm_list_[patient_index].study_list_[study_index].series_list_[series_index];
    std::vector<DcmInstance> instance_list;
    if (select_series.has_multi_frames_instance_)
    {
        const int instance_index = global_state_->study_browser_.select_instance_index_;
        if (instance_index == -1) return;
        instance_list.push_back(select_series.instance_list_[instance_index]);
    }
    else
    {
        instance_list = select_series.instance_list_;
    }
    
    DcmIO* dcmio = new DcmIO();
    DcmDatasetThread* thread = new DcmDatasetThread(instance_list, global_state_->study_browser_.dcm_data_set_);

    QObject::connect(thread, SIGNAL(startToLoadInstanceDataSet(std::vector<DcmInstance>*, DcmDataSet*)), dcmio, SLOT(LoadInstanceDataSet(std::vector<DcmInstance>*, DcmDataSet*)));
    QObject::connect(dcmio, SIGNAL(progress(int)), ui_->progressbar, SLOT(setValue(int)));
    QObject::connect(dcmio, SIGNAL(send(DcmDataSet*)), this, SLOT(ReceiveFromOtherThreadDcmIO(DcmDataSet*)));
    QObject::connect(dcmio, SIGNAL(finish()), dcmio, SLOT(deleteLater()));
    QObject::connect(dcmio, SIGNAL(destroyed(QObject*)), thread, SLOT(quit()));
    QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    global_state_->study_browser_.open_patient_index_ = patient_index;
    global_state_->study_browser_.open_study_index_ = study_index;
    global_state_->study_browser_.open_series_index_ = series_index;

    is_opening = true;
    dcmio->moveToThread(thread);
    thread->start();
}

void StudyBrowser::ReceiveFromOtherThreadDcmIO(QString* path, DcmContent* list)
{
    global_state_->study_browser_.open_dir_ = *path;
    global_state_->study_browser_.dcm_list_ = *list;
    is_opening = false;

    this->ToOpenDicomSeries();
}
void StudyBrowser::ReceiveFromOtherThreadDcmIO(DcmDataSet* data_set)
{
    global_state_->study_browser_.dcm_data_set_ = *data_set;
    is_opening = false;
    
    //auto& data = global_state_->study_browser_.dcm_data_set_;
    //data.TransformPixelData();
    //cv::Mat src(data.rows(), data.cols(), CV_8UC1, data.get_instance_pixel_data(0));
    //cv::imshow("src", src);

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
    if (global_state_->study_browser_.dcm_list_.empty()) return;

    std::vector<std::vector<QString>> study_table;
    for (const auto& patient : global_state_->study_browser_.dcm_list_)
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
    global_state_->study_browser_.study_table_.select_index = 0;
    global_state_->study_browser_.select_patient_index_ = 0;
    global_state_->study_browser_.select_study_index_ = 0;
    ui_->table_study->selectRow(0);
}

void StudyBrowser::RefreshSeriesTableContents()
{
    QStandardItemModel* model = reinterpret_cast<QStandardItemModel*>(ui_->table_series->model());
    if (model->rowCount() > 0) model->removeRows(0, model->rowCount());
    if (global_state_->study_browser_.dcm_list_.empty()) return;

    int patient_index = global_state_->study_browser_.select_patient_index_;
    int study_index = global_state_->study_browser_.select_study_index_;
    std::vector<std::vector<QString>> series_table;
    for (const auto& s :
         global_state_->study_browser_.dcm_list_[patient_index].study_list_[study_index].series_list_)
    {
        std::vector<QString> res;
        res.push_back(QString::number(s.series_number_));
        res.push_back(s.series_description_);
        series_table.push_back(res);
    }
    TableOperate::RefreshTableContents(model, series_table);
    global_state_->study_browser_.series_table_.select_index = 0;
    global_state_->study_browser_.select_series_index_ = 0;
    ui_->table_series->selectRow(0);
}

void StudyBrowser::RefreshInstanceTableContents()
{
    QStandardItemModel* model = reinterpret_cast<QStandardItemModel*>(ui_->table_instance->model());
    if (model->rowCount() > 0) model->removeRows(0, model->rowCount());
    if (global_state_->study_browser_.dcm_list_.empty()) return;

    int patient_index = global_state_->study_browser_.select_patient_index_;
    int study_index = global_state_->study_browser_.select_study_index_;
    int series_index = global_state_->study_browser_.select_series_index_;
    auto& select_series = global_state_->study_browser_.dcm_list_[patient_index].study_list_[study_index].series_list_[series_index];
    
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
    global_state_->study_browser_.instance_table_.select_index = 0;
    global_state_->study_browser_.select_instance_index_ = 0;
    ui_->table_instance->selectRow(0);
}

void StudyBrowser::SelectStudyTable(const QModelIndex& index)
{
    if (!index.isValid()) return;
    int study_index = index.row() + 1;
    global_state_->study_browser_.study_table_.select_index = index.row();
    auto it = global_state_->study_browser_.dcm_list_.begin();
    for (int patient = 0; it != global_state_->study_browser_.dcm_list_.end(); ++it, ++patient)
    {
        int study_count = static_cast<int>(it->study_list_.size());
        if (study_index > study_count)
        {
            study_index -= study_count;
        }
        else
        {
            global_state_->study_browser_.select_patient_index_ = patient;
            global_state_->study_browser_.select_study_index_ = study_index - 1;
            break;
        }
    }
    RefreshSeriesTableContents();
    RefreshInstanceTableContents();
}

void StudyBrowser::SelectSeriesTable(const QModelIndex& index)
{
    if (!index.isValid()) return;
    global_state_->study_browser_.series_table_.select_index = index.row();
    global_state_->study_browser_.select_series_index_ = index.row();
    RefreshInstanceTableContents();
}

void StudyBrowser::SelectInstanceTable(const QModelIndex& index)
{
    if (!index.isValid()) return;
    if (index.row() != global_state_->study_browser_.instance_table_.select_index)
    {
        global_state_->study_browser_.instance_table_.select_index = index.row();
        global_state_->study_browser_.select_instance_index_ = index.row();
    }
    else
    {
        ui_->table_instance->clearSelection();
        global_state_->study_browser_.instance_table_.select_index = -1;
        global_state_->study_browser_.select_instance_index_ = -1;
    }
}

void StudyBrowser::ToClearOpenedDicom()
{
    if (is_opening) return;
    global_state_->study_browser_.open_dir_.clear();
    global_state_->study_browser_.dcm_list_.clear();
    global_state_->study_browser_.dcm_data_set_.ClearAll();
    global_state_->study_browser_.study_table_.select_index = 0;
    global_state_->study_browser_.series_table_.select_index = 0;
    global_state_->study_browser_.instance_table_.select_index = -1;
    global_state_->study_browser_.select_patient_index_ = 0;
    global_state_->study_browser_.select_series_index_ = 0;
    global_state_->study_browser_.select_study_index_ = 0;
    global_state_->study_browser_.select_instance_index_ = -1;
    global_state_->study_browser_.open_patient_index_ = 0;
    global_state_->study_browser_.open_study_index_ = 0;
    global_state_->study_browser_.open_series_index_ = 0;
    global_state_->study_browser_.open_instance_index_ = -1;
    RefreshStudyTableContents();
    RefreshSeriesTableContents();
    RefreshInstanceTableContents();
    ui_->progressbar->setValue(0);
}

void StudyBrowser::SetStudyTableHeader()
{
    int size = static_cast<int>(global_state_->study_browser_.study_table_.header.size());
    QStandardItemModel* model = new QStandardItemModel(0, size, this);
    TableOperate::SetTableHeader(model, global_state_->study_browser_.study_table_.header);
    ui_->table_study->setModel(model);
    ui_->table_study->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_->table_study->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_->table_study->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    ui_->table_study->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
}

void StudyBrowser::SetSeriesTableHeader()
{
    int size = static_cast<int>(global_state_->study_browser_.series_table_.header.size());
    QStandardItemModel* model = new QStandardItemModel(0, size, this);
    TableOperate::SetTableHeader(model, global_state_->study_browser_.series_table_.header);
    ui_->table_series->setModel(model);
    ui_->table_series->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_->table_series->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_->table_series->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    ui_->table_series->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
}

void StudyBrowser::SetInstanceTableHeader()
{
    int size = static_cast<int>(global_state_->study_browser_.instance_table_.header.size());
    QStandardItemModel* model = new QStandardItemModel(0, size, this);
    TableOperate::SetTableHeader(model, global_state_->study_browser_.instance_table_.header);
    ui_->table_instance->setModel(model);
    ui_->table_instance->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_->table_instance->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_->table_instance->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    ui_->table_instance->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
}

