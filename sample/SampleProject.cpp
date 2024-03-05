#include <QApplication>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include "SampleProject.h"
#include "JZNodeBuilder.h"
#include "JZNodeVM.h"
#include "JZNodeUtils.h"

SampleProject::SampleProject()
{

}

SampleProject::~SampleProject()
{

}

QString SampleProject::loadUi(QString filename)
{    
    QString filepath = "sample/" + filename;
    QFile file(filepath);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        Q_ASSERT(0);
        return QString();
    }

    return QString::fromUtf8(file.readAll());
}

void SampleProject::newProject(QString name)
{
    m_name = name;

    QString dir = qApp->applicationDirPath() + "/sample";
    if (!QDir().exists(dir))
        QDir().mkdir(dir);
        
    m_project.newProject(dir, m_name, "ui");
}

void SampleProject::saveProject()
{
    Q_ASSERT(!m_name.isEmpty());
    projectUpdateLayout(&m_project);
    m_project.saveAllItem();
    m_project.save();

    
}

bool SampleProject::run()
{    
    QString program_path = m_project.path() + "/build/" + m_name + ".program";
    QString jsm_path = m_project.path() + "/build/" + m_name + ".jsm";
    if (!QFile::exists(m_project.path() + "/build"))
        QDir().mkdir(m_project.path() + "/build");

    JZNodeBuilder builder(&m_project);

    JZNodeProgram program;
    if (!builder.build(&program))
    {
        qDebug().noquote() << builder.error();
        return false;
    }
    //save asm
    QFile file(jsm_path);
    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream s(&file);
        s << program.dump();
        file.close();
    }
    //save bin
    if (!program.save(program_path))
    {
        qDebug() << "save failed";
        return false;
    }

    JZNodeVM vm;
    QString error;
    if (!vm.init(program_path, false, error))
    {
        QMessageBox::information(nullptr, "", "init program \"" + program_path + "\" failed\n" + error);
        return 1;
    }
    return qApp->exec();
}