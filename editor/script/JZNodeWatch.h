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

    void setReadOnly(bool flag);
    void setRunning(bool isRun);
    void setRuntimeStatus(int status);
    
    void setParamInfo(JZNodeDebugParamInfo *info);
    void updateParamInfo(JZNodeDebugParamInfo *info);
    QStringList watchList();

    void clear();
        
signals:
    void sigParamValueChanged(JZNodeParamCoor coor,QString value);
    void sigParamNameChanged(JZNodeParamCoor coor);
    
protected slots:   
    void onTreeWidgetItemDoubleClicked(QTreeWidgetItem * item, int column);
    void onItemChanged(QTreeWidgetItem *item, int column);

protected:       
    virtual void keyPressEvent(QKeyEvent *e) override;

    void updateStatus();
    void updateWatchItem();
    int indexOfItem(QTreeWidgetItem *root, const QString &name);

    void setItem(QTreeWidgetItem *root, int index,const JZNodeParamCoor &coor, const JZNodeDebugParamValue &info);
    QTreeWidgetItem *updateItem(QTreeWidgetItem *root,int index,const QString &name,const JZNodeDebugParamValue &info);
    
    JZNodeDebugParamValue getParamValue(QTreeWidgetItem *item);    

    bool m_readOnly;
    bool m_running;
    int m_status;          
    QTreeWidgetItem *m_editItem;
    int m_editColumn;

    QTreeWidget *m_view;    

};






#endif
