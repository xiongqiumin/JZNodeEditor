#ifndef JZNODE_LIST_BIND_H_
#define JZNODE_LIST_BIND_H_

#include "JZNodeVariableBind.h"

class ListWidgetBind;
class QListWidget;

//ListDelegate
class ListDelegate
{
public:
    ListDelegate();
    ~ListDelegate();

    void set(int idx, const QVariant &v);
    QVariant get(int idx);
    int size();
    void clear();
    void insert(int index, const QVariant& t);
    void push_back(int index, const QVariant & t);
    void pop_back(const QVariant& t);
    void push_front(const QVariant & t);
    void pop_front();
    void removeAt(int idx);

    QVariant mid(int idx,int len);
    void append(int idx,const QVariant &v);
    void resize(int size);
    void swap(int i,int j);

protected:
    ListWidgetBind* m_bind;
    QListWidget* m_listWidget;
    JZNodeObject* m_list;
};  

//ListWidgetBind
class ListWidgetBind : public JZBindObject
{
    Q_OBJECT

public:
    ListWidgetBind();
    virtual ~ListWidgetBind();

    QListWidget* listWidget();

protected slots:
    virtual void bind(QWidget* widget, JZNodeObject* object, QString prop) override;
    virtual void uiToDataImpl() override;
    virtual void dataToUiImpl() override;
};

#endif // !JZNODE_LIST_BIND_H_
