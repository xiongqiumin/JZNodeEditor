#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include "JZModuleConfigWidget.h"

//JZModuleConfigWidget
JZModuleConfigWidget::JZModuleConfigWidget(QWidget *parent)
    : QWidget(parent)
{
    m_table = new QTableWidget();
    
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setEditTriggers(QTableWidget::NoEditTriggers);
    connect(m_table, &QTableWidget::itemDoubleClicked, this, &JZModuleConfigWidget::onItemDoubleClicked);

    QVBoxLayout *l = new QVBoxLayout();
    QHBoxLayout *h = new QHBoxLayout();

    QPushButton *btnAdd = new QPushButton("添加");
    QPushButton *btnRemove = new QPushButton("删除");
    QPushButton *btnSetting = new QPushButton("设置");
    
    connect(btnAdd,&QPushButton::clicked,this, &JZModuleConfigWidget::onBtnAddClicked);
    connect(btnRemove,&QPushButton::clicked,this, &JZModuleConfigWidget::onBtnRemoveClicked);
    connect(btnSetting,&QPushButton::clicked,this, &JZModuleConfigWidget::onBtnSettingClicked);

    h->addStretch();
    h->addWidget(btnAdd);
    h->addWidget(btnRemove);
    h->addWidget(btnSetting);
    h->setContentsMargins(0,0,0,0);
    h->setSpacing(3);
    
    l->addWidget(m_table);
    l->addLayout(h);

    setLayout(l);
    resize(600, 400);
}

JZModuleConfigWidget::~JZModuleConfigWidget()
{
}

void JZModuleConfigWidget::onBtnAddClicked()
{
    addConfig();
}

void JZModuleConfigWidget::onBtnRemoveClicked()
{
    int idx = m_table->currentRow();
    if (idx == -1)
        return;

    removeConfig(idx);
}

void JZModuleConfigWidget::onItemDoubleClicked(QTableWidgetItem *item)
{
    int row = item->row();
    settingConfig(row);
}

void JZModuleConfigWidget::onBtnSettingClicked()
{
    int idx = m_table->currentRow();
    if (idx == -1)
        return;

    settingConfig(idx);
}