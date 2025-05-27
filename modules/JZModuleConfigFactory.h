#ifndef JZ_MODULE_CONFIG_Factory_H_
#define JZ_MODULE_CONFIG_Factory_H_

#include <QSharedPointer>
#include <functional>
#include <QMap>

template<class T>
T *JZModuleConfigCreator()
{
	return new T();
}

template<class T>
class JZModuleConfigEnum
{
public:
    JZModuleConfigEnum()
    {
        m_ptr = nullptr;
    }

    JZModuleConfigEnum(T *ptr)
    {
        m_ptr = ptr;
    }

    JZModuleConfigEnum(const JZModuleConfigEnum &other);
    
    T* operator->() const
    {
        return data();
    }
    void operator=(const JZModuleConfigEnum &other);

    ~JZModuleConfigEnum()
    {
        clear();
    }

    bool isNull() const
    {
        return (m_ptr == nullptr);
    }

    T *data() const
    {
        return m_ptr;
    }

    void clear()
    {
        if (m_ptr)
        {
            delete m_ptr;
            m_ptr = nullptr;
        }
    }

protected:    
    T *m_ptr;
};

template<class T>
class JZModuleConfigFactory
{
public:
	static JZModuleConfigFactory* instance()
	{
		static JZModuleConfigFactory inst;
		return &inst;
	}

	void regist(int type, std::function<T* ()> creator)
	{
		Q_ASSERT(!m_creator.contains(type));
		m_creator[type] = creator;
	}

    T *create(int type)
    {
        return m_creator[type]();
    }
    
    void copyTo(const T *src, T *dst)
    {
        Q_ASSERT(src->type == dst->type);

        QByteArray buffer;
        QDataStream write(&buffer, QIODevice::WriteOnly);
        src->saveToStream(write);

        QDataStream read(&buffer, QIODevice::ReadOnly);
        dst->loadFromStream(read);
    }

	void saveToStream(QDataStream& s, const JZModuleConfigEnum<T>& ptr)
	{
		s << (int)ptr->type;
		ptr->saveToStream(s);
	}

	void loadFromStream(QDataStream& s, JZModuleConfigEnum<T>& ptr)
	{
		int type = 0;
		s >> type;
		ptr = JZModuleConfigEnum<T>(m_creator[type]());
		ptr->loadFromStream(s);
	}

protected:
	QMap<int, std::function<T*()>> m_creator;
};

template<class T>
JZModuleConfigEnum<T>::JZModuleConfigEnum(const JZModuleConfigEnum<T> &other)
{
    m_ptr = nullptr;
    *this = other;
}

template<class T>
void JZModuleConfigEnum<T>::operator=(const JZModuleConfigEnum<T> &other)
{
    if (other.isNull())
    {
        clear();
        return;
    }

    if (!m_ptr)
        m_ptr = JZModuleConfigFactory<T>::instance()->create(other.data()->type);

    JZModuleConfigFactory<T>::instance()->copyTo(other.data(), m_ptr);
}

#endif // ! JZ_Shared_Pointer_Factory_H_
