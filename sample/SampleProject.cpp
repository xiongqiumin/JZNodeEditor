#include <QApplication>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include "SampleProject.h"
#include "JZNodeBuilder.h"
#include "JZNodeVM.h"
#include "JZNodeUtils.h"
#include "JZUiItem.h"
#include "JZNodeProgramDumper.h"
#include "JZEditorGlobal.h"
#include "JZProjectTemplate.h"
#include "JZEditorUtils.h"

SampleProject::SampleProject()
{    
    m_objInst = m_project.environment()->objectManager();
    m_funcInst = m_project.environment()->functionManager();
}

SampleProject::~SampleProject()
{

}

QString SampleProject::loadUi(QString filename)
{    
    QString filepath = m_root + "/" + filename;
    QFile file(filepath);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        Q_ASSERT(0);
        return QString();
    }

    return QString::fromUtf8(file.readAll());
}

void SampleProject::newProject(QString name, QString template_name)
{
    Q_ASSERT(!m_root.isEmpty());
    
    m_name = name;
    QString dir = qApp->applicationDirPath() + "/sample/" + m_name;
    if (!QDir().exists(dir))
        QDir().mkpath(dir);

    QString project_path = dir + "/" + m_name + ".jzproj";
        
    JZProjectTemplate::instance()->initProject(&m_project, template_name);
    
    m_project.saveAllItem();
    m_project.saveAs(project_path);
}

void SampleProject::addResources(QString name)
{
    m_resources = m_root + "/" + name;
}

bool SampleProject::copyDir(QString srcPath, QString dstPath)
{
    QDir dir(dstPath);
    if (!dir.exists())
        dir.mkdir("./");

    bool error = false;
    QStringList fileNames = QDir(srcPath).entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    for (int i = 0; i != fileNames.size(); ++i)
    {
        QString fileName = fileNames.at(i);
        QString srcFilePath = srcPath + "/" + fileName;
        QString dstFilePath = dstPath + "/" + fileName;

        QFileInfo fileInfo(srcFilePath);
        if (fileInfo.isFile() || fileInfo.isSymLink())
        {
            QFile::copy(srcFilePath, dstFilePath);
        }
        else if (fileInfo.isDir())
        {                        
            if (!copyDir(srcFilePath, dstFilePath))
            {
                error = true;
            }
        }
    }

    return !error;    
}

JZProject *SampleProject::project()
{
    return &m_project;
}

void SampleProject::loadProject()
{
    QString path = qApp->applicationDirPath() + "/sample/" + m_name + "/" + m_name + ".jzproj";
    m_project.open(path);
}

void SampleProject::saveProject()
{
    Q_ASSERT(!m_name.isEmpty());    
    JZEditorUtils::projectUpdateLayout(&m_project);
    m_project.saveAllItem();
    if (!m_project.save())
    {
        qDebug() << "save to" << m_project.path() + "failed";
        return;
    }
    
    if (!m_resources.isEmpty())
    {
        QFileInfo info(m_resources);
        copyDir(m_resources, m_project.path() + "/" + info.fileName());
    }
}

int SampleProject::run()
{    
    QString program_path = m_project.path() + "/build/" + m_name + ".program";
    QString dump_path = m_project.path() + "/build/dump";
    if (!QFile::exists(m_project.path() + "/build"))
        QDir().mkdir(m_project.path() + "/build");
    if (!QFile::exists(dump_path))
        QDir().mkdir(dump_path);

    JZNodeBuilder builder;
    builder.setProject(&m_project);

    JZNodeProgram program;
    if (!builder.build(&program))
    {
        qDebug().noquote() << "build failed\n" << builder.error();
        return 1;
    }
    QDir::setCurrent(m_project.path());

    //save asm    
    JZNodeProgramDumper dumper;
    dumper.init(&m_project, &program);
    dumper.dump(dump_path);    

    //save bin
    if (!program.save(program_path))
    {
        qDebug() << "save failed";
        return 1;
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