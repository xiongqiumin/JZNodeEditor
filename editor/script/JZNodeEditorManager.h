#ifndef JZNODE_EDITOR_MANAGER_H_
#define JZNODE_EDITOR_MANAGER_H_

#include <QString>
#include <QMap>

class JZNodeEditorManager
{
public:
    static JZNodeEditorManager *instance();

    void registCustomFunctionNode(QString function,int node_type);
    void unregistCustomFunctionNode(QString function);
    int customFunctionNode(QString function);

protected:
    JZNodeEditorManager();
    ~JZNodeEditorManager();

    QMap<QString, int> m_functionMap;
};


#endif // !JZNODE_EDITOR_MANAGER_H_
