#ifndef JZNODE_AUTO_RUN_EDIT_DIALOG_H_
#define JZNODE_AUTO_RUN_EDIT_DIALOG_H_

#include <QWidget>
#include "UiCommon.h"
#include "JZNode.h"
#include "JZNodeCompiler.h"
#include "JZNodePropertyBrowser.h"

//JZNodeAutoRunWidget
class JZNodeAutoRunWidget : public QWidget
{
    Q_OBJECT

public:
    JZNodeAutoRunWidget(QWidget *p = nullptr);
    ~JZNodeAutoRunWidget();
    
    void setDepend(const ScriptDepend &depend);
    const ScriptDepend &depend() const;

signals:
    void sigDependChanged();

protected slots:
    void onValueChanged(JZNodeProperty *pin, const QString &value);

protected:       
    enum {
        Pin_funcIn,
        Pin_member,
        Pin_global,
        Pin_hook,
    };

    class PropCoor
    {
    public:
        PropCoor();

        JZNodeProperty *pin;
        int type;
        int nodeId;
        int index;
    };

    void addPin(JZNodeProperty *pin, int type, int index, int nodeId = -1);
    void clear();

    ScriptDepend m_depend;
    JZNodePropertyBrowser *m_tree;        
    QList<PropCoor> m_propList;
};

#endif
