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

void SampleProject::saveProject()
{
    Q_ASSERT(!m_name.isEmpty());
    projectUpdateLayout(&m_project);

    QString dir = qApp->applicationDirPath() + "/sample";
    if (!QDir().exists(dir))
        QDir().mkdir(dir);
    QString path = qApp->applicationDirPath() + "/sample/" + m_name + ".jzproject";
    m_project.saveAs(path);
}

bool SampleProject::run()
{
    QString program_path = qApp->applicationDirPath() + "/sample/build/" + m_name + ".program";
    QString jsm_path = qApp->applicationDirPath() + "/sample/build/" + m_name + ".jsm";

    JZNodeBuilder builder;
    JZNodeProgram program;
    if (!builder.build(&m_project, &program))
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