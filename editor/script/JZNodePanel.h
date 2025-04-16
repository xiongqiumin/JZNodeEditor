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

    QTreeWidgetItem *localVariableItem(QString name);    

protected slots:
    void onSearch();
    void onTreeItemClicked(QTreeWidgetItem *current,int col);
    void onAddScriptParam();    
    void onContextMenu(const QPoint &pos);

protected:      
    enum {
        Create_Function,
        Create_Param,
        Create_Class,
    };

    void init();       
    void initBasic();    
    void initLocalDefine();        

    void initThis(QTreeWidgetItem *root);
    void initConstParam(QTreeWidgetItem *root);        
        
    void initProcess(QTreeWidgetItem *root);
    void initExpression(QTreeWidgetItem *root);
    void initAll(QTreeWidgetItem *root);

    void addModule(QTreeWidgetItem *item_root,QString name);    

    void updateFunction();
    void updateThis();        
    void updateLocalDefine();
    void updateGlobalVariable();        
        
    QTreeWidgetItem *createFolder(QString name);
    QTreeWidgetItem *createNode(JZNode *node);
    QTreeWidgetItem *createParam(QString name);
    QTreeWidgetItem *createClass(QString name);
    QTreeWidgetItem *createFunction(QString name);
    void setNode(QTreeWidgetItem *item,JZNode *node);

    void removeItem(QTreeWidgetItem *root, QString name);
    bool filterItem(QTreeWidgetItem *root,QString name);    
    bool isClassItem(QTreeWidgetItem *item);
    const JZModule *module(QString name);    

    QStringList childItemText(QTreeWidgetItem *root);
    void sortChildItem(QTreeWidgetItem *root);
    void syncChildList(QTreeWidgetItem *root,QStringList list,int type);

    JZNodeTreeWidget *m_tree;    
    JZNodeView *m_view;
    JZScriptItem *m_file;
    JZScriptClassItem *m_classFile;
    QLineEdit *m_lineSearch;    
    
    QTreeWidgetItem *m_itemFunction;       
    QTreeWidgetItem *m_itemLocalParam;
    QTreeWidgetItem *m_itemClassDefine;
    QTreeWidgetItem *m_itemLocalDefine;    
    QTreeWidgetItem *m_itemGlobalVariable;
};

#endif
