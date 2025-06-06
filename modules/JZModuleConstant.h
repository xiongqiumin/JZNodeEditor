#ifndef JZ_MODULE_CONSTANT_H_
#define JZ_MODULE_CONSTANT_H_



class JZModuleConstant
{
public:
    static JZModuleConstant *instance();    

    QList<int> BitOrderValue;
    QStringList BitOrderText;
    
    QList<int> BaudValue;
    QStringList BaudText;
    
    QList<int> DataBitValue;
    QStringList DataBitText;
    
    QList<int> ParityBitValue;
    QStringList ParityBitText;

    QList<int> StopBitValue;
    QStringList StopBitText;

protected:
    JZModuleConstant();
    ~JZModuleConstant();
};



#endif