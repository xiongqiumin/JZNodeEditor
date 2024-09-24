#ifndef JZNODE_VIEW_H_
#define JZNODE_VIEW_H_

#include <QGraphicsView>
#include <functional>
#include <QMap>
#include <QTextEdit>
#include <QShortcut>
#include <QUndoStack>
#include <QGraphicsRectItem>
#include "JZNodeScene.h"
#include "JZNodeGraphItem.h"
#include "JZNodeLineItem.h"
#include "JZNodeGroupItem.h"
#include "JZNodePropertyEditor.h"
#include "JZNodeAutoRunWidget.h"
#include "JZScriptItem.h"
#include "JZNodeProgram.h"
#include "JZNodeViewMap.h"
#include "JZProcess.h"

class JZNodeView;
class JZNodePanel;
class JZNodeViewPanel;
class JZProject;
class JZNodeViewCommand : public QUndoCommand
{
public:    
    JZNodeViewCommand(JZNodeView *view,int type);

    virtual void redo() override;
    virtual void undo() override;       
    virtual int id() const override;
    virtual bool mergeWith(const QUndoCommand *command);

    int command;
    int itemId;
    int pinId;
    QVariant oldValue;
    QVariant newValue;   
    QPointF oldPos;
    QPointF newPos; 

protected:
    JZNodeView *m_view;
};

class JZNodeMoveCommand : public QUndoCommand
{
public:
    struct NodePosInfo
    {
        int itemId;
        QPointF oldPos;
        QPointF newPos;
    };

    JZNodeMoveCommand(JZNodeView *view, int type);

    virtual void redo() override;
    virtual void undo() override;
    virtual int id() const override;
    virtual bool mergeWith(const QUndoCommand *command);    
   
    int command;
    QList<NodePosInfo> nodeList;

protected:
    JZNodeView *m_view;
};

class JZNodeVariableCommand : public QUndoCommand
{
public:
    JZNodeVariableCommand(JZNodeView *view, int type);

    virtual void redo() override;
    virtual void undo() override;

    int command;
    JZParamDefine newParam;
    JZParamDefine oldParam;

protected:
    JZNodeView *m_view;
};

class JZNodeView : public QGraphicsView
{
    Q_OBJECT

public:
    JZNodeView(QWidget *widget = nullptr);
    virtual ~JZNodeView();

    void setPropertyEditor(JZNodePropertyEditor *propEditor);
    void setRunEditor(JZNodeAutoRunWidget *runEditor);
    
    void setPanel(JZNodePanel *panel);
    void setFlowPanel(JZNodeViewPanel *panel);

    void setFile(JZScriptItem *file);
    JZScriptItem *file();
    void resetFile();

    bool isModified();    

    /* node */
    JZNode *getNode(int id);
    JZNodePin *getPin(JZNodeGemo gemo);
    JZNodeGraphItem *createNode(JZNode *node);
    JZNodeGraphItem *insertNode(JZNode *node);
    void removeNode(int id);   //只remove node,需要在remove node之前先删除所有连线。

    QByteArray getNodeData(int id);
    void setNodeData(int id,const QByteArray &buffer);
    void setNodePos(int id, QPointF pos);
    
    void updateNode(int id);
    bool isPropEditable(int id,int pinId);

    JZNodeGraphItem *createNodeItem(int id);    
    JZNodeGraphItem *getNodeItem(int id);

    void setNodePropValue(int nodeId, int prop_id,QString value);
    QString getNodePropValue(int nodeId, int prop_id);
    void setNodeTimer(int ms,int nodeId,int event);

    /* connect */
    JZNodeLineItem *createLine(JZNodeGemo from, JZNodeGemo to);
    JZNodeLineItem *insertLine(const JZNodeConnect &connect);
    void removeLine(int id);    

    JZNodeLineItem *createLineItem(int id);    
    JZNodeLineItem *getLineItem(int id);    
    void startLine(JZNodeGemo from);
    void endLine(JZNodeGemo to);
    void cancelLine();
    
    /* group */
    JZNodeGroupItem *createGroup(const JZNodeGroup &group);
    JZNodeGroupItem *insertGroup(const JZNodeGroup &group);
    void removeGroup(int id);
    JZNodeGroupItem *createGroupItem(int id);
    JZNodeGroupItem *getGroupItem(int id);
    QByteArray getGroupData(int id);
    void setGroupData(int id, QByteArray buffer);
    void updateGroup(int id);

    /* local variable */
    void addLocalVariableCommand(JZParamDefine def);
    void removeLocalVariableCommand(QString name);
    void changeLocalVariableCommand(QString name, JZParamDefine def);

    void addLocalVariable(JZParamDefine def);
    void removeLocalVariable(QString name);
    void changeLocalVariable(QString name,JZParamDefine def);

