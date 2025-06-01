#ifndef JZNODE_ABSTRACT_VIEW_H_
#define JZNODE_ABSTRACT_VIEW_H_

#include <QGraphicsView>
#include <functional>
#include <QMap>
#include <QTextEdit>
#include <QShortcut>
#include <QUndoStack>
#include <QGraphicsRectItem>
#include "JZNodeBaseItem.h"
#include "JZNodeGroupItem.h"
#include "JZNodePropertyEditor.h"
#include "JZNodeAutoRunWidget.h"
#include "JZScriptItem.h"
#include "JZNodeProgram.h"
#include "JZNodeDebugPacket.h"
#include "JZProcess.h"
#include "JZNodeViewCommand.h"
#include "JZNodeViewMap.h"

class JZNodeAbstractPanel;

//JZNodeAbstractView
class JZNodeAbstractView : public QGraphicsView
{
    Q_OBJECT

public:
    JZNodeAbstractView(QWidget *widget = nullptr);
    virtual ~JZNodeAbstractView();

    void setPanel(JZNodeAbstractPanel *panel);

    void setFile(JZScriptItem *file);
    JZScriptItem *file();
    void resetFile();

    bool isModified();    

    /* node */
    JZNode *getNode(int id);
    JZNodePin *getPin(JZNodeGemo gemo);
    JZAbstractNodeItem *createNode(JZNode *node);
    JZAbstractNodeItem *insertNode(JZNode *node);
    void removeNode(int id);   //只remove node,需要在remove node之前先删除所有连线。

    QByteArray getNodeData(int id);
    void setNodeData(int id,const QByteArray &buffer);
    void setNodePos(int id, QPointF pos);    
    void setNodePinValue(int id, int pin, QString value);
    
    void updateNode(int id);
    bool isPropEditable(int id,int pinId);

    JZAbstractNodeItem *createNodeItem(int id);    
    JZAbstractNodeItem *getNodeItem(int id);       

    /* connect */
    JZAbstractLineItem *createLine(JZNodeGemo from, JZNodeGemo to);
    JZAbstractLineItem *insertLine(const JZNodeConnect &connect);
    void removeLine(int id);    

    JZAbstractLineItem *createLineItem(int id);    
    JZAbstractLineItem *getLineItem(int id);    
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

    /* node */
    void addCreateNodeCommand(const QByteArray &buffer, QPointF pt);
    void addNodeChangedCommand(int id, const QByteArray &oldValue);
    void addNodeValueChangedCommand(int id, const QByteArray &oldValue);
    void addPinValueChangedCommand(int id, int pin_id, const QString &oldValue);
    void addMoveNodeCommand(int id, QPointF pt);

    /* line */
    void addCreateLineConmmand(JZNodeGemo from,JZNodeGemo to);
    void addRemoveLineCommand(int line_id);

    /* group */
    int addCreateGroupCommand(const JZNodeGroup &group);
    void addRemoveGroupCommand(int id);
    void addSetGroupCommand(int id, const JZNodeGroup &group);

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
    
    void fitNodeView();
    void ensureNodeVisible(int id);
    void selectNode(int id);    

    void setCompilerResult(const CompilerResult *compilerInfo);

signals:
    void redoAvailable(bool available);
    void undoAvailable(bool available);
    void modifyChanged(bool modify);    
    
    void sigAutoCompiler();        
    void sigSelectNodeChanged(JZAbstractNodeItem *item);

public slots:    
    void onScrpitNodeChanged(JZScriptItem *item, int nodeId, const QByteArray &buffer);

protected slots:        
    void onMouseMoveTimer();   //边缘滚屏    

    void onCleanChanged(bool modify);
    void onUndoStackChanged();
    void onMapSceneChanged(QRectF rc);
    void onMapSceneScaled(bool flag);            

protected:
    friend JZNodeViewCommand;
    
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

    virtual JZAbstractNodeItem *createNodeItem(JZNode *node) = 0;
    virtual JZAbstractLineItem *createLineItem(JZNodeGemo from) = 0;    

    JZAbstractNodeItem *nodeItemAt(QPoint pos);
    JZNodeGemo pinAt(QPoint pos);
    void foreachNode(std::function<void(JZAbstractNodeItem *)> func);
    void foreachNode(int node_type, std::function<void(JZAbstractNodeItem *)> func);
    void foreachLine(std::function<void(JZAbstractLineItem *)> func);    

    void copyItems(QList<QGraphicsItem*> item);
    void removeItems(QList<QGraphicsItem*> item);
    void removeItem(QGraphicsItem *item);
    bool canRemoveItem(QGraphicsItem *item);
    QList<JZAbstractNodeItem*> selectNodeItems();
    QList<JZAbstractNodeItem*> nodeItems();
    
    void initGraph();            
    void setSelectNode(int id);
    void updatePropEditable(const JZNodeGemo &gemo);
    void saveNodePos();
    void sceneScale(QPoint center, bool up);
    void sceneTranslate(int x,int y);
    void sceneCenter(QPointF pt);     
    void autoCompiler();    

    int popMenu(QStringList list);    

    JZNodeViewMap *m_map;
    QGraphicsScene *m_scene;
    JZScriptItem *m_file;        
    JZAbstractLineItem *m_selLine;
    JZNodeAbstractPanel *m_panel;
    QPoint m_tipPoint;

    bool m_loadFlag;    
    QUndoStack m_commandStack;        
    bool m_recordMove;
    bool m_groupIsMoving;    

    QPoint m_downPoint;    
    QPointF m_downCenter;    
    QTimer *m_mouseMoveTimer;            
};

#endif
