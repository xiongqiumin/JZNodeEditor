#ifndef JZNODE_PANEL_H_
#define JZNODE_PANEL_H_

#include <QWidget>
#include <QTreeWidget>
#include "JZNode.h"
#include "JZScriptItem.h"
#include "JZNodePropertyEditor.h"

enum {
    TreeItem_type = Qt::UserRole,
    TreeItem_value,
    TreeItem_isClass,
};

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

    QTreeWidgetItem *itemOp();
    QTreeWidgetItem *itemProcess();

protected slots:
    void onSearch();
    void onTreeItemClicked(QTreeWidgetItem *current,int col);
    void onAddScriptParam();    
    void onContextMenu(const QPoint &pos);

protected:
    struct Module
    {
        QString name;
        QStringList typeList;
        QStringList functionList;
    };
    
    void init();
    void initData();
    void initBasicFlow();
    void initModule();
    void initLocalDefine();
    
    void initThis(QTreeWidgetItem *root);
    void initConstParam(QTreeWidgetItem *root);    
    void initProjectParam(QTreeWidgetItem *root);
    void initScriptParam(QTreeWidgetItem *root);    
        
    void initProcess(QTreeWidgetItem *root);
    void initExpression(QTreeWidgetItem *root);
    void initConvert(QTreeWidgetItem *root);

    void updateClass();
    void updateLocalVariable();

    QTreeWidgetItem *createFolder(QString name);
    QTreeWidgetItem *createNode(JZNode *node);
    QTreeWidgetItem *createParam(QString name);
    QTreeWidgetItem *createMemberParam(QString name);
    QTreeWidgetItem *createClass(QString name);
    QTreeWidgetItem *createFunction(QString name);

    bool filterItem(QTreeWidgetItem *root,QString name);    
    bool isClassItem(QTreeWidgetItem *item);

    JZNodeTreeWidget *m_tree;    
    JZScriptItem *m_file;
    JZScriptClassItem *m_classFile;
    QLineEdit *m_lineSearch;    
    
    QTreeWidgetItem *m_itemOp;
    QTreeWidgetItem *m_itemProcess;
    QTreeWidgetItem *m_memberFunction;
    QTreeWidgetItem *m_memberParam;
    QTreeWidgetItem *m_itemLocalParam;
    QList<Module> m_modules;
};

#endif
