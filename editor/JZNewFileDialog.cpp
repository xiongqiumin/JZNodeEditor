#include <QDir>
#include "JZRegExpHelp.h"
#include "JZNewFileDialog.h"
#include "ui_JZNewFileDialog.h"

JZNewFileDialog::JZNewFileDialog(QWidget *p)
    :QDialog(p)
{
    ui = new Ui::JZNewFileDialog();
    ui->setupUi(this);

    ui->listWidget->addItem("file");
    ui->listWidget->addItem("class");
    ui->listWidget->addItem("ui class");        

    ui->listWidget->item(0)->setData(Qt::UserRole,NewFile);
    ui->listWidget->item(1)->setData(Qt::UserRole,NewClass);
    ui->listWidget->item(2)->setData(Qt::UserRole,NewUiClass);

    ui->listWidget->setCurrentRow(0);
}

JZNewFileDialog::~JZNewFileDialog()
{
    delete ui;
}

void JZNewFileDialog::init(QString path)
{    
    QDir project_dir(path);
    QStringList fileList;
    auto fileInfoList = project_dir.entryInfoList({"*.jz"},QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    for(int i = 0; i < fileList.size(); i++)
        fileList << fileInfoList[i].baseName();

    QString new_name = JZRegExpHelp::uniqueString("Class",fileList);
    ui->lineName->setText(new_name);
    ui->lineDir->setText(path);
}   

QString JZNewFileDialog::name()
{
    return ui->lineName->text();
}

QString JZNewFileDialog::path()
{
    return ui->lineDir->text();
}

int JZNewFileDialog::type()
{
    return ui->listWidget->currentItem()->data(Qt::UserRole).toInt();
}

void JZNewFileDialog::on_btnOk_clicked()
{
    QDialog::accept();
}

void JZNewFileDialog::on_btnCancel_clicked()
{
    QDialog::reject();
}