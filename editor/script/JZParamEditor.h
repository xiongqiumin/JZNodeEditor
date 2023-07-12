#ifndef JZPARAM_EDITOR_H_
#define JZPARAM_EDITOR_H_

#include "JZEditor.h"
#include <QTableWidget>
#include "JZParamFile.h"

class JZParamEditor : public JZEditor
{
    Q_OBJECT
    
public:
    JZParamEditor();
    ~JZParamEditor();

    virtual void open(JZProjectItem *item) override;
    virtual void close() override;
    virtual void save() override;

protected slots:

protected:
    QTableWidget *m_table;
};

#endif
