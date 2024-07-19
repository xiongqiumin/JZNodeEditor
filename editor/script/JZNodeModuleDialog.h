#pragma once

#include <QDialog>
#include <QTableWidget>

class JZNodeModuleDialog : public QDialog
{
    Q_OBJECT

public:
    JZNodeModuleDialog(QWidget *parent = Q_NULLPTR);
    ~JZNodeModuleDialog();
    
protected:
    void init();
};
