#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QStackWidget>
#include "ui_mainwindow.h"
#include "JZProjectTree.h"
#include "JZEditor.h"
#include "JZNodeEngine.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected slots:    
    void onActionOpenProject();
    void onActionSaveProject();

    void onActionOpenTriggered();
    void onActionSaveTriggered();
    void onActionSaveAsTriggered();
    void onActionSaveAllTriggered();          

    void onFileOpened(QString filepath);

private:
    void initUi();
    
    JZNodeEngine m_nodeEngine;
    JZProject m_project;
    JZProjectTree *m_projectTree;
    JZEditor *m_editor;  
    QStackWidget *m_editorStack;
    QTextEdit *m_log;
};
#endif // MAINWINDOW_H
