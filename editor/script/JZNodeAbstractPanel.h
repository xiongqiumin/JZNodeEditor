#ifndef JZNODE_ABSTRACT_PANEL_H_
#define JZNODE_ABSTRACT_PANEL_H_

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
class JZNodeAbstractView;
class JZNodeAbstractPanel : public QWidget
{
    Q_OBJECT

public:
    JZNodeAbstractPanel(QWidget *widget = nullptr);
    ~JZNodeAbstractPanel();    

    void setFile(JZScriptItem *file);        
    void setView(JZNodeAbstractView *view);

    virtual void init() = 0;
    virtual void updateDefine() = 0;    

    QTreeWidgetItem *itemOp();
    QTreeWidgetItem *itemProcess();
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
            
    static bool name_sort(const QString &a, const QString &b);

    void initProcess(QTreeWidgetItem *root);
    void initExpression(QTreeWidgetItem *root);
    void initLocalParam(QTreeWidgetItem *root);
    void updateLocalParam();
        
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
    JZNodeAbstractView *m_view;
    JZScriptItem *m_file;
    JZScriptClassItem *m_classFile;
    QLineEdit *m_lineSearch;    
        
    QTreeWidgetItem *m_itemLocalParam;    
    QTreeWidgetItem *m_itemOp;
    QTreeWidgetItem *m_itemProcess;
};

#endif
