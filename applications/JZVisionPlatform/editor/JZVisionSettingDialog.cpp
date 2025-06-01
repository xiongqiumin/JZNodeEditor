#include <QTabWidget>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include "JZVisionLinkDialog.h"
#include "JZVisionSettingDialog.h"
#include "JZScriptItem.h"

//Block
bool JZVisionSettingDialog::Block::isLink()
{
    return !linkGemo.isNull();
}

//JZVisionSettingDialog
JZVisionSettingDialog::JZVisionSettingDialog(QWidget *parent) 
    : JZBaseDialog(parent)
{    
    m_node = nullptr;
}

JZVisionSettingDialog::~JZVisionSettingDialog()
{
}

QMap<int, JZVisionSettingDialog::Block> JZVisionSettingDialog::blockList()
{
    return m_blockList;
}

void JZVisionSettingDialog::setNode(JZNode* node)
{
    m_node = node;
    QWidget *area_widget = new QWidget();    
    
    QVBoxLayout *v = new QVBoxLayout();
    area_widget->setLayout(v);        

    v->addWidget(createRow("节点:", node->name()));
    auto in_list = node->paramInList();
    for (int i = 0; i < in_list.size(); i++)
    {
        QWidget *pin_widget = createPin(node->pin(in_list[i]));
        v->addWidget(pin_widget);
        updateBlock(in_list[i]);
    }
    v->addStretch();

    QScrollArea *area = new QScrollArea();
    area->setWidgetResizable(true);
    area->setWidget(area_widget);

    QTabWidget *tab = new QTabWidget();
    tab->addTab(area, "基本参数");
    setCentralWidget(tab);

    resize(480, 600);
}

QWidget *JZVisionSettingDialog::createRow(QString name, QString value)
{
    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);

    QWidget *w = new QWidget();
    l->addWidget(new QLabel(name));
    l->addWidget(new QLabel(value));
    w->setLayout(l);

    return w;
}

QWidget *JZVisionSettingDialog::createPin(JZNodePin *pin)
{
    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    
    QWidget *w = new QWidget();
    l->addWidget(new QLabel(pin->name()));

    auto lineEdit = new QLineEdit();
    l->addWidget(lineEdit);

    QWidget *link = nullptr;
    if (pin->isConstValue())
        link = new QWidget();
    else
    {
        auto btn_link = new QToolButton();
        btn_link->setProperty("PinId", pin->id());

        link = btn_link;
        connect(btn_link, &QToolButton::clicked, this, &JZVisionSettingDialog::onBtnLink);
    }

    QList<int> in_list = m_node->file()->getConnectInput(m_node->id(), pin->id());

    Block block;
    if (in_list.size() > 0)
        block.linkGemo = m_node->file()->getConnect(in_list[0])->from;
    block.line = lineEdit;
    block.pinId = pin->id();
    m_blockList.insert(block.pinId, block);
    
    link->setFixedWidth(30);
    l->addWidget(link);
    w->setLayout(l);
    return w;
}

void JZVisionSettingDialog::onBtnLink()
{
    int pinId = sender()->property("PinId").toInt();
    if (m_blockList[pinId].isLink())
    {
        m_blockList[pinId].linkGemo = JZNodeGemo();
    }
    else
    {
        JZVisionLinkDialog dlg(this);
        dlg.setNode(m_node, pinId);
        if (dlg.exec() != JZVisionLinkDialog::Accepted)
            return;

        m_blockList[pinId].linkGemo = dlg.result();
    }
    updateBlock(pinId);
}

void JZVisionSettingDialog::accept()
{
    JZBaseDialog::accept();
}

void JZVisionSettingDialog::updateBlock(int id)
{
    auto b = m_blockList[id];
    if (b.isLink())
    {
        b.line->setReadOnly(true);
        b.line->setText("已连接");
    }
    else
    {
        b.line->setText(m_node->pinValue(id));
    }
}