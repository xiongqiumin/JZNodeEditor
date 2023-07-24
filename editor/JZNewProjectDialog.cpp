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
}

JZNewProjectDialog::~JZNewProjectDialog()
{
	delete ui;
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
        QMessageBox::information(this,tr("��ʾ"),tr("��Ŀ������Ŀ·������Ϊ��"));
		return;
	}
    if (QFile::exists(dir + "/" + name))
    {
        QMessageBox::information(this, tr("��ʾ"), tr("�ļ����Ѵ���"));
        return;
    }
	QDialog::accept();
}

void JZNewProjectDialog::on_btnCancel_clicked()
{
    QDialog::reject();
}