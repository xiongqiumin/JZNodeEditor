#include <QVBoxLayout>
#include <QMenu>
#include <QPainter>
#include "JZCameraListWidget.h"
#include "JZRegExpHelp.h"
#include "modules/camera/JZCameraWidget.h"
#include "modules/opencv/CvToQt.h"
#include "jzProfiler/JZTx.h"
#include "JZCameraViewWidget.h"
#include "mainwindow.h"

//JZCameraListWidget
JZCameraListWidget::JZCameraListWidget(QWidget* parent)
{
	m_view = nullptr;
    m_window = nullptr;

	QVBoxLayout* l = new QVBoxLayout();
	l->setContentsMargins(0, 0, 0, 0);

	m_tree = new QTreeWidget();
	m_tree->setColumnCount(1);
	m_tree->setHeaderHidden(true);
	m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_tree, &QTreeWidget::customContextMenuRequested, this, &JZCameraListWidget::onContexMenu);

    connect(m_tree, &QTreeWidget::itemDoubleClicked, this, &JZCameraListWidget::onItemDoubleClicked);

    m_tree->setSelectionMode(QTreeWidget::SingleSelection);

	l->addWidget(m_tree);
	setLayout(l);    
}

JZCameraListWidget::~JZCameraListWidget()
{
}

void JZCameraListWidget::setMainWindow(MainWindow *mainwindow)
{
    m_window = mainwindow;
    m_cameraManager = m_window->cameraManager();
	
	m_tree->clear();
	updateCamera();
}

void JZCameraListWidget::updateCamera()
{    
	m_tree->clear(); 
    m_view->clear();
    m_root = new QTreeWidgetItem();
    m_root->setText(0, "所有设备");
    m_tree->addTopLevelItem(m_root);

    auto list = m_cameraManager->config().cameraList;
	for (int i = 0; i < list.size(); i++)
	{
        addCameraItem(list[i]->name);        
	}

    m_tree->expandAll();
}

void JZCameraListWidget::setViewWidget(JZCameraViewWidget* view)
{
	m_view = view;
}

QTreeWidgetItem *JZCameraListWidget::cameraItem(QString name)
{
    for (int i = 0; i < m_root->childCount(); i++)
    {
        auto item = m_root->child(i);
        if (item->text(0) == name)
            return item;
    }
    return nullptr;
}

QTreeWidgetItem *JZCameraListWidget::addCameraItem(QString name)
{    
    QTreeWidgetItem* new_item = new QTreeWidgetItem();
    new_item->setText(0, name);
    m_root->addChild(new_item);

    m_view->addCamera(name);
    return new_item;
}

void JZCameraListWidget::onItemDoubleClicked(QTreeWidgetItem *item)
{
    if (item == m_root)
        return;

    settingCamera(item->text(0));
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
        actNew = menu.addAction("新建");        
    }
    else
    {        
        if (item != m_root)        
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
        
        JZCameraFileConfig *cfg = new JZCameraFileConfig();
                
        JZCameraConfigDialog dlg(this);
        dlg.setConfig(JZCameraConfigEnum(cfg));
        dlg.makeUniqueName("camera", camera_list);
        if (dlg.exec() != JZCameraConfigDialog::Accepted)
            return;

        auto result = dlg.getConfig();        
        m_cameraManager->addCamera(result);
        QTreeWidgetItem* new_item = addCameraItem(result->name);        

        m_tree->clearSelection();
        m_tree->setItemSelected(new_item, true);
        m_tree->scrollToItem(new_item);

        emit sigCameraChanged();
    }
    else if (act == actOpen)
    {        
        m_window->startCamera(camera_name);
	}
    else if (act == actClose)
	{
        m_window->stopCamera(camera_name);
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
        emit sigCameraChanged();
    }
}

void JZCameraListWidget::settingCamera(QString name)
{
    auto &config = m_cameraManager->config();
    int cam_idx = config.indexOfCamera(name);

    QString old_name = name;
    JZCameraConfigDialog dlg(this);
    dlg.setConfig(config.cameraList[cam_idx]);
    if (dlg.exec() != JZCameraConfigDialog::Accepted)
        return;

    JZCameraConfigEnum new_cfg = dlg.getConfig();
    if (old_name != new_cfg->name)
    {
        if (m_cameraManager->camera(new_cfg->name))
        {
            QMessageBox::information(this, "", "名称已存在");
            return;
        }

        QTreeWidgetItem *item = cameraItem(name);
        item->setText(0, name);
    }

	m_cameraManager->setCamera(name, dlg.getConfig());
    emit sigCameraChanged();
}