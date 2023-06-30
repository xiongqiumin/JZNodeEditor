#ifndef JZNODE_PANEL_H_
#define JZNODE_PANEL_H_

#include <QWidget>
#include <QTreeWidget>
#include "JZNode.h"
#include "JZScriptFile.h"

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

protected slots:
    void onSearch();

protected:
    void init();
    void initEvent(QTreeWidgetItem *root);
    void initClassParam(QTreeWidgetItem *root);
    void initProjectParam(QTreeWidgetItem *root);
    void initVariable(QTreeWidgetItem *root);
    void initProcess(QTreeWidgetItem *root);
    void initExpression(QTreeWidgetItem *root);
    void initFunction(QTreeWidgetItem *root,bool flow);    
    QTreeWidgetItem *createFolder(QString name);
    QTreeWidgetItem *createNode(JZNode *node);
    QTreeWidgetItem *createParam(QString name,int dataType,QString preName);
    QTreeWidgetItem *createClass(QString className);
    bool filterItem(QTreeWidgetItem *root,QString name);

    JZNodeTreeWidget *m_tree;
    int m_fileType;
    JZScriptFile *m_file;
    JZScriptClassFile *m_classFile;
    QLineEdit *m_lineSearch;
};

#endif
