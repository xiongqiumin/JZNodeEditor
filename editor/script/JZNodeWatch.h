#ifndef JZNODE_WATCH_H_
#define JZNODE_WATCH_H_

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QTreeWidget>
#include "JZNodeDebugPacket.h"
#include "JZProcess.h"

class JZNodeWatch : public QWidget
{
    Q_OBJECT

public:
    JZNodeWatch(QWidget *parent = nullptr);
    ~JZNodeWatch();

    void setReadOnly(bool flag);
    void setRunningMode(ProcessStatus status);    
    
    void setNodeInfo(const NodeInfo &info);    
    void updateParamInfo(JZNodeGetDebugParamResp *info);    

    QStringList watchList();    
        
signals:
    void sigSetWatch(JZNodeIRParam coor,QString value);
    void sigGetWatch(JZNodeIRParam coor);
    
protected slots:   
    void onTreeWidgetItemDoubleClicked(QTreeWidgetItem * item, int column);
    void onItemChanged(QTreeWidgetItem *item, int column);

protected:       
    virtual void keyPressEvent(QKeyEvent *e) override;

    void updateStatus();
    void updateWatchItem();
    int indexOfItem(QTreeWidgetItem *root, const QString &coor,int start);
    QString coorName(const JZNodeIRParam &param);
        
    void setItem(QTreeWidgetItem *root, const JZNodeDebugParamValue &info);    

    bool m_readOnly;    
    ProcessStatus m_status;          
    QTreeWidgetItem *m_editItem;
    QTreeWidgetItem *m_nodeItem;
    NodeInfo m_nodeInfo;
    int m_editColumn;

    QTreeWidget *m_view;    
};






#endif
