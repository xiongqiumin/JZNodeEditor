#ifndef JZNODE_AUTO_RUN_EDIT_DIALOG_H_
#define JZNODE_AUTO_RUN_EDIT_DIALOG_H_

#include <QWidget>
#include "UiCommon.h"
#include "JZNode.h"
#include "JZNodeCompiler.h"
#include "3rd/JZCommon/jzWidgets/JZPropertyBrowser.h"
#include "JZScriptUnitTest.h"

//JZNodeAutoRunWidget
class JZNodeEditor;
class JZNodeAutoRunWidget : public QWidget
{
    Q_OBJECT

public:
    JZNodeAutoRunWidget(QWidget *p = nullptr);
    ~JZNodeAutoRunWidget();

    void setEditor(JZNodeEditor *editor);

    void setDepend(JZScriptItemDepend *depend);
    JZScriptItemDepend *depend();

    void setResult(QVariantList params);

signals:
    void sigDependChanged();

protected slots:
    void onValueChanged(JZProperty *pin, const QVariant &value);

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

        JZProperty *pin;
        PinType type;
        int nodeId;
        QString name;
        int index;
    };

    void addPin(JZProperty *pin, PinType type, QString name);
    void addPin(JZProperty *pin, PinType type, int index, int nodeId);
    void clear();
    PropCoor *propCoor(PinType type, int index);    
    bool typeEqual(const JZParamDefine &p1, const JZParamDefine &p2);
    bool typeEqual(const QList<JZParamDefine> &p1, const QList<JZParamDefine> &p2);
    int editType(int data_type);

    JZScriptItemDepend *m_depend;
    JZPropertyBrowser *m_tree;        
    QList<PropCoor> m_propList;
    JZNodeEditor *m_editor;
};

#endif
