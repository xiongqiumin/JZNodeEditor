#pragma once

#include <QGraphicsView>
#include "JZScriptItem.h"
#include "JZVisionNodeItem.h"
#include "JZVisionLineItem.h"
#include "JZVisionCommand.h"

class JZVisionView : public QGraphicsView
{
    Q_OBJECT
    
public:
    explicit JZVisionView(QWidget *parent = nullptr);
    ~JZVisionView();

    /* node */
    JZNode* getNode(int id);
    JZVisionNodeItem* createNode(JZNode* node);
    JZVisionNodeItem* insertNode(JZNode* node);
    void removeNode(int id);   //只remove node,需要在remove node之前先删除所有连线。

    QByteArray getNodeData(int id);
    void setNodeData(int id, const QByteArray& buffer);
    void setNodePos(int id, QPointF pos);
    void setNodePinValue(int id, int pin, QString value);
    void updateNode(int id);
    
    JZVisionNodeItem* createNodeItem(int id);
    JZVisionNodeItem* getNodeItem(int id);

    /* connect */
    JZVisionLineItem* createLine(JZNodeGemo from, JZNodeGemo to);
    JZVisionLineItem* insertLine(const JZNodeConnect& connect);
    void removeLine(int id);

    JZVisionLineItem* createLineItem(int id);
    JZVisionLineItem* getLineItem(int id);
    void startLine(JZNodeGemo from);
    void endLine(JZNodeGemo to);
    void cancelLine();

    QVariant onItemChange(JZVisionItemBase* item, QGraphicsItem::GraphicsItemChange change, const QVariant& value);

protected:
    friend JZVisionCommand;

    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

    void onContextMenu(const QPoint& pos);

    void addCreateNodeCommand(const QByteArray& buffer, QPointF pt);
    void addNodeChangedCommand(int id, const QByteArray& oldValue);
    void addPinValueChangedCommand(int id, int pin_id, const QString& oldValue);
    void addMoveNodeCommand(int id, QPointF pt);

    void addRemoveLineCommand(int line_id);

    QUndoStack m_commandStack;

    bool m_recordMove;
    JZVisionLineItem *m_selLine;
    JZScriptItem* m_file;
    QGraphicsScene* m_scene;
};