#include <QMessageBox>
#include <QFontMetrics>
#include <QDebug>
#include <QComboBox>
#include "JZNodeFunctionManager.h"
#include "JZNodeVirtualFuctionEditDialog.h"

enum {
    Item_name = Qt::UserRole,    
    Item_type,
};


//JZNodeVirtualFuctionEditDialog
JZNodeVirtualFuctionEditDialog::JZNodeVirtualFuctionEditDialog(QWidget *parent)
    : QDialog(parent)
{
}

JZNodeVirtualFuctionEditDialog::~JZNodeVirtualFuctionEditDialog()
{
}
