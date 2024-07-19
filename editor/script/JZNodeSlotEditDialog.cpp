#include <QMessageBox>
#include <QFontMetrics>
#include <QDebug>
#include <QComboBox>
#include "JZNodeFunctionManager.h"
#include "JZNodeSlotEditDialog.h"


//JZNodeSlotEditDialog
JZNodeSlotEditDialog::JZNodeSlotEditDialog(QWidget *parent)
    : QDialog(parent)
{
}

JZNodeSlotEditDialog::~JZNodeSlotEditDialog()
{
}

QString JZNodeSlotEditDialog::param()
{
    return QString();
}

QString JZNodeSlotEditDialog::single()
{
    return QString();
}