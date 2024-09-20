#ifndef JZNODE_PANEL_H_
#define JZNODE_PANEL_H_

#include <QWidget>
#include <QTreeWidget>
#include "JZNode.h"
#include "JZScriptItem.h"
#include "JZNodePropertyEditor.h"
#include "JZModule.h"

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
class JZNodeView;
class JZNodePanel : public QWidget
{
    Q_OBJECT

public:
    JZNodePanel(QWidget *widget = nullptr);
    ~JZNodePanel();    

    void setFile(JZScriptItem *file);        
    void setView(JZNodeView *view);
    void updateDefine();

    QTreeWidgetItem *itemOp();
    QTreeWidgetItem *itemProcess();

    void addLocalVariable(JZParamDefine def);
    void removeLocalVariable(QString name);
    void changeLocalVariable(QString name, JZParamDefine def);

protected slots:
    void onSearch();
    void onTreeItemClicked(QTreeWidgetItem *current,int col);
    void onAddScriptParam();    
    void onContextMenu(const QPoint &pos);

protected:        
    void init();
    void initData();
    void initBasicFlow();    
    void initLocalDefine();    

    void initThis(QTreeWidgetItem *root);
    void initConstParam(QTreeWidgetItem *root);        
    void initScriptParam(QTreeWidgetItem *root);    
        
    void initProcess(QTreeWidgetItem *root);
    void initExpression(QTreeWidgetItem *root);
    void initConvert(QTreeWidgetItem *root);

    void addModule(QTreeWidgetItem *item_root,QString name);
    void updateClass(QTreeWidgetItem *item_root,const QString &class_name,bool show_protected);

    void updateThis();    
    void updateInputVariable();
    void updateLocalVariable();
    void updateGlobalVariable();
    void updateModule();
    void updateLocalDefine();
    void updateVariable(QTreeWidgetItem *item, QStringList paramList);
    
    QTreeWidgetItem *createFolder(QString name);
    QTreeWidgetItem *createNode(JZNode *node);
    QTreeWidgetItem *createParam(QString name);
    QTreeWidgetItem *createMemberParam(QString name);
    QTreeWidgetItem *createClass(QString name);
    QTreeWidgetItem *createFunction(QString name);
    void setNode(QTreeWidgetItem *item,JZNode *node);

    void removeItem(QTreeWidgetItem *root, QString name);
    bool filterItem(QTreeWidgetItem *root,QString name);    
    bool isClassItem(QTreeWidgetItem *item);
    const JZModule *module(QString name);
    QTreeWidgetItem *localVariableItem(QString name);

    JZNodeTreeWidget *m_tree;    
    JZNodeView *m_view;
    JZScriptItem *m_file;
    JZScriptClassItem *m_classFile;
    QLineEdit *m_lineSearch;    
    
    QTreeWidgetItem *m_itemOp;
    QTreeWidgetItem *m_itemProcess;
    QTreeWidgetItem *m_memberFunction;
    QTreeWidgetItem *m_itemMemberParam;
    QTreeWidgetItem *m_itemInputParam;
    QTreeWidgetItem *m_itemLocalParam;
    QTreeWidgetItem *m_itemGlobalParam;

    QTreeWidgetItem *m_itemLocalDefine;
    QTreeWidgetItem *m_module;

    QList<JZModuleStatic> m_modules;
};

#endif
