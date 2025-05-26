#ifndef JZ_VISON_WIDGET_H_
#define JZ_VISON_WIDGET_H_

#include <QWidget>
#include <QTreeWidget>
#include <opencv2/opencv.hpp>
#include <QGridLayout>
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

    void startCamera(JZCamera *camera);
    void settingCamera(QString name);

protected slots:
    void onContexMenu(QPoint pt);
    void onFrameReady(cv::Mat mat);
    void onCameraError();

protected:    

private:
    QTreeWidgetItem *addCameraItem(QString name);

    QTreeWidget* m_tree;

    JZCameraViewWidget* m_view;
    JZCameraManager* m_cameraManager;
    QTreeWidgetItem *m_root;
};

//JZCameraViewWidget
class JZCameraViewWidget : public QWidget
{
    Q_OBJECT

public:
    enum
    {
        Layout_Auto,
        Layout_1,
        Layout_2,        
        Layout_4,
        Layout_9,
        Layout_16,
    };

    JZCameraViewWidget(QWidget* parent = nullptr);
    ~JZCameraViewWidget();
    
    void addCamera(QString name);
    void removeCamera(QString name);
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

    virtual void paintEvent(QPaintEvent *event) override;

    void updateCamViewLayout();
    int indexOfLabel(QString name);
    LabelInfo* labelAt(QPoint pt);

    int m_layoutType;
    QList<LabelInfo> m_labelList;
    QList<QWidget*> m_emptyWidget;
    JZCameraManager* m_cameraManager;
    QGridLayout *m_layout;
    int m_viewId;
};

#endif // ! JZ_VISON_WIDGET_H_
