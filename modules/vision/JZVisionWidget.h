#ifndef JZ_VISON_WIDGET_H_
#define JZ_VISON_WIDGET_H_

#include <QWidget>
#include <QTreeWidget>
#include <opencv2/opencv.hpp>
#include "modules/camera/JZCameraManager.h"
#include "jzWidgets/JZImageLabel.h"

//JZCameraListWidget
class JZCameraViewWidget;
class JZCameraListWidget : public QWidget 
{
    Q_OBJECT

public:
    JZCameraListWidget(QWidget* parent = nullptr);
    ~JZCameraListWidget();

    void setCameraManager(JZCameraManager *cameraManager);
    void setViewWidget(JZCameraViewWidget *view);
    void updateCamera();
    void settingCamera(QString name);

protected slots:
    void onContexMenu(QPoint pt);

protected:
    void onFrameReady(cv::Mat mat);

private:
    QTreeWidget* m_tree;

    JZCameraViewWidget* m_view;
    JZCameraManager* m_cameraManager;
};

//JZCameraViewWidget
class JZCameraViewWidget : public QWidget
{
    Q_OBJECT

public:
    enum
    {

    };

    JZCameraViewWidget(QWidget* parent = nullptr);
    ~JZCameraViewWidget();

    void init(JZCameraManager* cameraManager);
    JZImageLabel* label(QString name);
    
protected slots:
    void onContexMenu(QPoint pt);

protected:
    struct LabelInfo
    {
        QString name;
        int index;
        JZImageLabel* label;
    };

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
    LabelInfo* labelAt(QPoint pt);

    QList<LabelInfo> m_labelList;
    JZCameraManager* m_cameraManager;
};

#endif // ! JZ_VISON_WIDGET_H_
