#ifndef JZ_MODEL_MANAGER_H_
#define JZ_MODEL_MANAGER_H_

#include <QDataStream>

class JZModelManagerConfig
{
public:    
};
QDataStream& operator<<(QDataStream &s,const JZModelManagerConfig &config);
QDataStream& operator>>(QDataStream &s,JZModelManagerConfig &config);

class JZModelManager
{
public:
    JZModelManager();
    ~JZModelManager();
    
    void initConfig();
    
};



#endif