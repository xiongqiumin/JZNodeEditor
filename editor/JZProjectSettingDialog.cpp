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

    QWidget *btn_box = new QWidget();
    QVBoxLayout *box_l = new QVBoxLayout();
    btn_box->setLayout(box_l);
    box_l->setContentsMargins(0,0,0,0);

    auto btn1 = new QPushButton("container");
    auto btn2 = new QPushButton("module");
    connect(btn1,&QPushButton::clicked,this, &onBtnNagtiveClicked);
    connect(btn2,&QPushButton::clicked,this, &onBtnNagtiveClicked);

    btn1->setProperty("index",0);
    btn2->setProperty("index",1);
    box_l->addWidget(btn1);
    box_l->addWidget(btn2);
    box_l->addStretch();

    m_stackWidget = new QStackedWidget();
    m_stackWidget->addWidget(addTitle(m_containerEdit,"container",""));
    m_stackWidget->addWidget(addTitle(m_moduleEdit,"module",""));

    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0,0,0,0);
    l->addWidget(btn_box);
    l->addWidget(m_stackWidget);
    m_mainWidget->setLayout(l);
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

void JZProjectSettingDialog::onBtnNagtiveClicked()
{
    int index = sender()->property("index").toInt();
    m_stackWidget->setCurrentIndex(index);
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

QWidget *JZProjectSettingDialog::addTitle(QWidget *main,QString title,QString help)
{
    QWidget *w = new QWidget();
    QVBoxLayout *w_l = new QVBoxLayout();
    w->setLayout(w_l);
    w_l->setContentsMargins(0,0,0,0);

    w_l->addWidget(new QLabel(title));
    w_l->addWidget(main);
    w_l->addWidget(new QLabel(help));

    return w;
}