    void showTip(QPointF pt,QString tip);    
    void clearTip();
    QVariant onItemChange(JZNodeBaseItem *item, QGraphicsItem::GraphicsItemChange change, const QVariant &value);

    void clear();

    void save();
    void redo();
    void undo();
    void remove();
    void cut();
    void copy();
    void paste();
    void selectAll();

    void updateNodeLayout();    
    void fitNodeView();
    void ensureNodeVisible(int id);
    void selectNode(int id);
    void breakPointTrigger();

    ProcessStatus runningMode();
    void setRunningMode(ProcessStatus mode);
    void setAutoRunning(bool flag);

    int runtimeNode();
    void setRuntimeNode(int nodeId);
    void resetPropValue();
    void setRuntimeValue(int node_id,int pin_id,const JZNodeDebugParamValue &value);

    bool isBreakPoint(int nodeId);
    void setCompilerResult(const CompilerResult *info);

signals:
    void redoAvailable(bool available);
    void undoAvailable(bool available);
    void modifyChanged(bool modify);    

    void sigFunctionOpen(QString name);
    void sigAutoCompiler();
    void sigAutoRun();
    void sigRuntimeValueChanged(int id,QString value);

protected slots:
    void onContextMenu(const QPoint &pos);
    void onItemPropChanged();     
    void onItemSizeChanged();
    void onNodeTimer();
    void onMouseMoveTimer();
    void onCleanChanged(bool modify);
    void onUndoStackChanged();
    void onMapSceneChanged(QRectF rc);
    void onMapSceneScaled(bool flag);

    void onScriptNodeChanged(JZScriptItem *file, int nodeId, const QByteArray &buffer);
    void onScriptNodeWidgetChanged(JZScriptItem *file, int nodeId, int propId);
    void onPropChanged(int nodeId, int pinId, const QString &value);
    void onDependChanged();

protected:
    friend JZNodeViewCommand;

    struct NodeTimerInfo
    {
        int nodeId;
        int event;
    };
    
    virtual bool event(QEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;    

    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;    

    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dragMoveEvent(QDragMoveEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

    virtual void drawBackground(QPainter* painter, const QRectF& r) override;

    JZNodeGraphItem *nodeItemAt(QPoint pos);
    JZNodeGemo pinAt(QPoint pos);
    void foreachNode(std::function<void(JZNodeGraphItem *)> func, int nodeType = -1);
    void foreachLine(std::function<void(JZNodeLineItem *)> func);    
    void copyItems(QList<QGraphicsItem*> item);
    void removeItems(QList<QGraphicsItem*> item);
    void removeItem(QGraphicsItem *item);
    bool canRemoveItem(QGraphicsItem *item);
    QList<JZNodeGraphItem*> selectNodeItems();
    void initGraph();            
    void setSelectNode(int id);
    void updatePropEditable(const JZNodeGemo &gemo);
    void saveNodePos();
    void sceneScale(QPoint center, bool up);
    void sceneTranslate(int x,int y);
    void sceneCenter(QPointF pt);
    void udpateFlowPanel();

    void addCreateNodeCommand(const QByteArray &buffer,QPointF pt);
    void addPropChangedCommand(int id,const QByteArray &oldValue);
    void addPinValueChangedCommand(int id,int pin_id, const QByteArray &oldValue);
    void addMoveNodeCommand(int id, QPointF pt);
    
    void addRemoveLineCommand(int line_id);
    
    int addCreateGroupCommand(const JZNodeGroup &group);
    void addRemoveGroupCommand(int id);
    void addSetGroupCommand(int id, const JZNodeGroup &group);
     
    void autoCompiler();
    void autoRunning();

    QString getExpr(const QString &text = QString());
    int popMenu(QStringList list);
    QStringList matchParmas(const JZNodeObjectDefine *define,int type,QString pre);    

    JZNodePanel *m_panel;
    JZNodeViewPanel *m_flowPanel;
    bool m_isUpdateFlowPanel;

    JZNodeViewMap *m_map;
    JZNodeScene *m_scene;
    JZScriptItem *m_file;    
    JZScriptItem *m_srcFile;
    JZNodeLineItem *m_selLine;   
    QPoint m_tipPoint;

    bool m_loadFlag;
    JZNodePropertyEditor *m_propEditor;
    JZNodeAutoRunWidget *m_runEditor;
    QUndoStack m_commandStack;        
    bool m_recordMove;
    bool m_groupIsMoving;    

    QPoint m_downPoint;    
    QPointF m_downCenter;
    QTimer *m_nodeTimer;
    QTimer *m_mouseMoveTimer;
    NodeTimerInfo m_nodeTimeInfo;

    ProcessStatus m_runningMode;
    bool m_autoRunning;    
    int m_runNode;
};

#endif
