#ifndef JZPARAM_EDITOR_H_
#define JZPARAM_EDITOR_H_

#include <QTableWidget>
#include <QUndoStack>
#include "JZEditor.h"
#include "JZParamItem.h"

namespace Ui {
    class JZNodeParamEditor;
}

class JZNodeParamEditor;
class JZNodeParamEditorCommand : public QUndoCommand
{
public:
    enum {
        NewParam,
        RemoveParam,
        Rename,
        Change,        
        Bind,
    };

    JZNodeParamEditorCommand(JZNodeParamEditor *view, int type);

    virtual void redo() override;
    virtual void undo() override;
    virtual int id() const override;
    virtual bool mergeWith(const QUndoCommand *command);

    int command;        
    JZParamDefine oldParam;
    JZParamDefine newParam;
    JZNodeParamBind oldBind;
    JZNodeParamBind newBind;

protected:    
    JZNodeParamEditor *m_editor;
};

//JZNodeParamEditor
class JZNodeParamEditor : public JZEditor
{
    Q_OBJECT
    
public:
    JZNodeParamEditor();
    ~JZNodeParamEditor();
    
    virtual void open(JZProjectItem *item) override;
    virtual void close() override;
    virtual void save() override;
    virtual bool isModified() override;
    virtual void redo() override;
    virtual void undo() override;

    virtual void navigate(QUrl url) override;
    JZScriptClassItem *classItem();

protected slots:
    void on_btnAdd_clicked();
    void on_btnRemove_clicked();    
    void on_boxParamType_currentIndexChanged(int index);

    void onParamBind();

    void onCleanChanged(bool modify);
    void onItemChanged(QTableWidgetItem *item);
    void onUiItemChanged(QTableWidgetItem *item);

protected:
    friend JZNodeParamEditorCommand;
    virtual void keyPressEvent(QKeyEvent *e) override;

    void updateItem(int row,const JZParamDefine *define);
    void updateUiItem(int row,const JZParamDefine *define);
    void addNewCommand(QString name, QString type);
    void addRemoveCommand(QString name);
    void addRenameCommand(QString oldName,QString newName);
    void addChangeCommand(QString name, JZParamDefine define);
    void addBindCommand(QString name, JZNodeParamBind define);

    void addParam(JZParamDefine define);
    void removeParam(QString newName);
    void renameParam(QString oldName, QString newName);
    void changeParam(QString name, JZParamDefine define);
    void bindParam(QString name, JZNodeParamBind define);
    int rowIndex(QString name);    
    int rowIndexUi(QString name);
    int rowDataType(int row);

    JZParamItem *m_file;    
    QTableWidget *m_table;
    QTableWidget *m_tableUi;    
    Ui::JZNodeParamEditor *ui;    
    JZScriptClassItem *m_class;

    QUndoStack m_commandStack;
};

#endif
