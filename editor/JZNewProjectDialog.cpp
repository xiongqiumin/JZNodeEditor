#include "JZNewProjectDialog.h"
#include "ui_JZNewProjectDialog.h"
#include <QFileDialog>
#include <QMessageBox>

JZNewProjectDialog::JZNewProjectDialog(QWidget *parent)
    :QDialog(parent)
{
    ui = new Ui::JZNewProjectDialog();

    ui->setupUi(this);
	ui->lineProjectName->setText("project");
	ui->lineProjectDir->setText(QDir::cleanPath(QApplication::applicationDirPath() + "/project"));

    ui->listWidget->addItem("Ui界面程序");
    ui->listWidget->addItem("命令行程序");

    ui->listWidget->setCurrentRow(0);
}

JZNewProjectDialog::~JZNewProjectDialog()
{
	delete ui;
}

int JZNewProjectDialog::projectType()
{
    return ui->listWidget->currentRow();
}

QString JZNewProjectDialog::name()
{
	return ui->lineProjectName->text();
}

QString JZNewProjectDialog::dir()
{
	return ui->lineProjectDir->text();
}

void JZNewProjectDialog::on_btnSelect_clicked()
{
    QString path = QFileDialog::getExistingDirectory();
    if (path.isEmpty())
        return;

    ui->lineProjectDir->setText(path);
}

void JZNewProjectDialog::on_btnOk_clicked()
{
	QString name = this->name();
	QString dir = this->dir();
	if (name.isEmpty() || dir.isEmpty())
    {
        QMessageBox::information(this,tr("提示"),tr("项目名和项目路径不能为空"));
		return;
	}
    if (QFile::exists(dir + "/" + name))
    {
        QMessageBox::information(this, tr("提示"), tr("文件夹已存在"));
        return;
    }
	QDialog::accept();
}

void JZNewProjectDialog::on_btnCancel_clicked()
{
    QDialog::reject();
}