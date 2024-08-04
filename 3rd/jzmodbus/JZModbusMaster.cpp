#include <QTimer>
#include <QDateTime>
#include "JZModbusMaster.h"

JZModbusMaster::JZModbusMaster()
{
    m_client = new JZModbusClient(this);
    connect(m_client, &JZModbusClient::sigModbusReplay, this, &JZModbusMaster::onModbusReply);

    m_timeId = 0;
}

JZModbusMaster::~JZModbusMaster()
{
}

JZModbusParam *JZModbusMaster::param(int addr)
{
    return m_map.param(addr);
}

void JZModbusMaster::writeParam(int addr, QVariant value)
{
    auto p = m_map.param(addr);
    if (!p)
        return;

    p->setValue(value);
}

QVariant JZModbusMaster::readParam(int addr)
{
    auto p = m_map.param(addr);
    if (!p)
        return QVariant();

    return p->value;
}

void JZModbusMaster::writeRemoteParam(int addr, QVariant value)
{
    m_client->waitFinish();

    auto p = m_map.param(addr);
    if (!p)
        return;

    p->value = value;    
    void *ptr = nullptr;
    if (p->addrType == Param_Coil)
        m_client->writeBit(p->addr, p->value.toInt());        
    else
    {
        QVector<uint16_t> buffer;
        buffer.resize(p->byteSize()/2);
        p->pack((char*)buffer.data());
        m_client->writeRegisters(p->addr, buffer);        
    }
}

QVariant JZModbusMaster::readRemoteParam(int addr)
{
    m_client->waitFinish();

    auto p = m_map.param(addr);
    if (!p)
        return QVariant();
     
    if (p->addrType == Param_Coil)
    {
        QVector<uint8_t> dest;
        if (m_client->readBits(p->addr, 1, dest))
            p->value = dest[0];        
    }
    else if (p->addrType == Param_DiscreteInput)
    {
        QVector<uint8_t> dest;
        if (m_client->readInputBits(p->addr, 1, dest))
            p->value = dest[0];
    }
    else
    {
        QVector<uint16_t> buffer;
        int nb = p->byteSize() / 2;
        bool ret = false;
        if (p->addrType == Param_HoldingRegister)
            ret = m_client->readRegisters(p->addr,nb,buffer);
        else if (p->addrType == Param_InputRegister)
            ret = m_client->readInputRegisters(p->addr, nb, buffer);

        if (ret)
            p->unpack((const char*)buffer.data());
    }
    return p->value;
}

void JZModbusMaster::writeRemoteParamAsync(int addr, QVariant value)
{
    auto p = m_map.param(addr);
    if (!p)
        return;

    Command cmd;
    cmd.type = Command::Write;
    cmd.addr = addr;
    cmd.value = value;
    m_commandList.push_back(cmd);    

    dealCommand();
}

void JZModbusMaster::readRemoteParamAsync(int addr)
{
    auto p = m_map.param(addr);
    if (!p)
        return;

    Command cmd;
    cmd.type = Command::Read;
    cmd.addr = addr;    
    m_commandList.push_back(cmd);

    dealCommand();
}

void JZModbusMaster::setSlave(int slave)
{
    m_client->setSlave(slave);
}

bool JZModbusMaster::isOpen()
{
    return m_client->isOpen();
}

bool JZModbusMaster::isBusy()
{
    return m_client->isBusy();
}

void JZModbusMaster::initRtu(QString com, int baud, QSerialPort::DataBits data_bit, QSerialPort::StopBits stop_bit, QSerialPort::Parity parity_bit)
{
    m_client->initRtu(com, baud, data_bit, stop_bit, parity_bit);        
}

void JZModbusMaster::initTcp(QString ip, int port)
{
    m_client->initTcp(ip, port);    
}

bool JZModbusMaster::open()
{
    bool ret = m_client->open();
    if (ret && m_timeId == 0)
        m_timeId = startTimer(20);

    return ret;
}

void JZModbusMaster::close()
{
    m_client->close();
    killTimer(m_timeId);
    m_timeId = 0;
}

JZModbusParamMap *JZModbusMaster::map()
{
    return &m_map;
}

void JZModbusMaster::setStrategy(int addr, JZModbusStrategy s)
{
    if (!m_map.contains(addr))
        return;

    Strategy info;
    info.s = s;
    info.readTime = 0;
    m_strategyMap[addr] = info;
}

