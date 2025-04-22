#ifndef JZ_ICON_MANAGER_H_
#define JZ_ICON_MANAGER_H_

#include <QIcon>

class JZIconManager
{
public:    
    static JZIconManager *instance();

    QIcon icon(QString name);

protected:
    JZIconManager();
    ~JZIconManager();
};



#endif