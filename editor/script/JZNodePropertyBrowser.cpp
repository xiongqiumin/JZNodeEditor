#include <QApplication>
#include <QPainter>
#include <QStyle>
#include "JZNodePropertyBrowser.h"
#include "JZNodeType.h"

JZNodeProperty::JZNodeProperty(QString name, int type)
{
    m_item = nullptr;
    m_enabled = true;
    m_name = name;
    m_type = type;
    m_parent = nullptr;
}

void JZNodeProperty::setEnabled(bool flag)
{
    m_enabled = flag;
    if (m_item) {
        auto tree = qobject_cast<JZNodePropertyBrowser*>(m_item->treeWidget());
        tree->setItemEnabled(m_item, flag);
    }    
}

void JZNodeProperty::addSubProperty(JZNodeProperty *prop)
{
    Q_ASSERT(!prop->parent());
    prop->m_parent = this;
    m_childs.push_back(QSharedPointer<JZNodeProperty>(prop));
    if (m_item)
    {
        auto tree = qobject_cast<JZNodePropertyBrowser*>(m_item->treeWidget());
        tree->createPropItem(m_item, prop);
    }
}

JZNodeProperty *JZNodeProperty::parent()
{
    return m_parent;
}

const QString &JZNodeProperty::value() const
{
    return m_value;
}

void JZNodeProperty::setValue(const QString &value)
{
    m_value = value;
    if(m_item)
        m_item->setText(1,value);
}

//JZNodePropertyBrowser
JZNodePropertyBrowser::JZNodePropertyBrowser()
    :m_root("root", Type_none)
{
    this->setColumnCount(2);
    this->setHeaderLabels({ "name","value" });
    this->setAlternatingRowColors(true);
    connect(this, &QTreeWidget::itemChanged, this, &JZNodePropertyBrowser::onItemChanged);
    this->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
}

JZNodePropertyBrowser::~JZNodePropertyBrowser()
{

}


void JZNodePropertyBrowser::onItemChanged(QTreeWidgetItem *item, int column)
{
    auto text = item->text(column);
    auto prop = m_propMap[item];
    emit valueChanged(prop, text);
}

void JZNodePropertyBrowser::clear()
{
    QTreeWidget::clear();
    m_root.m_childs.clear();
    m_propMap.clear();
}

void JZNodePropertyBrowser::addProperty(JZNodeProperty *prop)
{    
    createPropItem(this->invisibleRootItem(), prop);    
}

void JZNodePropertyBrowser::setItemEnabled(QTreeWidgetItem *item, bool flag)
{
    this->blockSignals(true);
    if (flag)
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    else
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    this->blockSignals(false);
}

void JZNodePropertyBrowser::createPropItem(QTreeWidgetItem *parent, JZNodeProperty *prop)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, prop->m_name);
    item->setText(1, prop->m_value);
    if (prop->m_enabled)
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    else
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    prop->m_item = item;
    parent->addChild(item);
    m_propMap[item] = prop;

    for (int i = 0; i < prop->m_childs.size(); i++)    
        createPropItem(prop->m_item, prop->m_childs[i].data());    
    prop->m_item->setExpanded(true);
}

void JZNodePropertyBrowser::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{    
    QTreeWidgetItem *item = this->itemFromIndex(index);

    QStyleOptionViewItem opt = option;  
    bool hasValue = (m_propMap[item]->m_type != NodeProprety_GroupId);
    if (!hasValue) {
        const QColor c = option.palette.color(QPalette::Dark);
        painter->fillRect(option.rect, c);
        opt.palette.setColor(QPalette::AlternateBase, c);
    }
    else {
        const QColor c = QColor();
        if (c.isValid()) {
            painter->fillRect(option.rect, c);
            opt.palette.setColor(QPalette::AlternateBase, c.lighter(112));
        }
    }    
    QTreeWidget::drawRow(painter, opt, index);
    QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt));
    painter->save();
    painter->setPen(QPen(color));
    painter->drawLine(opt.rect.x(), opt.rect.bottom(), opt.rect.right(), opt.rect.bottom());
    painter->restore();    
}