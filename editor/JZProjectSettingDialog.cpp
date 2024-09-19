#include <QFileDialog>
#include <QMessageBox>
#include <QToolBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include "JZProjectSettingDialog.h"
#include "JZContainer.h"
#include "JZModule.h"

ModuleEdit::ModuleEdit()
{
    auto list = JZModuleManager::instance()->moduleList();
    QGridLayout *grid = new QGridLayout();
    for (int i = 0; i < list.size(); i++)
    {
        QCheckBox *box = new QCheckBox(list[i]);
        grid->addWidget(box,i/3,i%3);
        m_checkList.push_back(box);
    }
    if(list.size() > 0)
        grid->setRowStretch((list.size()-1)/3 + 1, 1);
    setLayout(grid);
}

QStringList ModuleEdit::getModule()
{
    QStringList ret;
    for (int i = 0; i < m_checkList.size(); i++)
    {
        if (m_checkList[i]->isChecked())
            ret << m_checkList[i]->text();
    }
    return ret;
}

void ModuleEdit::setModule(QStringList module)
{    
    for (int i = 0; i < m_checkList.size(); i++)
    {
        for (int j = 0; j < module.size(); j++)
        {
            if (m_checkList[i]->text() == module[j])
                m_checkList[i]->setChecked(true);
        }
    }
}

//JZProjectSettingDialog
JZProjectSettingDialog::JZProjectSettingDialog(QWidget *parent)
    :JZBaseDialog(parent)
{
    m_project = nullptr;

    m_moduleEdit = new ModuleEdit();
	m_containerEdit = new QTextEdit();

    m_tree = new QTreeWidget();
    m_tree->setColumnCount(1);
    m_tree->setHeaderHidden(true);

    QTreeWidgetItem *item_root = new QTreeWidgetItem();
    item_root->setText(0,"设置");
    m_tree->addTopLevelItem(item_root);
    m_tree->setFixedWidth(200);
    connect(m_tree, &QTreeWidget::itemClicked, this, &JZProjectSettingDialog::onTreeItemClicked);

    QTreeWidgetItem *item_c = new QTreeWidgetItem();
    QTreeWidgetItem *item_m = new QTreeWidgetItem();
    item_c->setText(0, "container");
    item_m->setText(0, "module");

    item_c->setData(0, Qt::UserRole, 0);
    item_m->setData(0, Qt::UserRole, 1);

    item_root->addChild(item_c);
    item_root->addChild(item_m);    

    m_stackWidget = new QStackedWidget();
    m_stackWidget->addWidget(addPage(m_containerEdit,"容器"));
    m_stackWidget->addWidget(addPage(m_moduleEdit,"模块"));

    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0,0,0,0);
    l->addWidget(m_tree);
    l->addWidget(m_stackWidget);    
    m_mainWidget->setLayout(l);
    m_tree->expandAll();

    resize(800, 600);
}

JZProjectSettingDialog::~JZProjectSettingDialog()
{
}

void JZProjectSettingDialog::setProject(JZProject *project)
{
    m_project = project;

    auto module_list = m_project->moduleList();
    auto container_list =  m_project->containerList();
    m_moduleEdit->setModule(module_list);
    m_containerEdit->setPlainText(container_list.join("\n"));
}

bool JZProjectSettingDialog::onOk()
{
    auto new_module_list = m_moduleEdit->getModule();
    auto new_container_list = m_containerEdit->toPlainText().split("\n");
    auto old_module_list = m_project->moduleList();
    auto old_container_list =  m_project->containerList();

    //unregist
    for (int i = 0; i < old_container_list.size(); i++)
    {
        QString name = old_container_list[i];
        if (!new_container_list.contains(name))
            m_project->unregistContainer(name);
    }

    for (int i = 0; i < old_module_list.size(); i++)
    {
        QString name = old_module_list[i];
        if(!new_module_list.contains(name))
            m_project->unimportModule(name);
    }

    //regist
    for(int i = 0; i < new_container_list.size(); i++)
    {
        QString name = new_container_list[i].trimmed();
        if(!name.isEmpty() && !old_container_list.contains(name))
            m_project->registContainer(name);
    }

    for(int i = 0; i < new_module_list.size(); i++)
    {
        QString name = new_module_list[i];
        if (!old_module_list.contains(name))
            m_project->importModule(name);
    }
    m_project->save();
    return true;
}

void JZProjectSettingDialog::onTreeItemClicked(QTreeWidgetItem *current, int col)
{
    QVariant v = current->data(0, Qt::UserRole);
    if (v.isNull())
        return;

    int index = v.toInt();
    m_stackWidget->setCurrentIndex(index);
}

QWidget *JZProjectSettingDialog::addPage(QWidget *main,QString help)
{
    QWidget *w = new QWidget();
    QVBoxLayout *w_l = new QVBoxLayout();
    w->setLayout(w_l);
    w_l->setContentsMargins(0,0,0,0);    
    w_l->addWidget(main);
    w_l->addWidget(new QLabel(help));

    return w;
}