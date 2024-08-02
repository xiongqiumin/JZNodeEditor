#include "JZModbusParam.h"

//JZModbusTrans
void JZModbusTrans::int16ToReg(int type, int16_t v, char *buffer)
{    
    const uint8_t *ptr_value = (const uint8_t *)&v;
    uint8_t *ptr_reg = (uint8_t *)buffer;
    for (int i = 0; i < 2; i++)
        ptr_reg[i] = ptr_value[i];
}

int16_t JZModbusTrans::regToInt16(int type, const char *buffer)
{
    int v;
    uint8_t *ptr_value = (uint8_t *)&v;
    const uint8_t *ptr_reg = (const uint8_t *)buffer;
    for (int i = 0; i < 2; i++)
        ptr_value[i] = ptr_reg[i];

    return v;
}

void JZModbusTrans::int32ToReg(int type,int v, char *buffer)
{
    const uint8_t *ptr_value = (const uint8_t *)&v;
    uint8_t *ptr_reg = (uint8_t *)buffer;
    for (int i = 0; i < 4; i++)
        ptr_reg[i] = ptr_value[i];
}

int JZModbusTrans::regToInt32(int type, const char *buffer)
{
    int v;
    uint8_t *ptr_value = (uint8_t *)&v;
    const uint8_t *ptr_reg = (const uint8_t *)buffer;
    for (int i = 0; i < 4; i++)
        ptr_value[i] = ptr_reg[i];

    return v;
}

void JZModbusTrans::floatToReg(int type,float v, char *buffer)
{
    const uint8_t *ptr_value = (const uint8_t *)&v;
    uint8_t *ptr_reg = (uint8_t *)buffer;
    for (int i = 0; i < 4; i++)
        ptr_reg[i] = ptr_value[i];
}

float JZModbusTrans::regToFloat(int type, const char *buffer)
{
    float v;
    uint8_t *ptr_value = (uint8_t *)&v;
    const uint8_t *ptr_reg = (const uint8_t *)buffer;
    for (int i = 0; i < 4; i++)
        ptr_value[i] = ptr_reg[i];

    return v;
}

void JZModbusTrans::doubleToReg(int type,double v, char *buffer)
{
    const uint8_t *ptr_value = (const uint8_t *)&v;
    uint8_t *ptr_reg = (uint8_t *)buffer;
    for (int i = 0; i < 8; i++)
        ptr_reg[i] = ptr_value[i];
}

double JZModbusTrans::regToDouble(int type, const char *buffer)
{
    double v;
    uint8_t *ptr_value = (uint8_t *)&v;
    const uint8_t *ptr_reg = (const uint8_t *)buffer;
    for(int i = 0; i < 8; i++)
        ptr_value[i] = ptr_reg[i];

    return v;
}

//JZModbusStrategy
JZModbusStrategy::JZModbusStrategy()
{    
    recvNotify = false;
    autoRead = false;
    autoReadTime = 1000;
}

//JZModbusParam
QStringList JZModbusParam::addrTypeList()
{
    QStringList list = { "Coil","DiscreteInput","HoldingRegister","InputRegister" };
    return list;
}

QStringList JZModbusParam::dataTypeList()
{
    QStringList list = { "bit","int16","int32","float","double" };
    return list;
}

JZModbusParam::JZModbusParam()
{
    addr = 0;
    addrType = Param_HoldingRegister;
    dataType = Data_int16;
    value = 0;
}

int JZModbusParam::addrEnd() const
{
    if (addrType == Param_HoldingRegister || addrType == Param_InputRegister)
        return addr + byteSize() / 2;
    else
        return addr + 1;
}

void JZModbusParam::setValue(const QVariant &v)
{
    value = v;
}

void JZModbusParam::unpack(const char *buffer)
{
    Q_ASSERT(dataType != Data_bit);
    if (dataType == Data_double)
        value = JZModbusTrans::regToDouble(dataType, buffer);
    else if (dataType == Data_float)
        value = JZModbusTrans::regToFloat(dataType, buffer);
    else if (dataType == Data_int32)
        value = JZModbusTrans::regToInt32(dataType, buffer);
    else if (dataType == Data_int16)
        value = JZModbusTrans::regToInt16(dataType, buffer);
}

void JZModbusParam::pack(char *buffer)
{    
    Q_ASSERT(dataType != Data_bit);
    if(dataType == Data_double)
        JZModbusTrans::doubleToReg(dataType, value.toDouble(), buffer);
    else if (dataType == Data_float)
        JZModbusTrans::floatToReg(dataType, value.toFloat(), buffer);
    else if (dataType == Data_int32)
        JZModbusTrans::int32ToReg(dataType, value.toInt(), buffer);
    else if (dataType == Data_int16)
        JZModbusTrans::int16ToReg(dataType, value.toInt(), buffer);
}

int JZModbusParam::byteSize() const
{
    int size = 0;
    if (dataType == Data_bit)
        size = 1;
    else if (dataType == Data_double)
        size = 8;
    else if (dataType == Data_float)
        size = 4;
    else if (dataType == Data_int32)
        size = 4;
    else if (dataType == Data_int16)
        size = 2;

    return size;
}

//JZModbusParamMap
void JZModbusParamMap::clear()
{
    m_params.clear();
}

bool JZModbusParamMap::add(const JZModbusParam &value)
{
    int type = value.addrType;
    int addr = value.addr;
    int addr_end = value.addrEnd();
    for (int i = 0; i < m_params.size(); i++)
    {
        auto &p = m_params[i];
        if (p.addrType == type)
        {
            if(p.addr < addr_end && p.addrEnd() > addr)
                return false;
        }
    }

    m_params.push_back(value);
    return true;
}

void JZModbusParamMap::remove(int addr)
{
    int idx = indexOf(addr);
    if (idx == -1)
        return;

    m_params.removeAt(idx);
}

JZModbusParam *JZModbusParamMap::param(int addr)
{
    int idx = indexOf(addr);
    if (idx == -1)
        return nullptr;

    return &m_params[idx];
}

int JZModbusParamMap::indexOf(int addr)
{
    for (int i = 0; i < m_params.size(); i++)
    {
        if (m_params[i].addr == addr)
            return i;
    }
    return -1;
}

QList<int> JZModbusParamMap::paramList()
{
    QList<int> ret;
    for (int i = 0; i < m_params.size(); i++)
        ret << m_params[i].addr;
    return ret;
}

int JZModbusParamMap::count()
{
    return m_params.size();
}