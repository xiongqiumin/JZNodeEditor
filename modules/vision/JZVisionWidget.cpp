#include <QVBoxLayout>
#include <QMenu>
#include <QPainter>
#include "JZVisionWidget.h"
#include "JZRegExpHelp.h"
#include "modules/camera/JZCameraWidget.h"
#include "modules/opencv/CvToQt.h"
#include "jzProfiler/JZTx.h"

//JZCameraListWidget
JZCameraListWidget::JZCameraListWidget(QWidget* parent)
{
	m_view = nullptr;
	m_cameraManager = nullptr;

	QVBoxLayout* l = new QVBoxLayout();
	l->setContentsMargins(0, 0, 0, 0);

	m_tree = new QTreeWidget();
	m_tree->setColumnCount(1);
	m_tree->setHeaderHidden(true);
	m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_tree, &JZCameraListWidget::customContextMenuRequested, this, &JZCameraListWidget::onContexMenu);

    m_tree->setSelectionMode(QTreeWidget::SingleSelection);

	l->addWidget(m_tree);
	setLayout(l);    
}

JZCameraListWidget::~JZCameraListWidget()
{
}

void JZCameraListWidget::setCameraManager(JZCameraManager* cameraManager)
{
	m_cameraManager = cameraManager;
	m_tree->clear();
	updateCamera();
}

void JZCameraListWidget::updateCamera()
{
	m_tree->clear(); 
    m_root = new QTreeWidgetItem();
    m_root->setText(0, "所有设备");
    m_tree->addTopLevelItem(m_root);

    auto list = m_cameraManager->config().cameraList;
	for (int i = 0; i < list.size(); i++)
	{
        addCameraItem(list[i]->name);        
	}
}

void JZCameraListWidget::setViewWidget(JZCameraViewWidget* view)
{
	m_view = view;
}

QTreeWidgetItem *JZCameraListWidget::addCameraItem(QString name)
{
    JZCamera *camera = m_cameraManager->camera(name);
    connect(camera, &JZCamera::sigFrameReady, this, &JZCameraListWidget::onFrameReady);

    QTreeWidgetItem* new_item = new QTreeWidgetItem();
    new_item->setText(0, name);
    m_root->addChild(new_item);

    m_view->addCamera(name);
    return new_item;
}

void JZCameraListWidget::onContexMenu(QPoint pt)
{
	auto item = m_tree->itemAt(pt);    	
	QMenu menu(this);
    QAction *actNew = nullptr, *actOpen = nullptr, *actClose = nullptr, *actSetting = nullptr;
    QAction *actOpenAll = nullptr;
    QAction *actStopAll = nullptr;
    QAction *actDel = nullptr;
    QString camera_name;
    JZCamera *camera = nullptr;
    if (!item)
    {
        actNew = menu.addAction("添加");        
    }
    else
    {        
        if (item == m_root)
        {
            actOpenAll = menu.addAction("打开全部");
            actStopAll = menu.addAction("关闭全部");
        }
        else
        {
            camera_name = item->text(0);
            camera = m_cameraManager->camera(camera_name);
            actOpen = menu.addAction("打开");
            actClose = menu.addAction("关闭");
            actSetting = menu.addAction("设置");
            actDel = menu.addAction("删除");
        }
    }

	QAction* act = menu.exec(m_tree->mapToGlobal(pt));
	if (!act)
		return;

    if (act == actNew)
    {
        auto &config = m_cameraManager->config();
        QStringList camera_list;
        for (int i = 0; i < config.cameraList.size(); i++)
            camera_list << config.cameraList[i]->name;

        JZCameraRtspConfig *cfg = new JZCameraRtspConfig();
        cfg->name = JZRegExpHelp::uniqueString("camera", camera_list);
        cfg->path = "rtsp://admin:123456HK@192.168.0.164:554/Streaming/Channels/101";

        JZCameraConfigDialog dlg(this);
        dlg.setConfig(JZCameraConfigPtr(cfg));
        if (dlg.exec() != JZCameraConfigDialog::Accepted)
            return;

        auto result = dlg.getConfig();
        m_cameraManager->addCamera(result);

        QTreeWidgetItem* new_item = addCameraItem(result->name);        

        m_tree->clearSelection();
        m_tree->setItemSelected(new_item, true);
        m_tree->scrollToItem(new_item);
    }
    else if (act == actOpen)
    {        
        startCamera(camera);
	}
    else if (act == actOpenAll)
    {
        auto camera_list = m_cameraManager->cameraList();
        for (int i = 0; i < camera_list.size(); i++)
            startCamera(camera_list[i]);
    }
    else if (act == actStopAll)
    {
        auto camera_list = m_cameraManager->cameraList();
        for (int i = 0; i < camera_list.size(); i++)
            camera_list[i]->stop();
    }
	else if (act == actClose)
	{
        camera->close();
	}
	else if (act == actSetting)
	{
		settingCamera(camera_name);
	}
    else if (act == actDel)
    {
        m_cameraManager->removeCamera(camera_name);
        m_root->removeChild(item);
        m_view->removeCamera(camera_name);
    }
}

void JZCameraListWidget::startCamera(JZCamera *camera)
{
    if (!camera->isOpen())
    {
        if (!camera->open())
            return;
    }
    camera->start();
}

