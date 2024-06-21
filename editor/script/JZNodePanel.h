#ifndef JZNODE_PANEL_H_
#define JZNODE_PANEL_H_

#include <QWidget>
#include <QTreeWidget>
#include "JZNode.h"
#include "JZScriptItem.h"
#include "JZNodePropertyEditor.h"

class JZNodeTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
protected:
    QMimeData *mimeData(const QList<QTreeWidgetItem *> items) const;
};

class JZScriptItem;
class JZNodePanel : public QWidget
{
    Q_OBJECT

public:
    JZNodePanel(QWidget *widget = nullptr);
    ~JZNodePanel();    

    void setFile(JZScriptItem *file);        
    void updateNode();

protected slots:
    void onSearch();
    void onTreeItemClicked(QTreeWidgetItem *current,int col);
    void onAddScriptParam();    
    void onContextMenu(const QPoint &pos);

protected:
    void init();
    void initData();
    void initBasicFlow();
    void initFunction();
    void initEnums();
    void initClass();
    
    void initThis(QTreeWidgetItem *root);
    void initConstParam(QTreeWidgetItem *root);    
    void initProjectParam(QTreeWidgetItem *root);
    void initScriptParam(QTreeWidgetItem *root);    
        
    void initProcess(QTreeWidgetItem *root);
    void initExpression(QTreeWidgetItem *root);
    void initConvert(QTreeWidgetItem *root);

    void updateClass();    
    void updateInput();

    QTreeWidgetItem *createFolder(QString name);
    QTreeWidgetItem *createNode(JZNode *node);
    QTreeWidgetItem *createParam(QString name);    
    QTreeWidgetItem *createClassEvent(QString name);

    bool filterItem(QTreeWidgetItem *root,QString name);    
    bool isClassItem(QTreeWidgetItem *item);

    JZNodeTreeWidget *m_tree;    
    JZScriptItem *m_file;
    JZScriptClassItem *m_classFile;
    QLineEdit *m_lineSearch;    
    
    QTreeWidgetItem *m_memberFunction;
    QTreeWidgetItem *m_memberParam;
    QTreeWidgetItem *m_itemLocal;
    QTreeWidgetItem *m_itemInput;

    QMap<QString, QStringList> m_functionMap;
};

#endif
