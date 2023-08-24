#ifndef JZPARAM_EDITOR_H_
#define JZPARAM_EDITOR_H_

#include <QTableWidget>
#include <QUndoStack>
#include "JZEditor.h"
#include "JZParamFile.h"

namespace Ui {
    class JZParamEditor;
}

class JZParamEditor;
class JZParamEditorCommand : public QUndoCommand
{
public:
    enum {
        NewParam,
        RemoveParam,
        Rename,
        SetType,        
    };

    JZParamEditorCommand(JZParamEditor *view, int type);

    virtual void redo() override;
    virtual void undo() override;
    virtual int id() const override;
    virtual bool mergeWith(const QUndoCommand *command);

    int command;    
    QString oldName;
    QString newName;
    int oldType;
    int newType;

protected:    
    JZParamEditor *m_editor;
};

//JZParamEditor
class JZParamEditor : public JZEditor
{
    Q_OBJECT
    
public:
    JZParamEditor();
    ~JZParamEditor();
    
    virtual void open(JZProjectItem *item) override;
    virtual void close() override;
    virtual void save() override;
    virtual bool isModified() override;

protected slots:
    void on_btnAdd_clicked();
    void on_btnRemove_clicked();
    void onTypeChanged(int index);
    void onCleanChanged(bool modify);
    void onItemChanged(QTableWidgetItem *item);

protected:
    friend JZParamEditorCommand;
    void updateItem(int row,JZParamDefine *define);
    void addNewCommand(QString name, int type);
    void addRemoveCommand(QString name);
    void addRenameCommand(QString oldName,QString newName);
    void addSetTypeCommand(QString name,int newType);
    void addParam(QString newName, int newType);
    void removeParam(QString newName);
    void renameParam(QString oldName, QString newName);
    void setParamType(QString newName, int newType);
    int rowIndex(QString name);
    int rowIndex(QComboBox *box);
    int rowDataType(int row);

    JZParamFile *m_file;    
    QTableWidget *m_table;
    Ui::JZParamEditor *ui;
    int m_widgetCount;

    QUndoStack m_commandStack;
};

#endif
