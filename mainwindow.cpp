#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);
    this->setWindowTitle("QtDicomReader");
    this->setFixedSize(this->width(), this->height());

    QObject::connect(ui_->menuButton_exit, SIGNAL(clicked()), this, SLOT(ToExitApp()));
    QObject::connect(ui_->menuButton_study_browser, SIGNAL(clicked()), this, SLOT(ToStudyBrowser()));

    // Menubar
    global_state_ = new GlobalState();
    study_browser_ = new StudyBrowser(global_state_);
    ui_->mainwindow_stacked_widget->insertWidget(MainStackedWidget::STUDY_BROWSER, study_browser_);


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
