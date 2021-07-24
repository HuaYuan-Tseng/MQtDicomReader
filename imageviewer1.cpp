#include "imageviewer1.h"
#include "ui_imageviewer1.h"


ImageViewer1::ImageViewer1(QWidget *parent) :
    QWidget(parent),
    ui_(new Ui::ImageViewer1)
{
    ui_->setupUi(this);
}

ImageViewer1::~ImageViewer1()
{
    delete ui_;
}

void ImageViewer1::SetupViewers()
{
}
