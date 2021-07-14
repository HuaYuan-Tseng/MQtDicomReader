#include "studybrowser.h"
#include "ui_studybrowser.h"
#include <QFileDialog>
#include <QMessageBox>

#include "tableoperate.h"
#include "dcmlayer.h"
#include "dcmio.h"

StudyBrowser::StudyBrowser(QWidget* parent) :
    QWidget(parent),
    ui_(new Ui::StudyBrowser)
{
    ui_->setupUi(this);

    SetStudyTableHeader();
    SetSeriesTableHeader();
    SetInformationTableHeader();

    QObject::connect(ui_->browserButton_open_folder, SIGNAL(clicked()), this, SLOT(ToLoadFromFolder()));
    QObject::connect(ui_->browserButton_open_series, SIGNAL(clicked()), this, SLOT(ToOpenDicomSeries()));
    QObject::connect(ui_->browserButton_clear_all, SIGNAL(clicked()), this, SLOT(ToClearOpenedDicom()));
    QObject::connect(ui_->table_study, SIGNAL(clicked(const QModelIndex&)), this, SLOT(SelectStudyTable(const QModelIndex&)));
    QObject::connect(ui_->table_series, SIGNAL(clicked(const QModelIndex&)), this, SLOT(SelectSeriesTable(const QModelIndex&)));
    QObject::connect(ui_->table_information, SIGNAL(clicked(const QModelIndex&)), this, SLOT(SelectInformationTable(const QModelIndex&)));

}

StudyBrowser::~StudyBrowser()
{
    delete ui_;
}

void StudyBrowser::ToLoadFromFolder()
{
    GlobalState::study_browser_.open_dir_ =
            QFileDialog::getExistingDirectory(this, "Select Dicom Folder",
                                              "/Users/huayuan/Documents/Dev/Dicom",
                                              QFileDialog::ShowDirsOnly |
                                              QFileDialog::DontResolveSymlinks);

    if (GlobalState::study_browser_.open_dir_.isEmpty()) return;
    qDebug() << "Open Path : " << GlobalState::study_browser_.open_dir_;

    DcmIO* dcmio = new DcmIO();
    QObject::connect(dcmio, SIGNAL(progress(int)), ui_->progressbar, SLOT(setValue(int)));
    if (dcmio->LoadFromFolder(GlobalState::study_browser_.open_dir_, GlobalState::study_browser_.dcm_list_))
    {
        RefreshStudyTableContents();
        RefreshSeriesTableContents();
        RefreshInformationTableContents();
    }
    QObject::disconnect(dcmio, SIGNAL(progress(int)), ui_->progressbar, SLOT(setValue(int)));
    delete dcmio;
}

void StudyBrowser::ToOpenDicomSeries()
{
    if (GlobalState::study_browser_.dcm_list_.empty()) return;

    const int patient_index = GlobalState::study_browser_.select_patient_index_;
    const int study_index = GlobalState::study_browser_.select_study_index_;
    const int series_index = GlobalState::study_browser_.select_series_index_;
    auto& instance_list = GlobalState::study_browser_.dcm_list_[patient_index].study_list_[study_index].series_list_[series_index].instance_list_;

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

void StudyBrowser::RefreshInformationTableContents()
{
    QStandardItemModel* model = reinterpret_cast<QStandardItemModel*>(ui_->table_information->model());
    if (model->rowCount() > 0) model->removeRows(0, model->rowCount());
    if (GlobalState::study_browser_.dcm_list_.empty()) return;

    int patient_index = GlobalState::study_browser_.select_patient_index_;
    int study_index = GlobalState::study_browser_.select_study_index_;
    int series_index = GlobalState::study_browser_.select_series_index_;
    std::vector<std::vector<QString>> info_table;
    info_table.push_back({"Patient ID", GlobalState::study_browser_.dcm_list_[patient_index].patient_id_});
    info_table.push_back({"Study Instance UID",
                          GlobalState::study_browser_.dcm_list_[patient_index].study_list_[study_index].study_instance_uid_});
    info_table.push_back({"Series Instance UID",
                         GlobalState::study_browser_.dcm_list_[patient_index].study_list_[study_index].series_list_[series_index].series_instance_uid_});
    TableOperate::RefreshTableContents(model, info_table);
    GlobalState::study_browser_.information_table_.select_index = 0;
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
    RefreshInformationTableContents();
}

void StudyBrowser::SelectSeriesTable(const QModelIndex& index)
{
    if (!index.isValid()) return;
    GlobalState::study_browser_.series_table_.select_index = index.row();
    GlobalState::study_browser_.select_series_index_ = index.row();
    RefreshInformationTableContents();
}

void StudyBrowser::SelectInformationTable(const QModelIndex& index)
{
    if (!index.isValid()) return;
    if (index.row() != GlobalState::study_browser_.information_table_.select_index)
    {
        GlobalState::study_browser_.information_table_.select_index = index.row();
    }
    else
    {
        ui_->table_information->clearSelection();
    }
}

void StudyBrowser::ToClearOpenedDicom()
{
    GlobalState::study_browser_.open_dir_.clear();
    GlobalState::study_browser_.dcm_list_.clear();
    GlobalState::study_browser_.study_table_.select_index = 0;
    GlobalState::study_browser_.series_table_.select_index = 0;
    GlobalState::study_browser_.information_table_.select_index = -1;
    GlobalState::study_browser_.select_patient_index_ = 0;
    GlobalState::study_browser_.select_series_index_ = 0;
    GlobalState::study_browser_.select_study_index_ = 0;
    RefreshStudyTableContents();
    RefreshSeriesTableContents();
    RefreshInformationTableContents();
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

void StudyBrowser::SetInformationTableHeader()
{
    int size = static_cast<int>(GlobalState::study_browser_.information_table_.header.size());
    QStandardItemModel* model = new QStandardItemModel(0, size, this);
    TableOperate::SetTableHeader(model, GlobalState::study_browser_.information_table_.header);
    ui_->table_information->setModel(model);
    ui_->table_information->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_->table_information->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_->table_information->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    ui_->table_information->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
}

/*
 *  //Test Dicom Layer
    auto& list = GlobalState::study_browser_.dcm_list_;
    qDebug() << "Patient Count : " << list.size();
    for (int i = 0; i < list.size(); ++i)
    {
        qDebug() << "Patient " << i << " : Study Size : " << list[i].study_list_.size();
        for (int j = 0; j < list[i].study_list_.size(); ++j)
        {
            qDebug() << "Study " << j << " : Series Size : " << list[i].study_list_[j].series_list_.size();
            for (int k = 0; k < list[i].study_list_[j].series_list_.size(); ++k)
                qDebug() << "Series " << k << " : Instance Size : " << list[i].study_list_[j].series_list_[k].instance_list_.size();
        }
    }
*/
