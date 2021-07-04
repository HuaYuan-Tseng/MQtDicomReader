#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "globalstate.h"
#include "studybrowser.h"

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
    void            ToExitApp();
    void            ToStudyBrowser();


private:
    Ui::MainWindow*                     ui_;
    StudyBrowser*                       study_browser_;

};
#endif // MAINWINDOW_H
