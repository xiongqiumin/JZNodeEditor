#include <QVBoxLayout>
#include <QMenu>
#include <QPainter>
#include "JZCameraViewWidget.h"
#include "JZRegExpHelp.h"
#include "modules/camera/JZCameraWidget.h"
#include "modules/opencv/CvToQt.h"
#include "jzProfiler/JZTx.h"

//JZCameraEmpty
class JZCameraEmpty : public QWidget
{
public:
    JZCameraEmpty()
    {
    }

    void paintEvent(QPaintEvent* event)
    {
        QPainter painter(this);
        painter.fillRect(rect(), Qt::white);
    }
};

//JZCameraViewWidget
JZCameraViewWidget::JZCameraViewWidget(QWidget* parent)
{
	m_cameraManager = nullptr;
    m_viewId = 0;
    m_layoutType = Layout_Auto;

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &JZCameraViewWidget::customContextMenuRequested, this, &JZCameraViewWidget::onContexMenu);
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
    if (label_list.size() == 0)
        return;

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
            JZCameraEmpty *w = new JZCameraEmpty();
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