#include <QVBoxLayout>
#include <QTextEdit>
#include "JZAboutDialog.h"

JZAboutDialog::JZAboutDialog(QWidget *parent)
    :JZBaseDialog(parent)
{
    QVBoxLayout *v = new QVBoxLayout();
    v->setContentsMargins(0, 0, 0, 0);

    QTextEdit *edit = new QTextEdit();
    edit->setReadOnly(true);
    v->addWidget(edit);

    m_mainWidget->setLayout(v);

    QString tips = R"(�����ǿ�������qt�ĵʹ���ƽ̨��������ͼ�Ľڵ��������֧��qt��������֣��ؼ������󶨣����̴��룬�Զ����������Թ��ܣ������Ե���c++����
����Ʒ���Խ���Ӿ��㷨���ɼ������Ƶȹ��ܣ����ٿ��������ڹ�˾�Լ��ĵʹ���ƽ̨��������Ϳ���������͵��Ե�ʱ�䡣
��ϸ��ѯ�����QQȺ

��Ҫ�ص�
1.����qt����������Ӧ�ã��򵥸�Ч.
2.�����ĳ����ܣ�ǿ���ͣ�֧�ַ�֧��ѭ�����жϣ��źţ��ۣ����ͣ��࣬lambda��ʵʱԤ�������Թ�������Ӧ�á�
3.������չ���ṩ����pybind�ӿڣ����ٰ�c++.
4.���ڵ�༭���⣬�ṩ�ȼ۽ű�֧�֣�Ҳ����ͬʱ֧�����߿����ͽű����������Ի�����á�
5.aot֧�֣����Ե���Ϊc++����.
6.�༭����չ���û����Է������չ���������������Լ��ĵʹ���Ӧ�á�

Ԥ��ʹ�ó���
�û��������ǵĵʹ���ƽ̨���ж��ο�����ͨ�����ӿ⣬�����ĳ����༭��
1.��ҵ�Զ����������Ӿ�
1.���ܼҾӣ�����node - red
2.��λ��Ӧ�ã�����labview
3.��ҵ����
4.ͼ��������blender
5.��Ƭ����plc����
6.aiӦ��
��Ϊ��Ʒ�����ǵʹ���ƽ̨������Ӧ�ó����Ǻܹ����ġ�)";

    QString text = QString("��ӭ����\n�汾��:") + __DATE__ " " __TIME__;
    text += "\n�ļ�->��->��Ŀ ���Դ� sample �����һ�� demo ����.";
    text += "\nQQȺ:598601341";
    text += "\n\n" + tips;
    edit->setText(text);

    showButton(Button_Cancel,false);
    resize(400, 500);
}

bool JZAboutDialog::onOk()
{
    return true;
}