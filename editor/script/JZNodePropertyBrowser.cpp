#include <QApplication>
#include <QPainter>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QDebug>
#include <QFocusEvent>
#include <QFileInfo>
#include "JZNodePropertyBrowser.h"
#include "JZNodePinWidget.h"
#include "JZNodeType.h"
#include "JZNodeObject.h"

static QIcon drawCheckBox(bool value)
{
    QStyleOptionButton opt;
    opt.state |= value ? QStyle::State_On : QStyle::State_Off;
    opt.state |= QStyle::State_Enabled;
    const QStyle *style = QApplication::style();
    // Figure out size of an indicator and make sure it is not scaled down in a list view item
    // by making the pixmap as big as a list view icon and centering the indicator in it.
    // (if it is smaller, it can't be helped)
    const int indicatorWidth = style->pixelMetric(QStyle::PM_IndicatorWidth, &opt);
    const int indicatorHeight = style->pixelMetric(QStyle::PM_IndicatorHeight, &opt);
    const int listViewIconSize = indicatorWidth;
    const int pixmapWidth = indicatorWidth;
    const int pixmapHeight = qMax(indicatorHeight, listViewIconSize);

    opt.rect = QRect(0, 0, indicatorWidth, indicatorHeight);
    QPixmap pixmap = QPixmap(pixmapWidth, pixmapHeight);
    pixmap.fill(Qt::transparent);
    {
        // Center?
        const int xoff = (pixmapWidth  > indicatorWidth) ? (pixmapWidth - indicatorWidth) / 2 : 0;
        const int yoff = (pixmapHeight > indicatorHeight) ? (pixmapHeight - indicatorHeight) / 2 : 0;
        QPainter painter(&pixmap);
        painter.translate(xoff, yoff);
        style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, &painter);
    }
    return QIcon(pixmap);
}

JZNodeProperty::JZNodeProperty(QString name, NodePropretyType type)
{    
    m_item = nullptr;
    m_enabled = true;
    m_name = name;
    m_type = type;
    m_dataType = Type_none;
    m_parent = nullptr;
}

NodePropretyType JZNodeProperty::type() const
{
    return m_type;
}

bool JZNodeProperty::isEnabled() const
{
    return m_enabled;
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

void JZNodeProperty::setName(const QString &value)
{
    m_name = value;     
    if(m_item)
    {
        auto tree = qobject_cast<JZNodePropertyBrowser*>(m_item->treeWidget());
        m_item->setText(0,value);        
    }   
}

const QString &JZNodeProperty::name() const
{
    return m_name;
}

void JZNodeProperty::setDataType(int data_type)
{
    m_dataType = data_type;
    if (m_item)
    {
        auto tree = qobject_cast<JZNodePropertyBrowser*>(m_item->treeWidget());
        tree->updateItem(m_item);
    }
}

int JZNodeProperty::dataType()
{
    return m_dataType;
}

void JZNodeProperty::setValue(const QString &value)
{
    m_value = value;
    if(m_item)
    {
        auto tree = qobject_cast<JZNodePropertyBrowser*>(m_item->treeWidget());        
        tree->updateItem(m_item);
    }
}

class PinValueItemDelegate : public QStyledItemDelegate
{
public:
    PinValueItemDelegate(QObject *parent)
        :QStyledItemDelegate(parent)
    {
    }

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        if (index.column() == 0)
            return nullptr;

        auto prop = browser->property(index);
        if (prop->type() != NodeProprety_Value || prop->dataType() == Type_none)
            return nullptr;
                
        auto edit = new JZNodeParamValueWidget();
        edit->setParent(parent);
        edit->initWidget(prop->dataType());
        edit->setValue(prop->value());
        edit->setAutoFillBackground(true);
        edit->setProperty("isEditor",true);

        ItemFocusEventFilter *filter = new ItemFocusEventFilter(edit);
        edit->focusWidget()->installEventFilter(filter);
        return edit;
    }

    void setModelData(QWidget *editor,
        QAbstractItemModel *model,
        const QModelIndex &index) const override
    {
        auto edit = qobject_cast<JZNodeParamValueWidget*>(editor);
        edit->deleteLater();

        auto prop = browser->property(index);
        if (prop->value() != edit->value())
        {
            prop->setValue(edit->value());
            emit browser->valueChanged(prop, edit->value());
        }
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const
    {
        QStyledItemDelegate::paint(painter, option, index);

        QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &option));
        painter->save();
        painter->setPen(QPen(color));
        if (index.column() == 0) {
            int right = (option.direction == Qt::LeftToRight) ? option.rect.right() : option.rect.left();
            painter->drawLine(right, option.rect.y(), right, option.rect.bottom());
        }
        painter->restore();
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
        const QModelIndex &index) const
    {
        editor->setGeometry(option.rect.adjusted(0, 0, 0, -1));
    }

    JZNodePropertyBrowser *browser;    
};

