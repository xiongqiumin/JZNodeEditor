#ifndef JZNODE_PROPERTY_BROWSER_H_
#define JZNODE_PROPERTY_BROWSER_H_

#include <QTreeWidget>
#include <QSharedPointer>

enum NodePropretyType{
    NodeProprety_GroupId,
    NodeProprety_NodeId,
    NodeProprety_Value,        
    NodeProprety_Count,
};

class JZNodePropertyBrowser;
class JZNodeProperty
{
public:
    JZNodeProperty(QString name, NodePropretyType prop_type);

    NodePropretyType type() const;

    void setEnabled(bool flag);
    bool isEnabled() const;

    void addSubProperty(JZNodeProperty *prop);    

    void setName(const QString &value);
    const QString &name() const;
    
    void setDataType(int data_type);
    int dataType();    

    void setValue(const QString &value);
    const QString &value() const;

    JZNodeProperty *parent();

protected:
    friend JZNodePropertyBrowser;    

    QString m_name;
    NodePropretyType m_type;
    QString m_value;
    bool m_enabled;
    QTreeWidgetItem *m_item;    
    JZNodeProperty *m_parent;
    QList<QSharedPointer<JZNodeProperty>> m_childs;
    int m_dataType;
};

class JZNodePropertyBrowser : public QTreeWidget
{
    Q_OBJECT

public:
    JZNodePropertyBrowser();
    ~JZNodePropertyBrowser();

    void addProperty(JZNodeProperty *prop);
    JZNodeProperty *property(const QModelIndex &index);

    void clear();

signals:
    void valueChanged(JZNodeProperty *prop,const QString &value);

protected:
    friend JZNodeProperty;

    void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void createPropItem(QTreeWidgetItem *parent, JZNodeProperty *prop);
    void setItemEnabled(QTreeWidgetItem *item,bool flag);
    void updateItem(QTreeWidgetItem *item);

    JZNodeProperty m_root;
    QMap<QTreeWidgetItem*, JZNodeProperty*> m_propMap;
};


#endif // !JZNODE_PROPERTY_BROWSER_H_
