#include "studybrowser.h"
#include "ui_studybrowser.h"

StudyBrowser::StudyBrowser(QWidget *parent) :
    QWidget(parent),
    ui_(new Ui::StudyBrowser)
{
    ui_->setupUi(this);
}

StudyBrowser::~StudyBrowser()
{
    delete ui_;
}
