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

	void saveToStream(QDataStream& s, const QSharedPointer<T>& ptr)
	{
		s << (int)ptr->type;
		ptr->saveToStream(s);
	}
	void loadFromStream(QDataStream& s, QSharedPointer<T>& ptr)
	{
		int type = 0;
		s >> type;
		ptr = QSharedPointer<T>(m_creator[type]());
		ptr->loadFromStream(s);
	}

protected:
	QMap<int, std::function<T*()>> m_creator;
};

#endif // ! JZ_Shared_Pointer_Factory_H_
