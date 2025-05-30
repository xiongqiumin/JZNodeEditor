#include <QSplitter>
#include <QHBoxLayout>
#include "JZVisionEditor.h"

JZVisionEditor::JZVisionEditor(QWidget *parent) 
    : JZEditor(parent)
{
    initLayout();
}

JZVisionEditor::~JZVisionEditor()
{
}

void JZVisionEditor::initLayout()
{
    // 创建组件
    m_leftPanel = new JZVisionPanel(this);
    m_centerView = new JZVisionView(this);
    m_rightImage = new JZVisionImage(this);
    m_rightOutput = new JZVisionOutput(this);

    QSplitter *rightSplitter = new QSplitter(Qt::Vertical, this);
    rightSplitter->addWidget(m_rightImage);
    rightSplitter->addWidget(m_rightOutput);

    rightSplitter->setChildrenCollapsible(false);

    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->addWidget(m_leftPanel);
    mainSplitter->addWidget(m_centerView);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setChildrenCollapsible(false);
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(mainSplitter);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}