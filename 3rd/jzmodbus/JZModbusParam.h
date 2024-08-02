#ifndef JZ_MODBUS_PARAM_H_
#define JZ_MODBUS_PARAM_H_

#include <inttypes.h>
#include <QVector>
#include <QVariant>

enum {
    Param_Coil,
    Param_DiscreteInput,
    Param_HoldingRegister,
    Param_InputRegister,
};

class JZModbusTrans 
{
public:
    static void int16ToReg(int type, int16_t i, char *buffer);
    static int16_t regToInt16(int type, const char *buffer);

    static void int32ToReg(int type, int i, char *buffer);
    static int regToInt32(int type, const char *buffer);

    static void floatToReg(int type,float v, char *buffer);
    static float regToFloat(int type, const char *buffer);
    
    static void doubleToReg(int type,double v, char *buffer);
    static double regToDouble(int type,const char *buffer);
};

class JZModbusStrategy
{
public:
    JZModbusStrategy();

    bool recvNotify;
    bool autoRead;
    int autoReadTime;
};

class JZModbusParam
{
public:
    enum {
        Data_bit,
        Data_int16,
        Data_int32,
        Data_float,
        Data_double,
    };

    static QStringList addrTypeList();
    static QStringList dataTypeList();

    JZModbusParam();

    void unpack(const char *buffer);
    void pack(char *buffer);
    int byteSize() const;
    int addrEnd() const;
    void setValue(const QVariant &v);

    QString name;
    int addr;
    int addrType;
    int dataType;
    QVariant value;
    QString memo;        
};

class JZModbusParamMap
{
public:
    void clear();

    bool add(const JZModbusParam &value);
    void remove(int addr);
    JZModbusParam *param(int addr);
    int indexOf(int addr);
    QList<int> paramList();
    int count();    

    bool save(QString file);
    bool load(QString file);    

protected:
    QList<JZModbusParam> m_params;
};

#endif // !JZ_MODBUS_VALUE_H_