void JZCameraListWidget::settingCamera(QString name)
{
    auto &config = m_cameraManager->config();
    int cam_idx = config.indexOfCamera(name);    

    JZCameraConfigDialog dlg(this);
    dlg.setConfig(config.cameraList[cam_idx]);
    if (dlg.exec() != JZCameraConfigDialog::Accepted)
        return;

	m_cameraManager->setCamera(name, dlg.getConfig());
}

void JZCameraListWidget::onFrameReady(cv::Mat mat)
{
	JZCamera *camera = qobject_cast<JZCamera*>(sender());
	QString name = camera->objectName();

    JZTX_FUNCTION
	m_view->label(name)->setImage(QtOcv::mat2Image(mat));
}

void JZCameraListWidget::onCameraError()
{

}

//JZCameraViewWidget
JZCameraViewWidget::JZCameraViewWidget(QWidget* parent)
{
	m_cameraManager = nullptr;
    m_viewId = 0;
    m_layoutType = Layout_Auto;

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &JZCameraListWidget::customContextMenuRequested, this, &JZCameraViewWidget::onContexMenu);    
}

JZCameraViewWidget::~JZCameraViewWidget()
{
}

void JZCameraViewWidget::addCamera(QString name)
{
    LabelInfo info;
    info.name = name;
    info.index = m_viewId++;
    info.label = new JZImageLabel(this);
    m_labelList.push_back(info);
    updateCamViewLayout();
}

void JZCameraViewWidget::removeCamera(QString name)
{
    int idx = indexOfLabel(name);
    delete m_labelList[idx].label;
    m_labelList.removeAt(idx);
    updateCamViewLayout();
}

JZCameraViewWidget::LabelInfo* JZCameraViewWidget::labelAt(QPoint pt)
{
	for (int i = 0; i < m_labelList.size(); i++)
	{
		auto label = m_labelList[i].label;
		if (label->isVisible() && label->geometry().contains(pt))
			return &m_labelList[i];
	}
	
	return nullptr;
}

int JZCameraViewWidget::indexOfLabel(QString name)
{
    for (int i = 0; i < m_labelList.size(); i++)
    {
        if (m_labelList[i].name == name)
            return i;
    }
    return -1;
}

JZImageLabel* JZCameraViewWidget::label(QString name)
{
	for (int i = 0; i < m_labelList.size(); i++)
	{
		if (m_labelList[i].name == name)
			return m_labelList[i].label;
	}
	
	return nullptr;
}

void JZCameraViewWidget::onContexMenu(QPoint pt)
{
    QMenu menu(this);
    
    menu.addAction("显示设置");

    QAction* act = menu.exec(this->mapToGlobal(pt));
    if (!act)
        return;

}

void JZCameraViewWidget::updateCamViewLayout()
{
	QList<JZImageLabel*> label_list;
	for (int i = 0; i < m_labelList.size(); i++)
	{
		label_list << m_labelList[i].label;
	}

    for (int i = 0; i < m_emptyWidget.size(); i++)
    {
        delete m_emptyWidget[i];
    }
    m_emptyWidget.clear();

    int layout_type = m_layoutType;
    if (layout_type == Layout_Auto)
    {
        if (label_list.size() == 1)
            layout_type = Layout_1;
        else if (label_list.size() <= 2)
            layout_type = Layout_2;
        else if (label_list.size() <= 4)
            layout_type = Layout_4;
        else if (label_list.size() <= 9)
            layout_type = Layout_9;
        else if (label_list.size() <= 16)
            layout_type = Layout_16;
    }

    int row = 1;
    int col = 1;
    int max = 1;
    if (layout_type == Layout_1)
    {
        row = 1;
        col = 1;
        max = 1;
    }
    else if (layout_type == Layout_2)
    {
        row = 1;
        col = 2;
        max = 2;
    }
    else if (layout_type == Layout_4)
    {
        row = 2;
        col = 2;
        max = 4;
    }
    else if (layout_type == Layout_9)
    {
        row = 3;
        col = 3;
        max = 9;
    }
    else if (layout_type == Layout_16)
    {
        row = 4;
        col = 4;
        max = 16;
    }

    int idx = 0;    
    if (layout())
        delete layout();
    m_layout = new QGridLayout();
    setLayout(m_layout);

    int cur_row = 0;
    int cur_col = 0;
    for (int i = 0; i < max; i++)
    {
        if(idx < m_labelList.size())
            m_layout->addWidget(label_list[idx], cur_row, cur_col);
        else
        {
            QWidget *w = new QWidget();
            m_layout->addWidget(w, cur_row, cur_col);
            m_emptyWidget << w;
        }

        idx++;
        cur_col++;
        if (cur_col == col)
        {
            cur_row++;
            cur_col = 0;
        }
    }
}

void JZCameraViewWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.fillRect(rect(), Qt::black);

    if (m_labelList.size() == 0)
    {
        painter.setPen(Qt::white);
        auto ft = painter.font();
        ft.setPixelSize(36);
        painter.setFont(ft);
        painter.drawText(rect(), "没有设备,请先在设备列表右键添加摄像头", QTextOption(Qt::AlignCenter));
    }
}