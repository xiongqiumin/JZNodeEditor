#ifndef JZ_PROJECT_TEMPLATE_
#define JZ_PROJECT_TEMPLATE_

#include <QString>

class JZProject;
class JZProjectTemplate
{
public:
    static JZProjectTemplate *instance();

    bool initProject(JZProject *project, QString temp);
};


#endif // ! JZ_PROJECT_TEMPLATE_
