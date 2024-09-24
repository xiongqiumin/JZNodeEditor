#include <QJsonDocument>
#include <QFile>
#include <QTextStream>
#include "JZNode.h"
#include "JZProjectStream.h"

//QByteArray
QByteArray ProjectMagic()
{
    QByteArray result;
    QDataStream s(&result, QIODevice::WriteOnly);
    s << QString("2");
    //node
    s << sizeof(JZNodePin);
    s << sizeof(JZNodeGemo);
    s << sizeof(JZNodeConnect);
    s << sizeof(JZNode);

    return result;
}

//JZProjectFile
JZProjectFile::JZProjectFile()
{
}

JZProjectFile::~JZProjectFile()
{    
    close();
}

bool JZProjectFile::openLoad(const QString &filename)
{
    m_file.setFileName(filename);
    if (!m_file.open(QFile::ReadOnly))
    {
        m_error = "can't open file";
        return false;
    }

    QByteArray magic;
    m_stream.setDevice(&m_file);
    m_stream >> magic;
    if(magic != ProjectMagic())
    {
        m_error = "version not match";
        m_file.close();
        return false;
    }

    return true;
}

bool JZProjectFile::openSave(const QString &filename)
{
    m_file.setFileName(filename);
    if (!m_file.open(QFile::WriteOnly | QFile::Truncate))
    {
        m_error = "can't open file";
        return false;
    }

    m_stream.setDevice(&m_file);
    m_stream << ProjectMagic();
    return true;
}

void JZProjectFile::close()
{
    m_file.close();
}

QString JZProjectFile::error()
{
    return m_error;
}

QDataStream &JZProjectFile::stream()
{
    return m_stream;
}