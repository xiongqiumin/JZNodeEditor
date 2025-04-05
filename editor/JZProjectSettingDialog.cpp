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

//JZProjectSettingDialog
JZProjectSettingDialog::JZProjectSettingDialog(QWidget *parent)
    :JZBaseDialog(parent)
{
    m_project = nullptr;
    
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
}

bool JZProjectSettingDialog::onOk()
{   
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