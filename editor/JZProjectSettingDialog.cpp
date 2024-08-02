#include <QFileDialog>
#include <QMessageBox>
#include <QToolBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include "JZProjectSettingDialog.h"
#include "JZContainer.h"

JZProjectSettingDialog::JZProjectSettingDialog(QWidget *parent)
    :JZBaseDialog(parent)
{
    m_project = nullptr;

    m_moduleEdit = new QTextEdit();
	m_containerEdit = new QTextEdit();

    m_tree = new QTreeWidget();
    m_tree->setColumnCount(1);
    m_tree->setHeaderHidden(true);

    QTreeWidgetItem *item_root = new QTreeWidgetItem();
    item_root->setText(0,"ÅäÖÃÊôÐÔ");
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
    m_stackWidget->addWidget(addPage(m_containerEdit,"Ìí¼ÓÈÝÆ÷"));
    m_stackWidget->addWidget(addPage(m_moduleEdit,"Ìí¼ÓÄ£¿é"));

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
    m_moduleEdit->setPlainText(module_list.join("\n"));
    m_containerEdit->setPlainText(container_list.join("\n"));
}

bool JZProjectSettingDialog::onOk()
{
    auto module_list = m_project->moduleList();
    auto container_list =  m_project->containerList();
    for(int i = 0; i < container_list.size(); i++)
        m_project->unregistContainer(container_list[i]);

    for(int i = 0; i < module_list.size(); i++)
        m_project->unimportModule(module_list[i]);

    module_list = m_moduleEdit->toPlainText().split("\n");
    container_list = m_containerEdit->toPlainText().split("\n");
    for(int i = 0; i < container_list.size(); i++)
    {
        QString line = container_list[i].trimmed();
        if(!line.isEmpty())
            m_project->registContainer(line);
    }

    for(int i = 0; i < container_list.size(); i++)
    {
        QString line = container_list[i].trimmed();
        if(!line.isEmpty())
            m_project->importModule(container_list[i]);
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