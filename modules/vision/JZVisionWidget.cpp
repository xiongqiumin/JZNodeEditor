#include <QVBoxLayout>
#include <QMenu>
#include <QPainter>
#include "JZVisionWidget.h"
#include "modules/opencv/CvToQt.h"

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
	connect(m_tree, &JZCameraListWidget::customContextMenuRequested, this, &JZCameraListWidget::onContexMenu);

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

	auto list = m_cameraManager->cameraList();
	for (int i = 0; i < list.size(); i++)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, list[i]);
		m_tree->addTopLevelItem(item);
	}
}

void JZCameraListWidget::setViewWidget(JZCameraViewWidget* view)
{
	m_view = view;
}

void JZCameraListWidget::onContexMenu(QPoint pt)
{
	auto item = m_tree->itemAt(pt);
	if (!item)
		return;

	QMenu menu(this);
	QAction *act = menu.exec(m_tree->mapToGlobal(pt));
	if (!act)
		return;
}

void JZCameraListWidget::onCameraStart()
{
	QString name;
	JZCamera *c = m_cameraManager->camera(name);
	c->start();
}

void JZCameraListWidget::onCameraStop()
{
	QString name;
	JZCamera* c = m_cameraManager->camera(name);
	c->stop();
}

void JZCameraListWidget::onCameraSetting()
{
	//QString name;
	//m_cameraManager->setting(name);
}

void JZCameraListWidget::onFrameReady(cv::Mat mat)
{
	JZCamera *camera = qobject_cast<JZCamera*>(sender());
	QString name = camera->objectName();
	m_view->label(name)->setImage(QtOcv::mat2Image(mat));
}

//JZCameraViewWidget
JZCameraViewWidget::JZCameraViewWidget(QWidget* parent)
{
	m_cameraManager = nullptr;

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &JZCameraListWidget::customContextMenuRequested, this, &JZCameraViewWidget::onContexMenu);
}

JZCameraViewWidget::~JZCameraViewWidget()
{
}

void JZCameraViewWidget::init(JZCameraManager* cameraManager)
{
	m_cameraManager = cameraManager;

	auto cam_list = m_cameraManager->cameraList();
	for (int i = 0; i < cam_list.size(); i++)
	{
		LabelInfo info;
		info.name = cam_list[i];
		info.index = i;
		info.label = new JZImageLabel(this);
		m_labelList.push_back(info);
	}
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

}

void JZCameraViewWidget::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);

	QList<JZImageLabel*> label_list;
	for (int i = 0; i < m_labelList.size(); i++)
	{
		if (m_labelList[i].label->isVisible())
			label_list << m_labelList[i].label;
	}

	if (label_list.size() == 1)
	{
		label_list[0]->setGeometry(0, 0, width(), height());
	}
	else
	{

	}
}

void JZCameraViewWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.fillRect(rect(), Qt::black);

}