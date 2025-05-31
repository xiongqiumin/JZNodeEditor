#ifndef JZ_CAMERA_LIST_WIDGET_H_
#define JZ_CAMERA_LIST_WIDGET_H_

#include <QWidget>
#include <QTreeWidget>
#include <opencv2/opencv.hpp>
#include <QGridLayout>
#include "modules/camera/JZCameraManager.h"
#include "jzWidgets/JZImageLabel.h"

//JZCameraListWidget
class JZCameraViewWidget;
class MainWindow;
class JZCameraListWidget : public QWidget 
{
    Q_OBJECT

public:
    JZCameraListWidget(QWidget* parent = nullptr);
    ~JZCameraListWidget();

    void setMainWindow(MainWindow *mainwindow);    
    void setViewWidget(JZCameraViewWidget *view);    
    void updateCamera();

    void settingCamera(QString name);

signals:
    void sigCameraChanged();

protected slots:
    void onContexMenu(QPoint pt);
    void onFrameReady(cv::Mat mat);
    void onCameraError();

protected:    

private:
    QTreeWidgetItem *addCameraItem(QString name);

    QTreeWidget* m_tree;

    JZCameraViewWidget* m_view;
    MainWindow* m_window;
    JZCameraManager* m_cameraManager;
    QTreeWidgetItem *m_root;
};

#endif // ! JZ_VISON_WIDGET_H_
