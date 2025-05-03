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
#include "jzWidgets/JZPropertyEditor.h"
#include "JZBaseDialog.h"

class JZManagerPropertyDialog : public JZBaseDialog
{
    Q_OBJECT

public:
    JZManagerPropertyDialog(QWidget *parent = nullptr);

protected slots:
    void onPropChanged(JZProperty * prop, const QVariant &v);

protected:
    void addPage(int type,QList<JZProperty*> propList);
    void switchPage(int page);

    JZProperty *m_typeProp;
    JZPropertyEditor *m_editor;
    QMap<int, QList<JZProperty*>> m_propType;
};


class JZNodeManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JZNodeManagerDialog(QWidget *parent = nullptr);
    ~JZNodeManagerDialog();
    
protected slots:
    void onBtnAddClicked();
    void onBtnRemoveClicked();  
    void onBtnSettingClicked();
    void onBtnOkClicked();  
    void onBtnCancelClicked();  

    void onItemDoubleClicked(QTableWidgetItem *item);

protected:
    virtual void addConfig() = 0;
    virtual void removeConfig(int index) = 0;
    virtual void settingConfig(int index) = 0;
    virtual void updateConfig() = 0;

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

protected slots:

protected:
};

#endif // JZNODESETTINGDIALOG_H    