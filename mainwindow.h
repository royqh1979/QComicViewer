#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ImageWidget;
class PagesNavigator;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:

    void on_actionShow_Double_Pages_toggled(bool arg1);

    void on_actionNext_Page_triggered();

    void on_actionPrev_Page_triggered();

    void on_actionLast_Page_triggered();

    void on_actionFirst_Page_triggered();

    void on_actionExit_triggered();

    void on_actionOpen_triggered();

    void on_actionFit_Width_toggled(bool arg1);

    void on_actionFit_Height_toggled(bool arg1);

    void on_actionFit_Page_toggled(bool arg1);

    void on_actionSingle_First_Page_toggled(bool arg1);

    void on_actionLeft_To_Right_toggled(bool arg1);

private:
    void setImageFitType();
    void onCurrentPageChanged();
private:
    Ui::MainWindow *ui;
    ImageWidget *mImageWidget;
    PagesNavigator *mPagesNavigator;
};
#endif // MAINWINDOW_H
