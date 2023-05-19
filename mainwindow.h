#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QStackedWidget>
#include <QTextEdit>
#include "JZProjectTree.h"
#include "JZEditor.h"
#include "JZNodeEngine.h"
#include "JZNodeDebugClient.h"

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
    void resizeEvent(QResizeEvent *event);

    JZNodeEngine m_nodeEngine;
    JZProject m_project;
    JZProjectTree *m_projectTree;
    JZEditor *m_editor;  
    QStackedWidget *m_editorStack;
    QTextEdit *m_log;
    JZNodeDebugClient m_debuger;

    QVector<JZEditor *> m_editorList;
};
#endif // MAINWINDOW_H
