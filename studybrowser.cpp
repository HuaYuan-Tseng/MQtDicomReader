#include "studybrowser.h"
#include "ui_studybrowser.h"

#include "tableoperate.h"

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
}

StudyBrowser::~StudyBrowser()
{
    delete ui_;
}

void StudyBrowser::ToOpenFromFolder()
{
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
