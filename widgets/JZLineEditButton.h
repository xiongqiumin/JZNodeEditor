#ifndef JZ_LINE_EDIT_BUTTON_H_
#define JZ_LINE_EDIT_BUTTON_H_

#include <QWidget>
#include <QLineEdit>
#include <QToolButton>
#include <QHBoxLayout>

class JZLineEditButton : public QWidget
{
    Q_OBJECT
public:
    explicit JZLineEditButton(QWidget *parent = nullptr);
    ~JZLineEditButton();
    
    QLineEdit *lineEdit();
    QToolButton * button();
    
private:
    QLineEdit *m_lineEdit;
    QToolButton *m_toolButton;
};









#endif