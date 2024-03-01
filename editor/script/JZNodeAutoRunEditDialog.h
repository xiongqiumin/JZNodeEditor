#ifndef JZNODE_AUTO_RUN_EDIT_DIALOG_H_
#define JZNODE_AUTO_RUN_EDIT_DIALOG_H_

#include <QDialog>
#include "UiCommon.h"
#include "JZNode.h"
#include "JZNodeCompiler.h"
#include "JZNodePropertyBrowser.h"

//JZNodeAutoRunEditDialog
class JZNodeAutoRunEditDialog : public QDialog
{
    Q_OBJECT

public:
    JZNodeAutoRunEditDialog(QWidget *p = nullptr);
    ~JZNodeAutoRunEditDialog();
    
    void init(ScriptDepend *depend);

protected slots:
    void on_btnOk_clicked();
    void on_btnCancel_clicked();
    void onValueChanged(JZNodeProperty *pin, const QString &value);

protected:        
    JZNodePropertyBrowser *m_tree;    
};

#endif
