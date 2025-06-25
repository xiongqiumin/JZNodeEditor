#ifndef JZNODE_VALIDATOR_
#define JZNODE_VALIDATOR_

#include <QString>
#include <QStringList>
#include <QMap>

class JZParamValidator
{
public:
    enum Type
    {
        none,
        minMax,
        enumeration,
    };

    Type type;
    QString min,max;
    QStringList enumList;
};


class JZNodeValidatorManager
{
public:
    static JZNodeValidatorManager *instance();

    JZNodeValidatorManager();

    void registPinValidator(int node_type,int pin_id,JZParamValidator validator);
    const JZParamValidator *pinValidator(int node_type,int pin_id);
    
protected:
    QMap<int,QMap<int,JZParamValidator>> m_pinValidator;
};

#endif