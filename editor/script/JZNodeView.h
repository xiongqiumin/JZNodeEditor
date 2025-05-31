#ifndef JZNODE_VIEW_H_
#define JZNODE_VIEW_H_

#include <QGraphicsView>
#include <functional>
#include <QMap>
#include <QTextEdit>
#include <QShortcut>
#include <QUndoStack>
#include <QGraphicsRectItem>
#include "JZNodeLineItem.h"
#include "JZNodeGroupItem.h"
#include "JZNodePropertyEditor.h"
#include "JZNodeAutoRunWidget.h"
#include "JZScriptItem.h"
#include "JZNodeProgram.h"
#include "JZNodeViewMap.h"
#include "JZNodeDebugPacket.h"
#include "JZProcess.h"
#include "JZNodeViewCommand.h"
#include "JZNodeAbstractView.h"

class JZNodePanel;
class JZNodeFlowPanel;
class JZProject;
class JZNodeGraphItem;

//JZNodeView
class JZNodeView : public JZNodeAbstractView
{
    Q_OBJECT

public:
    JZNodeView(QWidget *widget = nullptr);
    virtual ~JZNodeView();

    void setPropertyEditor(JZNodePropertyEditor *propEditor);
    void setRunEditor(JZNodeAutoRunWidget *runEditor);
    
    void setPanel(JZNodePanel *panel);
    void setFlowPanel(JZNodeFlowPanel *panel);
              
    void editPinValue(int node_id, int pin_id);
    void editFinish();
    
    void updateNodeLayout();        
    void breakPointTrigger();

    ProcessStatus runningMode();
    void setRunningMode(ProcessStatus mode);    

    int runtimeNode();
    void setRuntimeNode(int nodeId);
    void clearRuntimeValue();
    void setRuntimeValue(int node_id,int pin_id,const JZNodeDebugParamValue &value);

    void clearDisplayValue();
    void displayValue(int node_id,int pin_id,QVariantPtr *ptr);

    bool isBreakPoint(int nodeId);    

    QList<int> watchList();

signals:    
    void sigFunctionOpen(QString name);
    void sigRuntimeValueChanged(int id,QString value);

public slots:    
    void onNodePinValueChanged(int nodeId, int pinId, const QString &value);        

protected slots:
    void onContextMenu(const QPoint &pos);
        
    void onEditWidgetTimer();  //监控焦点
    void onEditFinish();    

protected:            
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dragMoveEvent(QDragMoveEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;    

    JZAbstractNodeItem *createNodeItem(JZNode *node);
    JZAbstractLineItem *createLineItem(JZNodeGemo gemo);

    void udpateFlowPanel();
    int propEditorNodeId();
             
    JZNodePanel *m_panel;
    JZNodeFlowPanel *m_flowPanel;
    JZNodePropertyEditor *m_propEditor;
    JZNodeAutoRunWidget *m_runEditor;

    bool m_isUpdateFlowPanel;    
                    
    ProcessStatus m_runningMode;    
    int m_runNode;

    QTimer *m_editWidgetTimer;
    QGraphicsProxyWidget *m_editProxy;    
};

#endif
