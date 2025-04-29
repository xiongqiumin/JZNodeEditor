#ifndef JZNODESETTINGDIALOG_H
#define JZNODESETTINGDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include <QTabWidget>
#include <QTableWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class JZNodeManagerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit JZNodeManagerDialog(QWidget *parent = nullptr);
    ~JZNodeManagerDialog();
    
private slots:
    void onBtnAddClicked();
    void onBtnRemoveClicked();  
    void onBtnOkClicked();  
    void onBtnCancelClicked();  

private:
    QTableWidget *m_table;
};


class JZNodeSettingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit JZNodeSettingDialog(const QJsonObject& json, QWidget *parent = nullptr);
    ~JZNodeSettingDialog();

    void setValue(const QJsonObject& json);
    QJsonObject value() const;

private slots:

private:
};

#endif // JZNODESETTINGDIALOG_H    