#ifndef FLOW_TREE_WIDGET_H_
#define FLOW_TREE_WIDGET_H_

#include <QTreeWidget>

class JZFlowTreeWidget : public QWidget
{
    Q_OBJECT

public:
    JZFlowTreeWidget(QWidget* parent = nullptr);
    ~JZFlowTreeWidget();

    
protected slots:
    void onContexMenu(QPoint pt);

protected:
    
private:
    QTreeWidget* m_tree;
};


#endif // ! FLOW_TREE_WIDGET_H_