void JZModbusMaster::removeStrategy(int addr)
{
    m_strategyMap.remove(addr);
}

void JZModbusMaster::onModbusReply(const JZModebusReply &reply)
{
    if (reply.code != 0)
        return;

    if (reply.function == MODBUS_FC_READ_COILS || reply.function == MODBUS_FC_READ_DISCRETE_INPUTS
        || reply.function == MODBUS_FC_READ_HOLDING_REGISTERS || reply.function == MODBUS_FC_READ_INPUT_REGISTERS)
    {
        bool is_bit = (reply.function == MODBUS_FC_READ_COILS || reply.function == MODBUS_FC_READ_DISCRETE_INPUTS);
        int nb = is_bit ? reply.bit.size() : reply.reg.size();

        auto list = m_map.paramList();        
        int offset = 0;
        for (int addr = reply.addr; addr < reply.addr + nb; addr++, offset++)
        {
            for (int i = 0; i < list.size(); i++)
            {
                if (addr == list[i])
                {
                    auto param = m_map.param(addr);
                    QVariant pre = param->value;
                    if (is_bit)
                    {
                        param->value = reply.bit[offset];
                    }
                    else
                    {
                        const char *buffer = (const char *)(reply.reg.constData() + offset);
                        param->unpack(buffer);
                    }

                    auto it = m_strategyMap.find(addr);
                    if (it != m_strategyMap.end())
                    {
                        if (it->s.recvNotify)
                            emit sigParamReceived(addr);
                    }

                    if (pre != param->value)
                        emit sigParamChanged(addr);
                }
            }
        }
    }
    dealCommand();
}

void JZModbusMaster::timerEvent(QTimerEvent *e)
{
    dealCommand();
}

void JZModbusMaster::dealCommand()
{
    if (!m_client->isOpen() || m_client->isBusy())
        return;

    if (m_commandList.size() > 0)
    {
        auto cmd = m_commandList[0];
        m_commandList.pop_front();
        
        auto p = m_map.param(cmd.addr);
        if (cmd.type == Command::Write)
        {
            p->setValue(cmd.value);
            if (p->addrType == Param_Coil)
                m_client->writeBitAsync(p->addr, p->value.toInt());
            else
            {
                QVector<uint16_t> buffer;
                buffer.resize(p->byteSize() / 2);
                p->pack((char*)buffer.data());
                m_client->writeRegistersAsync(p->addr, buffer);
            }        
        }
        else if(cmd.type == Command::Read)
        {
            if (p->addrType == Param_Coil)
            {
                m_client->readBitsAsync(p->addr, 1);
            }
            else if (p->addrType == Param_DiscreteInput)
            {
                m_client->readInputBitsAsync(p->addr, 1);
            }
            else
            {
                QVector<uint16_t> buffer;
                int nb = p->byteSize() / 2;                
                if (p->addrType == Param_HoldingRegister)
                    m_client->readRegistersAsync(p->addr, nb);
                else if (p->addrType == Param_InputRegister)
                    m_client->readInputRegistersAsync(p->addr, nb);
            }
        }
        return;
    }

    qint64 cur_ms = QDateTime::currentMSecsSinceEpoch();

    auto it = m_strategyMap.begin();
    while (it != m_strategyMap.end())
    {
        if (it->s.autoRead && it->readTime + it->s.autoReadTime < cur_ms)
        {
            auto param = m_map.param(it.key());
            int nb = param->byteSize() / 2;
            m_client->readRegistersAsync(param->addr,nb);
            it->readTime = cur_ms;
            break;
        }
        it++;
    }    
}

void modbusMasterSetConfig(JZModbusMaster *master, const JZModbusConfig *c)
{
    auto map = master->map();
    map->clear();

    for (int i = 0; i < c->paramList.size(); i++)
        map->add(c->paramList[i]);

    if (c->isRtu)
    {
        master->initRtu(c->portName, c->baud, (QSerialPort::DataBits)c->dataBit, (QSerialPort::StopBits)c->stopBit, (QSerialPort::Parity)c->parityBit);
        master->setSlave(c->slave);
    }
    else
    {
        master->initTcp(c->ip, c->port);
        master->setSlave(c->slave);
    }

    auto it = c->strategyMap.begin();
    while (it != c->strategyMap.end())
    {
        master->setStrategy(it.key(), it.value());
        it++;
    }
}