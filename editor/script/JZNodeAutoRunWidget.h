#ifndef JZNODE_AUTO_RUN_EDIT_DIALOG_H_
#define JZNODE_AUTO_RUN_EDIT_DIALOG_H_

#include <QWidget>
#include "UiCommon.h"
#include "JZNode.h"
#include "JZNodeCompiler.h"
#include "JZNodePropertyBrowser.h"

//JZNodeAutoRunWidget
class JZNodeEditor;
class JZNodeAutoRunWidget : public QWidget
{
    Q_OBJECT

public:
    JZNodeAutoRunWidget(QWidget *p = nullptr);
    ~JZNodeAutoRunWidget();

    void setEditor(JZNodeEditor *editor);
    
    void setDepend(const ScriptDepend &depend);
    const ScriptDepend &depend() const;

    void setResult(QVariantList params);

signals:
    void sigDependChanged();

protected slots:
    void onValueChanged(JZNodeProperty *pin, const QString &value);

protected:       
    enum PinType{
        Pin_none,
        Pin_funcIn,
        Pin_funcOut,
        Pin_member,
        Pin_global,
        Pin_hook,
    };

    class PropCoor
    {
    public:
        PropCoor();

        JZNodeProperty *pin;
        PinType type;
        int nodeId;
        int index;
    };

    void addPin(JZNodeProperty *pin, PinType type, int index, int nodeId = -1);
    void clear();
    PropCoor *propCoor(PinType type, int index);
    void copyDependValue(ScriptDepend &old, ScriptDepend &dst);
    bool typeEqual(const JZParamDefine &p1, const JZParamDefine &p2);
    bool typeEqual(const QList<JZParamDefine> &p1, const QList<JZParamDefine> &p2);

    ScriptDepend m_depend;
    JZNodePropertyBrowser *m_tree;        
    QList<PropCoor> m_propList;
    JZNodeEditor *m_editor;
};

#endif
