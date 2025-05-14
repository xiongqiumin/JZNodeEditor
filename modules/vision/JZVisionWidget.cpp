#include <QVBoxLayout>
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
}

JZCameraViewWidget::~JZCameraViewWidget()
{
}

void JZCameraViewWidget::init()
{
	for (int i = 0; i < m_labelList.size(); i++)
		delete m_labelList[i].label;

	m_labelList.clear();
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

void JZCameraViewWidget::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);

	if (m_labelList.size() == 1)
	{
		m_labelList[0].label->setGeometry(0, 0, width(), height());
	}
	else
	{

	}
}