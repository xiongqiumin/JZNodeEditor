#include <QApplication>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include "SampleProject.h"
#include "JZNodeBuilder.h"
#include "JZNodeVM.h"
#include "JZNodeUtils.h"
#include "JZUiFile.h"
#include "JZNodeProgramDumper.h"

SampleProject::SampleProject()
{
    QFileInfo info(__FILE__);
    m_root = info.path();

    m_objInst = m_project.environment()->objectManager();
    m_funcInst = m_project.environment()->functionManager();
}

SampleProject::~SampleProject()
{

}

QString SampleProject::loadUi(QString filename)
{    
    QString filepath = m_root + "/" + m_name + "/" + filename;
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

    QString dir = qApp->applicationDirPath() + "/sample/" + m_name;
    if (!QDir().exists(dir))
        QDir().mkpath(dir);
        
    m_project.newProject(dir, m_name, "ui");
}

void SampleProject::addClassFile(QString class_name, QString super, QString ui_file)
{
    JZUiFile *file_ui = new JZUiFile();
    file_ui->setName(class_name + ".ui");
    file_ui->setXml(loadUi(ui_file));
    m_project.addItem("./", file_ui);    

    JZScriptFile *file = new JZScriptFile();
    file->setName(class_name + ".jz");
    m_project.addItem("./", file);

    auto class_item = file->addClass(class_name, super);
    if (!ui_file.isEmpty())
        class_item->setUiFile(ui_file);    
}

void SampleProject::addResources(QString name)
{
    m_resources = m_root + "/" + m_name + "/" + name;
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
    JZNodeUtils::projectUpdateLayout(&m_project);
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

bool SampleProject::run()
{    
    QString program_path = m_project.path() + "/build/" + m_name + ".program";
    QString jsm_path = m_project.path() + "/build/" + m_name + ".jsm";
    if (!QFile::exists(m_project.path() + "/build"))
        QDir().mkdir(m_project.path() + "/build");

    JZNodeBuilder builder;
    builder.setProject(&m_project);

    JZNodeProgram program;
    if (!builder.build(&program))
    {
        qDebug().noquote() << builder.error();
        return false;
    }
    QDir::setCurrent(m_project.path());

    //save asm
    QFile file(jsm_path);
    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        JZNodeProgramDumper dumper;

        QTextStream s(&file);
        s << dumper.dump(&program);
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