//JZNodePropertyBrowser
JZNodePropertyBrowser::JZNodePropertyBrowser()
    :m_root("root", NodeProprety_GroupId)
{    
    this->setColumnCount(2);
    this->setHeaderLabels({ "name","value" });
    this->setAlternatingRowColors(true);
    this->header()->setSectionsMovable(false);
    this->header()->setSectionResizeMode(QHeaderView::Stretch);        
    this->setEditTriggers(QAbstractItemView::AllEditTriggers);
    
    PinValueItemDelegate *delegate = new PinValueItemDelegate(this);
    delegate->browser = this;
    this->setItemDelegate(delegate);
}

JZNodePropertyBrowser::~JZNodePropertyBrowser()
{

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

JZNodeProperty *JZNodePropertyBrowser::property(const QModelIndex &index)
{
    auto item = itemFromIndex(index);
    return m_propMap.value(item, nullptr);
}

void JZNodePropertyBrowser::setItemEnabled(QTreeWidgetItem *item, bool flag)
{    
    if (flag)
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    else
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);    
}

void JZNodePropertyBrowser::createPropItem(QTreeWidgetItem *parent, JZNodeProperty *prop)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, prop->m_name);    
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
    updateItem(prop->m_item);
}

void JZNodePropertyBrowser::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{    
    QTreeWidgetItem *item = this->itemFromIndex(index);

    QStyleOptionViewItem opt = option;  
    /*
    bool hasValue = (m_propMap[item]->m_type != NodeProprety_GroupId);
    if (!hasValue) {
        const QColor c = option.palette.color(QPalette::Dark);
        painter->fillRect(option.rect, c);
        opt.palette.setColor(QPalette::AlternateBase, c);
    }
    else {        
        QColor c;
        if (c.isValid()) {
            painter->fillRect(option.rect, c);
            opt.palette.setColor(QPalette::AlternateBase, c.lighter(112));
        }
    }
    */
    QTreeWidget::drawRow(painter, opt, index);
    QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt));
    painter->save();
    painter->setPen(QPen(color));
    painter->drawLine(opt.rect.x(), opt.rect.bottom(), opt.rect.right(), opt.rect.bottom());
    painter->restore();    
}

void JZNodePropertyBrowser::updateItem(QTreeWidgetItem *item)
{
    int type = m_propMap[item]->dataType();

    QIcon icon;
    QString value = m_propMap[item]->value();
    if (type == Type_boolCheck)
    {
        bool flag = (value == "true");
        icon = drawCheckBox(flag);
        if (item->text(1).isEmpty())
            item->setText(1, "false");
    }
    else if (type == Type_imageEdit)
    {
        QString path = value;
        if (!path.isEmpty())
        {
            QPixmap map(path);
            map = map.scaled(QSize(16, 16), Qt::KeepAspectRatio);

            QString file = QFileInfo(path).fileName();
            item->setText(1, file);
            icon = QIcon(map);
        }
    }
    else
    {
        item->setText(1,value);
    }
    item->setIcon(1, icon);
}