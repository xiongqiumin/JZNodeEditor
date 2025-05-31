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

protected:
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

    virtual JZAbstractNodeItem *createNodeItem(JZNode *node);
    virtual JZAbstractLineItem *createLineItem(JZNodeGemo from);
};