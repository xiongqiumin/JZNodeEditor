#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QWidget>
#include <QStackedWidget>
#include "modules/vision/JZVisionWidget.h"
#include "modules/communication/JZCommManager.h"
#include "modules/camera/JZCameraManager.h"
#include "modules/model/JZModelManager.h"
#include "flowTreeWidget.h"
#include "JZPanelWidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected slots:
    void onBtnCamera();
    void onBtnFlow();
    void onBtnModel();
    void onBtnComm();
    void onBtnSetting();

    void onActionNewProject();
    void onActionOpenProject();
    void onActionHelp();

private:
    void initUi();    
    QMenuBar *createMenuBar();
    QWidget *createTitleBar();
    QIcon icon(QString name);

    void setSliderStyle(QWidget *w);
    void addCameraPage();
    void addFlowPage();
    void addModelPage();
    void addCommPage();
    void addSettingPage();

    QStackedWidget *m_stack;    
    JZCameraListWidget *m_list;
    JZCameraViewWidget *m_view;
    JZFlowTreeWidget *m_flow;

    JZModelManager* m_modelManager;
    JZCameraManager* m_cameraManager;
    JZCommManager* m_commManager;
};

#endif // MAINWINDOW_H    