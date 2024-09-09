#ifndef JZNODE_WATCH_H_
#define JZNODE_WATCH_H_

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QTreeWidget>
#include "JZNodeDebugPacket.h"
#include "JZProcess.h"

class MainWindow;
class JZNodeWatch : public QWidget
{
    Q_OBJECT

public:
    JZNodeWatch(QWidget *parent = nullptr);
    ~JZNodeWatch();

    void setReadOnly(bool flag);
    void setRunningMode(ProcessStatus status);
    void setMainWindow(MainWindow *w);
    
    void setParamInfo(JZNodeGetDebugParamResp *info);
    void updateParamInfo(JZNodeGetDebugParamResp *info);
    QStringList watchList();

    void clear();
        
signals:
    void sigParamValueChanged(JZNodeIRParam coor,QString value);
    void sigParamNameChanged(JZNodeIRParam coor);
    
protected slots:   
    void onTreeWidgetItemDoubleClicked(QTreeWidgetItem * item, int column);
    void onItemChanged(QTreeWidgetItem *item, int column);

protected:       
    virtual void keyPressEvent(QKeyEvent *e) override;

    void updateStatus();
    void updateWatchItem();
    int indexOfItem(QTreeWidgetItem *root, const QString &name,int start);
    QString coorName(const JZNodeIRParam &param);

    void setItem(QTreeWidgetItem *root, int index,const JZNodeIRParam &coor, const JZNodeDebugParamValue &info);
    QTreeWidgetItem *updateItem(QTreeWidgetItem *root,int index,const QString &name,const JZNodeDebugParamValue &info);
    
    JZNodeDebugParamValue getParamValue(QTreeWidgetItem *item);    

    bool m_readOnly;    
    ProcessStatus m_status;          
    QTreeWidgetItem *m_editItem;
    int m_editColumn;

    QTreeWidget *m_view;    
    MainWindow *m_mainWindow;
};






#endif
