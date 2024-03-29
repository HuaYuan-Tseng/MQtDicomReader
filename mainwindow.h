#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "globalstate.h"
#include "studybrowser.h"
#include "imageviewer1.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots :
    void                                ToExitApp();
    void                                ToStudyBrowser();
    void                                ToImageViewer1();


private:
    Ui::MainWindow*                     ui_;
    GlobalState*                        global_state_ = nullptr;
    StudyBrowser*                       study_browser_ = nullptr;
    ImageViewer1*                       image_viewer_1_ = nullptr;

};
#endif // MAINWINDOW_H
