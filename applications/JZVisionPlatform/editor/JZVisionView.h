#pragma once

#include <QGraphicsView>
#include "JZScriptItem.h"
#include "JZVisionNodeItem.h"
#include "JZVisionLineItem.h"
#include "JZNodeAbstractView.h"

class JZVisionView : public JZNodeAbstractView
{
    Q_OBJECT
    
public:
    explicit JZVisionView(QWidget *parent = nullptr);
    ~JZVisionView();

    QString nodeName(JZNode *node);
    QString pinName(JZNodeGemo gemo);

protected slots:
    void onContextMenu(const QPoint &pos);

protected:
    static bool nodeIdCmp(const JZNode* n1, const JZNode* n2);

    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

    virtual JZAbstractNodeItem *createNodeItem(JZNode *node);
    virtual JZAbstractLineItem *createLineItem(JZNodeGemo from);

    void configNode(JZNode *node);


    QList<int> m_cacheNodeList;
    QMap<JZNode*, QString> m_nodeName;
};