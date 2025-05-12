#include <QListWidget>
#include "JZNodeListBind.h"

//ListDelegate
ListDelegate::ListDelegate()
{
	m_bind = nullptr;
	m_list = nullptr;
	m_listWidget = nullptr;
}

ListDelegate::~ListDelegate()
{
}

void ListDelegate::set(int idx, const QVariant &v)
{
	m_listWidget->item(idx)->setText(v.toString());
}

QVariant ListDelegate::get(int idx)
{
	return QVariant();
}

int ListDelegate::size()
{
	return 0;
}

void ListDelegate::clear()
{
}

void ListDelegate::insert(int index, const QVariant& t)
{
}

void ListDelegate::push_back(int index, const QVariant& t)
{
	QListWidgetItem* item = new QListWidgetItem();
	m_listWidget->addItem(item);
}

void ListDelegate::pop_back(const QVariant& t)
{
	int n = size();
	delete m_listWidget->takeItem(n - 1);
}

void ListDelegate::push_front(const QVariant& t)
{
	QListWidgetItem* item = new QListWidgetItem();
	m_listWidget->insertItem(0,item);
}

void ListDelegate::pop_front()
{
	delete m_listWidget->takeItem(0);
}

void ListDelegate::removeAt(int idx)
{
	delete m_listWidget->takeItem(idx);
}

QVariant ListDelegate::mid(int idx, int len)
{
	return QVariant();
}

void ListDelegate::append(int idx, const QVariant& v)
{
	for (int i = 0; i < 100; i++)
	{
		QListWidgetItem* item = new QListWidgetItem();
		m_listWidget->addItem(item);
	}
}

void ListDelegate::resize(int new_size)
{
	int cur_size = 0;
	if (cur_size > new_size)
	{
		for (int i = cur_size - 1; i > new_size; i--)
		{
			delete m_listWidget->takeItem(i);
		}
	}
	else if (cur_size < new_size)
	{
		for (int i = cur_size; i < new_size; i++)
		{
			QListWidgetItem* item = new QListWidgetItem();
			m_listWidget->addItem(item);
		}
	}
}

void ListDelegate::swap(int i, int j)
{
	// 获取当前选中项
	int currentRow = m_listWidget->currentRow();

	// 保存选中状态
	bool isSelectedI = m_listWidget->item(i)->isSelected();
	bool isSelectedJ = m_listWidget->item(j)->isSelected();

	// 获取两个item的文本和数据
	QListWidgetItem* itemI = m_listWidget->takeItem(i);
	QListWidgetItem* itemJ = m_listWidget->takeItem(j < i ? j : j - 1);

	// 交换位置插入
	m_listWidget->insertItem(i, itemJ);
	m_listWidget->insertItem(j, itemI);

	// 恢复选中状态
	m_listWidget->item(i)->setSelected(isSelectedJ);
	m_listWidget->item(j)->setSelected(isSelectedI);

	// 恢复当前选中行
	if (currentRow == i) {
		m_listWidget->setCurrentRow(j);
	}
	else if (currentRow == j) {
		m_listWidget->setCurrentRow(i);
	}
	else if (currentRow >= 0) {
		m_listWidget->setCurrentRow(currentRow);
	}
}

//ListWidgetBind
ListWidgetBind::ListWidgetBind()
{
}

ListWidgetBind::~ListWidgetBind()
{
}

QListWidget* ListWidgetBind::listWidget()
{
	return qobject_cast<QListWidget*>(m_widget);
}

void ListWidgetBind::bind(QWidget* widget, JZNodeObject* object, QString prop)
{
	JZBindObject::bind(widget, object, prop);
}

void ListWidgetBind::uiToDataImpl()
{
	QVariant value;

	QListWidget *list_widget = listWidget();
	for (int i = 0; i < list_widget->count(); i++)
	{
		QListWidgetItem* item = list_widget->item(i);
	}

	m_context->setParam(m_param, value);
}

void ListWidgetBind::dataToUiImpl()
{
	QListWidget* list_widget = listWidget();
	list_widget->clear();

	for (int i = 0; i < 100; i++)
	{
		QListWidgetItem* item = new QListWidgetItem();
		list_widget->addItem(item);
	}
}