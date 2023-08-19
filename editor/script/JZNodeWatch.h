#ifndef JZNODE_WATCH_H_
#define JZNODE_WATCH_H_

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QTreeWidget>
#include "JZNodeDebugPacket.h"

class JZNodeWatch : public QWidget
{
    Q_OBJECT

public:
    JZNodeWatch(QWidget *parent = nullptr);
    ~JZNodeWatch();

    void setRunning(bool isRun);
    void setRuntimeStatus(int status);
    void setParamInfo(JZNodeDebugParamInfo *info,bool isNew);
    void clear();
        
signals:
    void sigParamChanged(JZNodeParamCoor coor,QVariant value);
    
protected slots:   
    void onTreeWidgetItemDoubleClicked(QTreeWidgetItem * item, int column);
    void onItemChanged(QTreeWidgetItem *item, int column);

protected:       
    void updateStatus();    
    int indexOfItem(QTreeWidgetItem *root, const QString &name);
    void setItem(QTreeWidgetItem *root,int index,const QString &name,const JZNodeDebugParamValue &info);    
    JZNodeDebugParamValue getParamValue(QTreeWidgetItem *item);

    bool m_running;
    int m_status;    
    bool m_newParam;
    
    QTreeWidget *m_view;
};






#endif
