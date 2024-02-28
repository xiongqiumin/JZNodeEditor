#include "JZProjectStream.h"
#include <QJsonDocument>
#include <QFile>
#include <QTextStream>

//JZProjectStream
JZProjectStream::JZProjectStream()
{    
}

JZProjectStream::~JZProjectStream()
{

}

void JZProjectStream::clear()
{
    m_childs.clear();
}

bool JZProjectStream::contains(const QString &name)
{
    return (indexOfStream(name) >= 0);
}

QJsonValue JZProjectStream::value()
{    
    if (m_childs.size() > 0)
    {
        QJsonObject obj;
        for (int i = 0; i < m_childs.size(); i++)
        {
            auto ptr = m_childs[i].data();
            obj[ptr->m_name] = ptr->value();
        }
        return obj;
    }
    return m_value;
}

void JZProjectStream::setValue(const QJsonValue &value)
{    
    m_value = value;
}

int JZProjectStream::indexOfStream(const QString &name)
{
    for (int i = 0; i < m_childs.size(); i++)
    {
        if (m_childs[i]->m_name == name)
            return i;
    }
    return -1;
}

JZProjectStream &JZProjectStream::operator[](const QString &name)
{
    auto index = indexOfStream(name);
    if (index >= 0)
        return *m_childs[index];

    auto s = new JZProjectStream();
    s->m_name = name;
    if (m_value.isObject())
        s->m_value = m_value.toObject()[name];
    
    m_childs.push_back(QSharedPointer<JZProjectStream>(s));
    return *m_childs.back();
}

void operator<<(JZProjectStream& s, int value)
{
    Q_ASSERT(s.value().isNull());
    s.setValue(value);
}

void operator<<(JZProjectStream& s, double value)
{
    Q_ASSERT(s.value().isNull());
    s.setValue(value);
}

void operator<<(JZProjectStream& s, const QString &value)
{
    Q_ASSERT(s.value().isNull());
    s.setValue(value);
}

void operator>>(JZProjectStream& s, int &value)
{
    if (s.value().isNull())
        return;
    value = s.value().toInt();
}

void operator>>(JZProjectStream& s, double &value)
{
    if (s.value().isNull())
        return;
    value = s.value().toDouble();
}

void operator>>(JZProjectStream& s, QString &value)
{
    if (s.value().isNull())
        return;
    value = s.value().toString();
}

//JZProjectFile
JZProjectFile::JZProjectFile()
{
}

JZProjectFile::~JZProjectFile()
{    
}

bool JZProjectFile::load(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        m_error = "can't open file";
        return false;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError)
    {
        m_error = "parse error," + QString::number(error.offset) + " " + error.errorString();
        return false;
    }

    m_stream.clear();
    m_stream.setValue(doc.object());
    return true;
}

bool JZProjectFile::save(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        m_error = "can't open file";
        return false;
    }

    QTextStream s(&file);
    QJsonDocument doc(m_stream.value().toObject());
    QString text = QString::fromUtf8(doc.toJson());
    s.setCodec("utf-8");
    s << text;
    return true;
}

QString JZProjectFile::error()
{
    return m_error;
}

JZProjectStream &JZProjectFile::stream()
{
    return m_stream;
}