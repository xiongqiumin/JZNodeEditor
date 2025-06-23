#include "JZLineEditButton.h"

JZLineEditButton::JZLineEditButton(QWidget *parent) 
    : QWidget(parent)
{
    // 创建部件
    m_lineEdit = new QLineEdit(this);
    m_toolButton = new QToolButton(this);
    
    // 创建布局
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_lineEdit);
    layout->addWidget(m_toolButton);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    // 设置部件大小策略
    m_lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_toolButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
}

JZLineEditButton::~JZLineEditButton()
{
}

QLineEdit* JZLineEditButton::lineEdit()
{
    return m_lineEdit;
}

QToolButton *JZLineEditButton::button()
{
    return m_toolButton;
}