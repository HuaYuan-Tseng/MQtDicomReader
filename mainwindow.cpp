#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);
    this->setWindowTitle("QtDicomReader");
    this->setFixedSize(this->width(), this->height());

    global_state_ = new GlobalState();
    
    // Menubar
    study_browser_ = new StudyBrowser(global_state_);
    ui_->mainwindow_stacked_widget->insertWidget(MainStackedWidget::STUDY_BROWSER, study_browser_);
    image_viewer_1_ = new ImageViewer1(global_state_);
    ui_->mainwindow_stacked_widget->insertWidget(MainStackedWidget::IMAGE_VIEWER_1, image_viewer_1_);

    QObject::connect(ui_->menuButton_exit, SIGNAL(clicked()), this, SLOT(ToExitApp()));
    QObject::connect(ui_->menuButton_study_browser, SIGNAL(clicked()), this, SLOT(ToStudyBrowser()));
    QObject::connect(ui_->menuButton_image_viewer_1, SIGNAL(clicked()), this, SLOT(ToImageViewer1()));
    QObject::connect(study_browser_, SIGNAL(SwitchToImageViewer1()), this, SLOT(ToImageViewer1()));
    QObject::connect(study_browser_, SIGNAL(StartToSetupImageViewer1()), image_viewer_1_, SLOT(SetupViewers()));
    QObject::connect(study_browser_, SIGNAL(StartToClearImageViewer1()), image_viewer_1_, SLOT(ClearAllViewers()));

    ui_->mainwindow_stacked_widget->setCurrentIndex(MainStackedWidget::STUDY_BROWSER);
}

MainWindow::~MainWindow()
{
    delete ui_;
}

void MainWindow::ToExitApp()
{
    QMessageBox msg;
    msg.setWindowTitle("QtDicomReader");
    msg.setText("Do you want to exit QtDicomReader ?");
    msg.setStandardButtons(QMessageBox::Yes);
    msg.addButton(QMessageBox::No);
    msg.setDefaultButton(QMessageBox::No);
    if (msg.exec() == QMessageBox::Yes)
    {
        QApplication::quit();
    }
}

void MainWindow::ToStudyBrowser()
{
    ui_->mainwindow_stacked_widget->setCurrentIndex(MainStackedWidget::STUDY_BROWSER);
}

void MainWindow::ToImageViewer1()
{
    ui_->mainwindow_stacked_widget->setCurrentIndex(MainStackedWidget::IMAGE_VIEWER_1);
}
