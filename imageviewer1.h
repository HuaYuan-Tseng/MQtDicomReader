#ifndef IMAGEVIEWER1_H
#define IMAGEVIEWER1_H

#include <QWidget>

#include "globalstate.h"

namespace Ui {
class ImageViewer1;
}

class ImageViewer1 : public QWidget
{
    Q_OBJECT

public:
    explicit ImageViewer1(QWidget *parent = nullptr);
    ~ImageViewer1();

private:
    Ui::ImageViewer1* ui_;
};

#endif // IMAGEVIEWER1_H
