#ifndef JZ_PROJECT_TEMPLATE_
#define JZ_PROJECT_TEMPLATE_

#include <QString>

class JZProjectTemplate
{
public:
    static JZProjectTemplate *instance();

    bool initProject(QString path, QString name, QString temp);
};


#endif // ! JZ_PROJECT_TEMPLATE_
