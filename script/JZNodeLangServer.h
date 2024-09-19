#ifndef JZNODE_LANG_SERVER_H_
#define JZNODE_LANG_SERVER_H_

#include <QObject>
#include <QString> 

class JZProject;
class JZNodeLangServer : public QObject
{
    Q_OBJECT

public:
    static JZNodeLangServer *instance();
    
    void setProject(JZProject *project);

    QStringList type(const QString &path);
    QStringList param(const QString &path);
    QStringList member(const QString &class_name);

signals:
    void sigClassChanged();    

protected:
    JZNodeLangServer();

    JZProject *m_project;
};

#endif // ! JZNODE_LANG_SERVER_H_
