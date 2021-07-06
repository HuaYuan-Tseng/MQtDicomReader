#include "studybrowser.h"
#include "ui_studybrowser.h"
#include <QFileDialog>
#include <QMessageBox>

#include "tableoperate.h"
#include "dcmio.h"

StudyBrowser::StudyBrowser(GlobalState* state, QWidget* parent) :
    QWidget(parent),
    ui_(new Ui::StudyBrowser),
    global_state_(state)
{
    ui_->setupUi(this);

    SetStudyTableHeader();
    SetSeriesTableHeader();
    SetInformationTableHeader();

    QObject::connect(ui_->browserButton_open_folder, SIGNAL(clicked()), this, SLOT(ToOpenFromFolder()));

    dcm_list_ = new DcmList();
}

StudyBrowser::~StudyBrowser()
{
    delete ui_;
}

void StudyBrowser::ToOpenFromFolder()
{
    global_state_->study_browser_.open_dir =
            QFileDialog::getExistingDirectory(this, "Select Dicom Folder",
                                              "/Users/huayuan/Documents/Dev/Dicom",
                                              QFileDialog::ShowDirsOnly |
                                              QFileDialog::DontResolveSymlinks);

    if (global_state_->study_browser_.open_dir.isEmpty())
    {
        QMessageBox::warning(this, "QtDicomReader", "This path is empty !");
        return;
    }
    qDebug() << "Open Path : " << global_state_->study_browser_.open_dir;

    DcmIO* dcmio = new DcmIO();
    QObject::connect(dcmio, SIGNAL(progress(int)), ui_->progressbar, SLOT(setValue(int)));
    if (dcmio->LoadFromFolder(global_state_->study_browser_.open_dir, *dcm_list_))
    {
        qDebug() << "Success !";
    }
    QObject::disconnect(dcmio, SIGNAL(progress(int)), ui_->progressbar, SLOT(setValue(int)));
    delete dcmio;
}

void StudyBrowser::SetStudyTableHeader()
{
    int size = static_cast<int>(global_state_->study_browser_.study_table.header.size());
    QStandardItemModel* model = new QStandardItemModel(0, size, this);
    TableOperate::SetTableHeader(model, global_state_->study_browser_.study_table.header);
    ui_->table_study->setModel(model);
    ui_->table_study->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void StudyBrowser::SetSeriesTableHeader()
{
    int size = static_cast<int>(global_state_->study_browser_.series_table.header.size());
    QStandardItemModel* model = new QStandardItemModel(0, size, this);
    TableOperate::SetTableHeader(model, global_state_->study_browser_.series_table.header);
    ui_->table_series->setModel(model);
    ui_->table_series->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void StudyBrowser::SetInformationTableHeader()
{
    int size = static_cast<int>(global_state_->study_browser_.information_table.header.size());
    QStandardItemModel* model = new QStandardItemModel(0, size, this);
    TableOperate::SetTableHeader(model, global_state_->study_browser_.information_table.header);
    ui_->table_information->setModel(model);
    ui_->table_information->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}
