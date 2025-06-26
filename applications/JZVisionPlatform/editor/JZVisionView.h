#pragma once

#include <QGraphicsView>
#include "JZScriptItem.h"
#include "JZVisionNodeItem.h"
#include "JZVisionLineItem.h"
#include "JZNodeAbstractView.h"
#include "JZVisionAppNode.h"

class JZVisionView : public JZNodeAbstractView
{
    Q_OBJECT
    
public:
    explicit JZVisionView(QWidget *parent = nullptr);
    ~JZVisionView();

    QString nodeName(int node_id);
    QString nodeBaseName(JZNode* node);
    QString pinName(JZNodeGemo gemo);
    void updateNodeName();

    JZVisionParamLink linkInfo(int node_id, int pin_id);
    void addCreateLinkCommand(const JZVisionParamLink& link, const JZNodeGemo& gemo);
    void addRemoveLinkCommand(int id);

protected slots:
    void onContextMenu(const QPoint &pos);

protected:
    static bool nodeIdCmp(const JZNode* n1, const JZNode* n2);

    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

    virtual void initGraph();
    virtual JZAbstractNodeItem *createNodeItem(JZNode *node);
    virtual JZAbstractLineItem *createLineItem(JZNodeGemo from);

    void configNode(JZNode *node);

    QMap<JZNode*, QString> m_nodeName;
};