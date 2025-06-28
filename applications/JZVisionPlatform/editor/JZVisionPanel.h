#ifndef JZ_VISION_PANEL_H_
#define JZ_VISION_PANEL_H_

#include "JZNodeAbstractPanel.h"
#include "JZNodeEditorManager.h"

class JZVisionPanel : public JZNodeAbstractPanel
{
public:
    JZVisionPanel();

    virtual void init() override;
    virtual void updateDefine() override;

protected:
    void initContainer(QTreeWidgetItem *item_root);
    void initLogicNode();
    void initLogic(QTreeWidgetItem *root);
    void registLogicNode(int node_type, QString path, QString icon = QString());

    QTreeWidgetItem *createDefaultNode(JZNode *node);

    QList<JZLogicNode> m_logicList;
};

#endif