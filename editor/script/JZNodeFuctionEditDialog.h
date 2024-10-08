﻿#pragma once

#include <QDialog>
#include <QTableWidget>
#include "JZNodeFunction.h"
#include "JZScriptItem.h"

namespace Ui { class JZNodeFuctionEditDialog; }

class JZNodeFuctionEditDialog : public QDialog
{
    Q_OBJECT

public:
    JZNodeFuctionEditDialog(QWidget *parent = Q_NULLPTR);
    ~JZNodeFuctionEditDialog();
    
    void init(JZProject *project);    
    void setFunctionInfo(JZFunctionDefine info,bool newFunction);
    JZFunctionDefine functionInfo();    

protected slots:    
    void on_btnInUp_clicked();
    void on_btnInDown_clicked();
    void on_btnInAdd_clicked();
    void on_btnInRemove_clicked();

    void on_btnOutUp_clicked();
    void on_btnOutDown_clicked();
    void on_btnOutAdd_clicked();
    void on_btnOutRemove_clicked();

    void on_btnReset_clicked();
    void on_btnOk_clicked();
    void on_btnCancel_clicked();
    
private:
    void add(QTableWidget *table);
    void remove(QTableWidget *table);
    void up(QTableWidget *table);
    void down(QTableWidget *table);
    void swap(QTableWidget *table,int row0,int row1);
    void dataToTable(const QList<JZParamDefine> &param,QTableWidget *table);
    void tableToData(QTableWidget *table,QList<JZParamDefine> &param);
    void addRow(QTableWidget *table, QString name, QString type);
    QStringList localVarList();
    bool isMemberFunction();
    void dataToUi();
    void uiToData();

    QString m_className;
    bool m_newFunction;
    JZFunctionDefine m_functionDefine;    
    JZProject *m_project;
    Ui::JZNodeFuctionEditDialog *ui;
};
