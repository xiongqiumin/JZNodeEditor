#ifndef JZNODE_VIEW_H_
#define JZNODE_VIEW_H_

#include <QGraphicsView>
#include <functional>
#include <QMap>
#include <QTextEdit>
#include "JZNodeGraphItem.h"
#include "JZNodeScene.h"
#include "JZNodeLineItem.h"
#include "JZNodePropertyEditor.h"
#include "JZScriptFile.h"
#include <QShortcut>
#include <QUndoStack>

class JZNodeView;
class JZNodeViewCommand : public QUndoCommand
{
public:
    enum{
        CreateNode,
        RemoveNode,
        MoveNode,
        CreateLine,
        RemoveLine,            
    };

    JZNodeViewCommand(JZNodeView *view,int type);

    virtual void redo() override;
    virtual void undo() override;
    virtual int id() const override;

    int command;
    QVariantList params;        
protected:
    JZNodeView *m_view;
};

class JZNodeView : public QGraphicsView
{
    Q_OBJECT

public:
    JZNodeView(QWidget *widget = nullptr);
    virtual ~JZNodeView();

    JZProject *project();
    void setPropertyEditor(JZNodePropertyEditor *propEditor);

    /* node */
    JZNode *getNode(int id);
    JZNodeGraphItem *createNode(JZNodePtr node);
    JZNodeGraphItem *insertNode(JZNodePtr node);
    void removeNode(int id);

    JZNodeGraphItem *createNodeItem(int id);    
    JZNodeGraphItem *getNodeItem(int id);

    /* connect */
    JZNodeLineItem *createLine(JZNodeGemo from, JZNodeGemo to);
    JZNodeLineItem *insertLine(const JZNodeConnect &connect);
    void removeLine(int id);    

    JZNodeLineItem *createLineItem(int id);    
    JZNodeLineItem *getLineItem(int id);
    void startLine(JZNodeGemo from);
    void endLine(JZNodeGemo to);
    void cancelLine();

    QVariant itemChange(JZNodeBaseItem *item, QGraphicsItem::GraphicsItemChange change, const QVariant &value);

    void clear();
    void redo();
    void undo();    
    void copy();
    void paste();

    void save(QString path);
    void load(QString path);

    void updateNodeLayout();
    int paramId(int nodeId,int propId);

protected slots:
    void onContextMenu(const QPoint &pos);
    void onPropUpdate(int nodeId);
    void onSetValue(int id, QVariant value);
    void onDispValue(int id, QVariant value);
    void onTimer();

    void onCopy();
    void onPaste();
    void onRedo();
    void onUndo();

protected:
    struct CopyData
    {
        bool isEmpty();

        QList<QByteArray> nodes;
        QList<QPointF> nodesPos;
        QList<JZNodeConnect> lines;                
    };    

    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dragMoveEvent(QDragMoveEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

    virtual void drawBackground(QPainter* painter, const QRectF& r) override;

    void foreachNode(std::function<void(JZNodeGraphItem *)> func, int nodeType = -1);
    void foreachLine(std::function<void(JZNodeLineItem *)> func);    
    void run();
    void autoRun();
    void stopRun();
    void copyItem(QList<QGraphicsItem*> item);
    void removeItem(QGraphicsItem *item);    

    JZNodeScene *m_scene;
    JZScriptFile *m_file;
    JZNodeLineItem *m_selLine;
    bool m_loadFlag;
    JZNodePropertyEditor *m_propEditor;

    QUndoStack m_commandStack;    
    bool m_autoRun;

    double m_scale;
    QPoint m_downPoint;    
    bool m_isMove;  
    CopyData m_copyData;      
};

#endif
