#include "studybrowser.h"
#include "ui_studybrowser.h"

StudyBrowser::StudyBrowser(GlobalState* state, QWidget* parent) :
    QWidget(parent),
    ui_(new Ui::StudyBrowser),
    global_state_(state)
{
    ui_->setupUi(this);

    QObject::connect(ui_->browserButton_open_folder, SIGNAL(clicked()), this, SLOT(ToOpenFromFolder()));
}

StudyBrowser::~StudyBrowser()
{
    delete ui_;
}

void StudyBrowser::ToOpenFromFolder()
{

}
