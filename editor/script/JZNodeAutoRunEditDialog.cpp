#include <QVBoxLayout>
#include "JZNodeAutoRunEditDialog.h"

//JZNodeAutoRunEditDialog
JZNodeAutoRunEditDialog::JZNodeAutoRunEditDialog(QWidget *p)
    :QDialog(p)
{    
    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);        

    m_tree = new JZNodePropertyBrowser();    
    //connect(m_propManager, &QtVariantPropertyManager::valueChanged, this, &JZNodeAutoRunEditDialog::onValueChanged);
    
    l->addWidget(m_tree);
    setLayout(l);

    ScriptDepend depend;
    depend.global.push_back(JZParamDefine("b", Type_int));
    depend.member.push_back(JZParamDefine("a", Type_int));

    JZFunctionDefine func_def;
    func_def.name = "help";
    func_def.paramOut.push_back(JZParamDefine("a", Type_int));
    depend.functions[0] = func_def;
    init(&depend);
}

JZNodeAutoRunEditDialog::~JZNodeAutoRunEditDialog()
{    
}

void JZNodeAutoRunEditDialog::init(ScriptDepend *depend)
{
    auto item_input = new JZNodeProperty("运行输入", NodeProprety_GroupId);
    auto item_output = new JZNodeProperty("运行输出", NodeProprety_GroupId);
    m_tree->addProperty(item_input);
    m_tree->addProperty(item_output);
/*
    QTreeWidgetItem *item_function_input = new QTreeWidgetItem();
    item_function_input->setText(0,"输入参数");
    ui->tree->addTopLevelItem(item_function_input);
*/
    if (depend->member.size() > 0)
    {
        auto item_member = new JZNodeProperty("成员变量", NodeProprety_GroupId);
        item_input->addSubProperty(item_member);

        for (int i = 0; i < depend->member.size(); i++)
        {
            auto sub_item = new JZNodeProperty(depend->member[i].name, depend->member[i].dataType());
            sub_item->setValue(depend->member[i].value);

            item_member->addSubProperty(sub_item);
        }
    }

    if (depend->global.size() > 0)
    {        
        auto item_global = new JZNodeProperty("全局变量", NodeProprety_GroupId);
        item_input->addSubProperty(item_global);

        for (int i = 0; i < depend->global.size(); i++)
        {
            auto sub_item = new JZNodeProperty(depend->global[i].name, depend->global[i].dataType());
            sub_item->setValue(depend->global[i].value);

            item_global->addSubProperty(sub_item);
        }
    }

    if (depend->functions.size() > 0)
    {        
        auto item_function_hook = new JZNodeProperty("函数返回", NodeProprety_GroupId);
        item_input->addSubProperty(item_function_hook);

        auto it = depend->functions.begin();
        while(it != depend->functions.end())
        {
            QString id = "(" + QString::number(it.key()) + ")";
            auto item_function = new JZNodeProperty(it->fullName() + id, NodeProprety_GroupId);
            item_function_hook->addSubProperty(item_function);

            auto &func_def = it.value();
            for (int i = 0; i < func_def.paramOut.size(); i++)
            {
                auto sub_item = new JZNodeProperty(func_def.paramOut[i].name, func_def.paramOut[i].dataType());
                sub_item->setValue(func_def.paramOut[i].value);

                item_function->addSubProperty(sub_item);
            }
            it++;
        }
    }
}

void JZNodeAutoRunEditDialog::onValueChanged(JZNodeProperty *pin, const QString &value)
{

}

void JZNodeAutoRunEditDialog::on_btnOk_clicked()
{    
    QDialog::accept();
}

void JZNodeAutoRunEditDialog::on_btnCancel_clicked()
{
    QDialog::reject();
}