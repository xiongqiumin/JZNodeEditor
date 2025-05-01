#ifndef JZ_PROJECT_TEMPLATE_
#define JZ_PROJECT_TEMPLATE_

#include <QString>

class JZProject;
class JZProjectTemplate
{
public:
    static JZProjectTemplate *instance();

    bool initProject(JZProject *project, QString temp);
    QStringList templateList();

protected:
    JZProjectTemplate();
    void createMainWindow();

    JZProject* m_project;
};


#endif // ! JZ_PROJECT_TEMPLATE_
