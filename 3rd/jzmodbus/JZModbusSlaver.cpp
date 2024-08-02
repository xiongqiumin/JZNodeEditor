#include <memory.h>
#include "JZModbusSlaver.h"

JZModbusSlaver::JZModbusSlaver(QObject *parent)
    :QObject(parent)
{
    m_server = new JZModbusServer(this);
    connect(m_server, &JZModbusServer::sigMappingChanged, this, &JZModbusSlaver::onMappingChanged);
}

JZModbusSlaver::~JZModbusSlaver()
{
}

void JZModbusSlaver::setSlave(int slave)
{
    m_server->setSlave(slave);
}

JZModbusParamMap *JZModbusSlaver::map()
{
    return &m_map;
}

bool JZModbusSlaver::startRtuServer(QString com, int baud, QSerialPort::DataBits data_bit, QSerialPort::StopBits stop_bit, QSerialPort::Parity parity_bit)
{
    return m_server->startRtuServer(com,baud,data_bit,stop_bit,parity_bit);
}

bool JZModbusSlaver::startTcpServer(int port)
{
    return m_server->startTcpServer(port);
}

void JZModbusSlaver::stopSever()
{
    m_server->stop();
}

void JZModbusSlaver::initMapping()
{    
    int start_bits = INT_MAX;
    int end_bits = 0;
    int start_input_bits = INT_MAX;
    int end_input_bits = 0;
    int start_input_registers = INT_MAX;
    int end_input_registers = 0;
    int start_registers = INT_MAX;
    int end_registers = 0;

    auto list = m_map.paramList();
    for (int i = 0; i < list.size(); i++)
    {
        auto p = m_map.param(list[i]);
        if(p->addrType == Param_Coil)
        {
            start_bits = qMin(start_bits, p->addr);
            end_bits = qMax(end_bits, p->addr + 1);
        }
        else if(p->addrType == Param_DiscreteInput)
        {
            start_input_bits = qMin(start_input_bits, p->addr);
            end_input_bits = qMax(end_input_bits, p->addr + 1);            
        }
        else if(p->addrType == Param_HoldingRegister)
        {
            int nb = p->byteSize() / 2;
            start_registers = qMin(start_registers, p->addr);
            end_registers = qMax(end_registers, p->addr + nb);            
        }
        else if(p->addrType == Param_InputRegister)
        {
            int nb = p->byteSize() / 2;
            start_input_registers = qMin(start_input_registers, p->addr);
            end_input_registers = qMax(end_input_registers, p->addr + nb);
        }
    }

    int nb_bits = (end_bits > start_bits) ? end_bits - start_bits : 0;
    int nb_input_bits = (end_input_bits > start_input_bits) ? end_input_bits - start_input_bits : 0;
    int nb_input_registers = (end_input_registers > start_input_registers) ? end_input_registers - start_input_registers : 0;
    int nb_registers = (end_registers > start_registers) ? end_registers - start_registers : 0;

    auto map = m_server->mapping();
    map->init(nb_bits, nb_input_bits, nb_registers, nb_input_registers);
    map->start_bits = start_bits;
    map->start_input_bits = start_input_bits;
    map->start_registers = start_registers;
    map->start_input_registers = start_input_registers;
    
    for (int i = 0; i < list.size(); i++)
    {
        auto p = m_map.param(list[i]);
        writeParam(p->addr, p->value);
    }
}

void JZModbusSlaver::writeParam(int addr, QVariant value)
{
    auto p = m_map.param(addr);
    if (!p)
        return;

    p->setValue(value);
    auto map = m_server->mapping();
    void *ptr = nullptr;
    if (p->addrType == Param_Coil || p->addrType == Param_DiscreteInput)
    {
        if (p->addrType == Param_Coil)
        {
            int mapping_address = addr - map->start_bits;
            map->tab_bits[mapping_address] = (value.toInt() != 0)? 1 : 0;
        }
        else if (p->addrType == Param_DiscreteInput)
        {
            int mapping_address = addr - map->start_input_bits;
            map->tab_input_bits[mapping_address] = (value.toInt() != 0)? 1 : 0;
        }
    }
    else
    {
        QByteArray buffer;
        buffer.resize(p->byteSize());
        p->pack(buffer.data());

        if (p->addrType == Param_HoldingRegister)
        {
            int mapping_address = addr - map->start_registers;
            ptr = map->tab_registers + mapping_address;
        }
        else if (p->addrType == Param_InputRegister)
        {
            int mapping_address = addr - map->start_input_registers;
            ptr = map->tab_input_registers + mapping_address;
        }
        memcpy(ptr, buffer.data(), buffer.size());
    }    
}

QVariant JZModbusSlaver::readParam(int addr)
{
    auto p = m_map.param(addr);
    if (!p)
        return QVariant();

    return p->value;
}

void JZModbusSlaver::onMappingChanged(int in_addr, int nb)
{
    auto map = m_server->mapping();
    auto list = m_map.paramList();    
    for (int addr = in_addr; addr < in_addr + nb; addr++)
    {
        for (int i = 0; i < list.size(); i++)
        {
            if (addr == list[i])
            {                
                auto p = m_map.param(addr);                
                QVariant pre = p->value;

                if (p->addrType == Param_Coil)
                {
                    auto ptr = map->tab_bits + addr - map->start_bits;
                    p->value = *ptr;
                }
                else if (p->addrType == Param_HoldingRegister)
                {
                    auto ptr = map->tab_registers + addr - map->start_registers;                    
                    p->unpack((const char*)ptr);
                }

                if (pre != p->value)
                    emit sigParamChanged(addr);
            }
        }
    }
}