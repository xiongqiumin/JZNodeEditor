#ifndef JZNODE_PANEL_H_
#define JZNODE_PANEL_H_

#include <QWidget>
#include <QTreeWidget>
#include "JZNode.h"
#include "JZScriptFile.h"
#include "JZNodePropertyEditor.h"

class JZNodeTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
protected:
    QMimeData *mimeData(const QList<QTreeWidgetItem *> items) const;
};

class JZScriptFile;
class JZNodePanel : public QWidget
{
    Q_OBJECT

public:
    JZNodePanel(QWidget *widget = nullptr);
    ~JZNodePanel();    

    void setFile(JZScriptFile *file);    
    void setPropertyEditor(JZNodePropertyEditor *propEditor);    

protected slots:
    void onSearch();
    void onTreeItemClicked(QTreeWidgetItem *current,int col);
    void onAddScriptParam();
    void onPropUpdate(int prop_id, const QVariant &value);

protected:
    void init();
    void initData();
    void initBasicFlow();
    void initEvent();
    void initFunction();
    void initClass();
    
    void initConstParam(QTreeWidgetItem *root);
    void initClassParam(QTreeWidgetItem *root);
    void initProjectParam(QTreeWidgetItem *root);
    void initScriptParam(QTreeWidgetItem *root);    
        
    void initProcess(QTreeWidgetItem *root);
    void initExpression(QTreeWidgetItem *root);
    void initConvert(QTreeWidgetItem *root);

    QTreeWidgetItem *createFolder(QString name);
    QTreeWidgetItem *createNode(JZNode *node);
    QTreeWidgetItem *createParam(QString name,int dataType,QString preName = QString());    
    QTreeWidgetItem *createClassEvent(QString name);

    bool filterItem(QTreeWidgetItem *root,QString name);

    JZNodeTreeWidget *m_tree;    
    JZScriptFile *m_file;
    JZScriptClassFile *m_classFile;
    QLineEdit *m_lineSearch;
    JZNodePropertyEditor *m_propEditor;    

    QTreeWidgetItem *m_itemLocalVariable;
};

#endif
